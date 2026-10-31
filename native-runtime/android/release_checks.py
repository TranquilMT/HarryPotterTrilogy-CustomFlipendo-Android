"""Release gates for complete HP2 data and external-only HD packaging."""
import hashlib
from pathlib import Path

SYSTEM = ('Game.exe', 'Core.u', 'Engine.u', 'HGame.u', 'HProps.u', 'HPModels.u', 'HPSounds.u')
HD_PREFIXES = ('assets/bundled-hd/', 'assets/texpacks/')

def hp2_inventory(root):
    root = Path(root)
    files = {}
    for p in root.rglob('*'):
        if p.is_symlink():
            raise ValueError('Game data must not contain symbolic links')
        if p.is_file() and not any(part.startswith('.') for part in p.relative_to(root).parts):
            relative = p.relative_to(root).as_posix()
            key = relative.lower()
            if key in files:
                raise ValueError('Case-colliding game paths: ' + relative)
            files[key] = (relative, p)
    required = ['System/' + name for name in SYSTEM] + ['Sounds/AllDialog.uax']
    for name in required:
        item = files.get(name.lower())
        if not item or item[1].stat().st_size == 0:
            raise ValueError('Missing HP2 game data: ' + name)
    for folder, suffix in [('Maps/', '.unr'), ('Textures/', '.utx')]:
        if not any(k.startswith(folder.lower()) and k.endswith(suffix) and p.stat().st_size > 0 for k, (_, p) in files.items()):
            raise ValueError('Missing HP2 packages: ' + folder)
    for name in ('System/Core.u', 'System/Engine.u', 'System/HGame.u'):
        with files[name.lower()][1].open('rb') as src:
            header = src.read(8)
        if len(header) < 8 or header[:4] != bytes.fromhex('c1832a9e') or not 60 <= int.from_bytes(header[4:6], 'little') < 100:
            raise ValueError('Unsupported or corrupt HP2 package: ' + name)
    result = []
    for _, (relative, p) in sorted(files.items()):
        with p.open('rb') as src:
            digest = hashlib.file_digest(src, 'sha256').hexdigest()
        result.append({'path': relative, 'bytes': p.stat().st_size, 'sha256': digest})
    return result

def verify_embedded_hp2(apk, inventory):
    names = apk.namelist()
    if any(n.startswith(HD_PREFIXES) for n in names):
        raise ValueError('HD texture packs must remain outside the APK')
    if any(n.lower().endswith(('.jks', '.keystore', '/password.txt')) for n in names):
        raise ValueError('Private signing material must never be packaged')
    expected = {'assets/hp2data/' + row['path'] for row in inventory}
    actual = {n for n in names if n.startswith('assets/hp2data/') and not n.endswith('/') and n != 'assets/hp2data/.stamp'}
    if actual != expected:
        raise ValueError('Embedded HP2 inventory mismatch')
    for row in inventory:
        name = 'assets/hp2data/' + row['path']
        if apk.getinfo(name).file_size != row['bytes']:
            raise ValueError('Embedded HP2 size mismatch: ' + row['path'])
        with apk.open(name) as src:
            digest = hashlib.file_digest(src, 'sha256').hexdigest()
        if digest != row['sha256']:
            raise ValueError('Embedded HP2 checksum mismatch: ' + row['path'])

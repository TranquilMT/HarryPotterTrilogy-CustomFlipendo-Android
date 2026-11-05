"""Release gates for complete HP2 data and external-only HD packaging."""
import hashlib
from pathlib import Path

SYSTEM = ('Game.exe', 'Core.u', 'Engine.u', 'HGame.u', 'HProps.u', 'HPModels.u', 'HPSounds.u')
HD_PREFIXES = ('assets/bundled-hd/', 'assets/texpacks/')

def game_inventory(root, hp2):
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
    system = SYSTEM if hp2 else ('HP.exe', 'Core.u', 'Engine.u', 'HarryPotter.u', 'HPBase.u', 'HPPuzzle.u', 'HPMenu.u')
    required = ['System/' + name for name in system] + ['Sounds/AllDialog.uax']
    for name in required:
        item = files.get(name.lower())
        if not item or item[1].stat().st_size == 0:
            raise ValueError('Missing HP2 game data: ' + name)
    for folder, suffix in [('Maps/', '.unr'), ('Textures/', '.utx')]:
        if not any(k.startswith(folder.lower()) and k.endswith(suffix) and p.stat().st_size > 0 for k, (_, p) in files.items()):
            raise ValueError('Missing HP2 packages: ' + folder)
    for name in ('System/Core.u', 'System/Engine.u', 'System/HGame.u' if hp2 else 'System/HarryPotter.u'):
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

def hp1_inventory(root):
    return game_inventory(root, False)

def hp2_inventory(root):
    return game_inventory(root, True)

def verify_embedded_game(apk, inventory, prefix):
    names = apk.namelist()
    if any(n.startswith(HD_PREFIXES) for n in names):
        raise ValueError('HD texture packs must remain outside the APK')
    if any(n.lower().endswith(('.jks', '.keystore', '/password.txt')) for n in names):
        raise ValueError('Private signing material must never be packaged')
    expected = {prefix + row['path'] for row in inventory}
    actual = {n for n in names if n.startswith(prefix) and not n.endswith('/') and n not in (prefix + '.stamp', prefix + '.inventory.tsv')}
    if len(names) != len(set(names)):
        raise ValueError('Duplicate APK entries')
    if actual != expected:
        raise ValueError('Embedded HP2 inventory mismatch')
    for row in inventory:
        name = prefix + row['path']
        if apk.getinfo(name).file_size != row['bytes']:
            raise ValueError('Embedded HP2 size mismatch: ' + row['path'])
        with apk.open(name) as src:
            digest = hashlib.file_digest(src, 'sha256').hexdigest()
        if digest != row['sha256']:
            raise ValueError('Embedded HP2 checksum mismatch: ' + row['path'])



def verify_embedded_hp2(apk, inventory):
    verify_embedded_game(apk, inventory, 'assets/hp2data/')

def verify_bundled_games(apk, hp1, hp2, hp3=None):
    for prefix, inventory in [('assets/gamedata/', hp1), ('assets/hp2data/', hp2)] + ([('assets/hp3data/', hp3)] if hp3 is not None else []):
        paths = {r['path'].lower() for r in inventory}
        if not {'system/default.ini', 'system/defuser.ini'} <= paths:
            raise ValueError('Both games require bundled startup configuration')
        if prefix + '.stamp' not in apk.namelist() or len(apk.read(prefix + '.stamp')) != 64:
            raise ValueError('Missing game installation stamp')
        verify_embedded_game(apk, inventory, prefix)
        if prefix == 'assets/hp3data/':
            expected = ''.join(r['sha256'] + '\t' + str(r['bytes']) + '\t' + r['path'] + '\n' for r in inventory).encode()
            if apk.read(prefix + '.inventory.tsv') != expected:
                raise ValueError('PoA installation inventory differs from bundled files')


def hp3_inventory(root):
    root = Path(root)
    files = {}
    for p in root.rglob('*'):
        if p.is_symlink():
            raise ValueError('Symbolic link in PoA payload')
        if p.is_file() and not any(x.startswith('.') for x in p.relative_to(root).parts):
            name = p.relative_to(root).as_posix()
            if name.lower() in files:
                raise ValueError('Case-colliding PoA path: ' + name)
            files[name.lower()] = (name, p)
    for name in ('hppoa.exe','core.u','engine.u','hgame.u','gui.u','uwindow.u','gameplay.u','default.ini','defuser.ini'):
        item = files.get('system/' + name)
        if not item or not item[1].stat().st_size:
            raise ValueError('Missing PoA System/' + name)
        if name.endswith('.u'):
            with item[1].open('rb') as f:
                header = f.read(8)
            if header[:4] != bytes.fromhex('c1832a9e') or int.from_bytes(header[4:6], 'little') != 129:
                raise ValueError('Invalid PoA v129 package: ' + name)
    for folder, suffix in [('maps/', '.unr'), ('textures/', '.utx'), ('animations/', '.ukx'), ('staticmeshes/', '.usx')]:
        if not any(k.startswith(folder) and k.endswith(suffix) for k in files):
            raise ValueError('Missing PoA resource: ' + folder)
    rows = []
    for _, (name, p) in sorted(files.items()):
        with p.open('rb') as f:
            digest = hashlib.file_digest(f, 'sha256').hexdigest()
        rows.append(dict(path=name, bytes=p.stat().st_size, sha256=digest))
    return rows

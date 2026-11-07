"""Inspect original PoA startup configuration; never enable or execute the game."""
import argparse
import hashlib
import json
from pathlib import Path
import re

from inspect_packages import inspect

CLASS_FIELDS = {
    'gameClass': 'DefaultGame', 'consoleClass': 'Console',
    'guiController': 'GUIController', 'worldInfoClass': 'WorldInfo',
    'loadingControllerClass': 'LevelLoadingPlayerControllerClass',
    'loadingHUDClass': 'LevelLoadingHUDClass',
}


def profile_identity(executable_sha256, scripts, reference):
    """Keep changed script sets separate even when they use the same executable."""
    def identities(rows):
        result = {}
        for row in rows:
            key = row['file'].casefold()
            if key in result:
                raise ValueError('Duplicate script identity: ' + key)
            result[key] = row['sha256']
        return result
    actual = identities(scripts)
    expected = identities(reference['startupEvidence']['scriptPackages'])
    if executable_sha256 == reference['exeSHA256'] and actual == expected:
        return reference['id']
    payload = json.dumps([executable_sha256, actual], sort_keys=True, separators=(',', ':')).encode()
    return 'hp3-pc-unrecognized-' + hashlib.sha256(payload).hexdigest()[:12]


def file_index(root):
    files = {}
    for path in sorted(root.rglob('*')):
        if not path.is_file() or any(p.startswith('.') for p in path.relative_to(root).parts):
            continue
        key = path.relative_to(root).as_posix().casefold()
        if key in files:
            raise ValueError('Case-colliding game paths: ' + key)
        files[key] = path
    return files


def read_ini(data):
    if data.startswith((b'\xff\xfe', b'\xfe\xff')):
        text = data.decode('utf-16')
    else:
        text = data.decode('utf-8-sig') if data.startswith(b'\xef\xbb\xbf') else data.decode('latin1')
    sections, section = {}, None
    for number, line in enumerate(text.splitlines(), 1):
        line = line.strip()
        if not line or line.startswith((';', '#')):
            continue
        if line.startswith('['):
            if not line.endswith(']') or not line[1:-1].strip():
                raise ValueError('Invalid INI section at line ' + str(number))
            section = sections.setdefault(line[1:-1].strip().casefold(), {})
        elif '=' in line and section is not None:
            key, value = line.split('=', 1)
            if not key.strip():
                raise ValueError('Empty INI key at line ' + str(number))
            section.setdefault(key.strip().casefold(), []).append(value.strip())
    return sections


def one(ini, section, key):
    values = ini.get(section.casefold(), {}).get(key.casefold(), [])
    if len(values) != 1 or not values[0]:
        raise ValueError('Missing or ambiguous configuration: [' + section + '] ' + key)
    return values[0]


def map_path(value):
    if not re.fullmatch(r'[A-Za-z0-9_\-]+\.unr', value, re.IGNORECASE):
        raise ValueError('Unsupported map filename or travel URL: ' + value)
    return 'maps/' + value.casefold()


def validate_classes(route, models, files=None):
    external = []
    for field in CLASS_FIELDS:
        value = route[field]
        if not re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*\.[A-Za-z_][A-Za-z0-9_]*', value):
            raise ValueError('Unsupported qualified class name: ' + value)
        package, name = value.casefold().split('.')
        model = models.get(package)
        found = [] if model is None else [e for e in model['exports']
                 if e['class'].casefold() == 'class' and not e['outer'] and e['name'].casefold() == name]
        if len(found) != 1:
            raise ValueError('Startup class unavailable or ambiguous: ' + value)
    return external


def audit_startup(root):
    root = Path(root)
    files = file_index(root)
    def required(key):
        if key not in files:
            raise ValueError('Required original file missing: ' + key)
        return files[key]
    default = required('system/default.ini')
    user = required('system/defuser.ini')
    exe = required('system/hppoa.exe')
    ini = read_ini(default.read_bytes())
    route = {'startupMap': one(ini, 'URL', 'LocalMap'),
             'firstStoryMap': one(ini, 'HGame.HPPageFrontend', 'FirstLevel')}
    for field, key in CLASS_FIELDS.items():
        route[field] = one(ini, 'Engine.Engine', key)
    models = {}
    for key, path in files.items():
        if key.startswith('system/') and path.suffix.casefold() == '.u':
            name = path.stem.casefold()
            if name in models:
                raise ValueError('Ambiguous script package: ' + name)
            models[name] = inspect(path)
    external_classes = validate_classes(route, models, files)
    server_packages = ini.get('engine.gameengine', {}).get('serverpackages', [])
    for name in server_packages:
        if not re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', name) or name.casefold() not in models:
            raise ValueError('Configured server package missing or invalid: ' + name)
    selected_maps = {}
    for field in ('startupMap', 'firstStoryMap'):
        path = required(map_path(route[field]))
        model = inspect(path)
        if model['classCounts'].get('Level', 0) != 1 or not model['classCounts'].get('PlayerStart', 0):
            raise ValueError('Configured map lacks expected Level/PlayerStart exports: ' + route[field])
        selected_maps[field] = {'configuredName': route[field], 'path': path.relative_to(root).as_posix(),
                                **{key: model[key] for key in ('bytes', 'sha256', 'version', 'licenseeVersion', 'classCounts')}}
    def identity(path):
        data = path.read_bytes()
        return {'path': path.relative_to(root).as_posix(), 'bytes': len(data),
                'sha256': hashlib.sha256(data).hexdigest()}
    return {'status': 'original-data inspection only', 'runtimeSupported': False,
            'route': route, 'serverPackages': server_packages,
            'language': one(ini, 'Engine.Engine', 'Language'),
            'configuration': [identity(default), identity(user)], 'executable': identity(exe),
            'scriptPackages': [{**{k: model[k] for k in ('file', 'bytes', 'sha256', 'version', 'licenseeVersion')},
                                'classes': model['classCounts'].get('Class', 0),
                                'functions': model['classCounts'].get('Function', 0)}
                               for _, model in sorted(models.items())],
            'maps': selected_maps,
            'classesVerifiedInPackageTables': [field for field in CLASS_FIELDS
                if field not in {item['field'] for item in external_classes}],
            'classesRequiringNativeModule': external_classes,
            'dependencyClosureVerified': False, 'liveStartupVerified': False,
            'interactiveGameplayVerified': False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('game', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    report = audit_startup(args.game)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({'scripts': len(report['scriptPackages']), 'route': report['route'],
                      'liveStartupVerified': False}, indent=2))


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Read-only Android build input checks. Never creates keys, APKs or game data."""
import argparse
import hashlib
import json
import os
import re
from pathlib import Path, PurePosixPath
import shutil
import subprocess

from release_checks import hp1_inventory, hp2_inventory

CERTIFICATE = '25aa34c1db19a5007e934df03f5927e4b71a990856fe439a57e3e73ead8eae40'


def source_identity(native_root):
    native_root = Path(native_root).resolve()
    git = shutil.which('git')
    if git:
        top = subprocess.run([git, '-C', str(native_root), 'rev-parse', '--show-toplevel'], capture_output=True, text=True)
        if top.returncode == 0 and Path(top.stdout.strip()).resolve() in (native_root, native_root.parent):
            revision = subprocess.run([git, '-C', str(native_root), 'rev-parse', 'HEAD'], capture_output=True, text=True)
            if revision.returncode == 0:
                dirty = subprocess.run([git, '-C', str(native_root), 'status', '--porcelain'], capture_output=True, text=True)
                if dirty.returncode:
                    raise ValueError('Cannot establish Git working-tree state')
                return 'git:' + revision.stdout.strip() + (';dirty' if dirty.stdout else ';clean')
    project = native_root.parent
    lock_path = project / 'docs/POA-SOURCE-LOCK.json'
    if not lock_path.is_file():
        raise ValueError('No Git revision or source checkpoint lock is available')
    data = lock_path.read_bytes()
    lock = json.loads(data)
    if not lock.get('checkpoint') or not lock.get('files'):
        raise ValueError('Source checkpoint lock is empty')
    if 'native-runtime/android/build_linux.py' not in lock['files']:
        raise ValueError('Source checkpoint lock does not cover the Android builder')
    for name, expected in lock['files'].items():
        rel = PurePosixPath(name)
        if rel.is_absolute() or '..' in rel.parts or '\\' in name or ':' in name:
            raise ValueError('Unsafe source checkpoint path')
        path = project.joinpath(*rel.parts)
        if not path.is_file() or path.is_symlink() or not path.resolve().is_relative_to(project):
            raise ValueError('Source checkpoint file missing or unsafe: ' + name)
        with path.open('rb') as stream:
            digest = hashlib.file_digest(stream, 'sha256').hexdigest()
        if path.stat().st_size != expected['bytes'] or digest != expected['sha256']:
            raise ValueError('Source differs from checkpoint: ' + name)
    return 'checkpoint:' + lock['checkpoint'] + ';lock-sha256:' + hashlib.sha256(data).hexdigest()


def check_inputs(native_root, inputs):
    checks = []
    def add(name, ok, detail):
        checks.append({'input': name, 'ready': bool(ok), 'detail': detail})
    def file(name, base, relative, executable=False):
        path = Path(base) / relative if base else None
        ok = path is not None and path.is_file() and (not executable or os.access(path, os.X_OK))
        add(name, ok, 'present' if ok else 'missing or not executable: ' + relative)
    try:
        add('sourceIdentity', True, source_identity(native_root))
    except (ValueError, OSError, KeyError) as error:
        add('sourceIdentity', False, str(error))
    for rel in ['platforms/android-36/android.jar', 'build-tools/36.0.0/aapt2', 'build-tools/36.0.0/d8',
                'build-tools/36.0.0/zipalign', 'build-tools/36.0.0/apksigner']:
        file('sdk:' + rel, inputs.get('sdk'), rel, executable=not rel.endswith('.jar'))
    for rel in ['build/cmake/android.toolchain.cmake', 'toolchains/llvm/prebuilt/linux-x86_64/bin/clang++',
                'toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip']:
        file('ndk:' + rel, inputs.get('ndk'), rel, executable=not rel.endswith('.cmake'))
    for rel in ['bin/java', 'bin/javac']:
        file('jdk:' + rel, inputs.get('jdk'), rel, executable=True)
    for name in ['cmake', 'ninja']:
        command = inputs.get(name) or name
        add(name, shutil.which(str(command)) is not None, 'executable lookup only; version/build not tested')
    for name in ['keystore', 'password_file', 'previous_apk']:
        path = inputs.get(name)
        add(name, bool(path and Path(path).is_file()), 'present' if path and Path(path).is_file() else 'not supplied or missing')
    key, password, keytool = inputs.get('keystore'), inputs.get('password_file'), shutil.which('keytool')
    cert_ok = False
    if key and password and Path(key).is_file() and Path(password).is_file() and keytool:
        result = subprocess.run([keytool, '-exportcert', '-alias', 'hp-native', '-keystore', str(key),
                                 '-storepass:file', str(password)], capture_output=True)
        cert_ok = result.returncode == 0 and hashlib.sha256(result.stdout).hexdigest() == CERTIFICATE
    add('permanentCertificate', cert_ok, 'matches recorded certificate' if cert_ok else 'not verified; private tool output withheld')
    previous, sdk = inputs.get('previous_apk'), inputs.get('sdk')
    apksigner = Path(sdk) / 'build-tools/36.0.0/apksigner' if sdk else None
    previous_ok = False
    if previous and Path(previous).is_file() and apksigner and apksigner.is_file() and os.access(apksigner, os.X_OK):
        result = subprocess.run([str(apksigner), 'verify', '--print-certs', str(previous)], capture_output=True, text=True)
        match = re.search(r'Signer #1 certificate SHA-256 digest: ([0-9a-fA-F]{64})', result.stdout)
        previous_ok = result.returncode == 0 and match is not None and match.group(1).lower() == CERTIFICATE
    add('previousApkCertificate', previous_ok, 'matches recorded certificate' if previous_ok else 'not verified')
    for name, inspector in [('hp1_data', hp1_inventory), ('hp2_data', hp2_inventory)]:
        path = inputs.get(name)
        if not path:
            add(name, False, 'complete original game folder not supplied')
            continue
        try:
            rows = inspector(Path(path))
            add(name, True, str(len(rows)) + ' payload files inventoried and hashed')
        except (ValueError, OSError) as error:
            add(name, False, str(error))
    return {'scope': 'read-only source/tool/input preflight; not a build or device acceptance',
            'checks': checks, 'allPreflightChecksPassed': all(c['ready'] for c in checks),
            'apkBuilt': False, 'poaRuntimeVerified': False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, default=Path(__file__).resolve().parent.parent)
    for name in ['sdk', 'ndk', 'jdk', 'cmake', 'ninja', 'keystore', 'password-file', 'previous-apk', 'hp1-data', 'hp2-data']:
        parser.add_argument('--' + name)
    parser.add_argument('--output', type=Path)
    args = vars(parser.parse_args())
    report = check_inputs(args.pop('source'), args)
    if args['output']:
        args['output'].parent.mkdir(parents=True, exist_ok=True)
        args['output'].write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
    return 0 if report['allPreflightChecksPassed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())

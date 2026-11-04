"""Run data-free checks, optionally followed by a supplied retail corpus.

No downloads, game execution or Android launch. Generated reports stay in OUTPUT.
"""
import argparse
import pathlib
import subprocess
import sys
import tempfile
import os


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--game', type=pathlib.Path)
    p.add_argument('--output', type=pathlib.Path)
    a = p.parse_args()
    if bool(a.game) != bool(a.output):
        p.error('--game and --output must be supplied together')
    root = pathlib.Path(__file__).resolve().parents[2]
    game = a.game.resolve() if a.game else None
    output = a.output.resolve() if a.output else None
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0')

    def run(*cmd):
        subprocess.run([str(x) for x in cmd], cwd=root, env=env, check=True)

    run(sys.executable, '-m', 'unittest', 'discover', '-s', 'tools/poa', '-p', 'test_*.py')
    with tempfile.TemporaryDirectory(prefix='poa-checks-') as tmp:
        for name in ('package_loading', 'named_calls', 'dispatch_resolver', 'receiver_program', 'instance_properties', 'bool_variables', 'default_properties', 'struct_properties', 'struct_defaults', 'object_context', 'object_constraints', 'object_call_constraints', 'receiver_lifetime'):
            binary = pathlib.Path(tmp) / name
            run('g++', '-std=c++17', '-O1', '-Wall', '-Wextra', '-Werror',
                '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
                'tools/poa/test_' + name + '.cpp', '-o', binary)
            run(binary)
        if game:
            output.mkdir(parents=True, exist_ok=True)
            metadata, fixture = output / 'metadata.json', output / 'dispatch.bin'
            run(sys.executable, 'tools/poa/dispatch_metadata.py', game, metadata)
            audit = output / 'calls.json'
            run(sys.executable, 'tools/poa/audit_dispatch_calls.py', game, audit)
            import json
            if json.loads(audit.read_text())['errors']:
                raise SystemExit('Call audit has decode errors; inspect calls.json')
            if not json.loads(metadata.read_text())['complete']:
                raise SystemExit('Metadata remains incomplete; inspect metadata.json. Complete corpus checks withheld.')
            run(sys.executable, 'tools/poa/make_dispatch_fixtures.py', metadata, fixture)
            binary = pathlib.Path(tmp) / 'corpus'
            run('g++', '-std=c++17', '-O1', '-Wall', '-Wextra', '-Werror',
                '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
                'tools/poa/test_dispatch_corpus.cpp', '-o', binary)
            run(binary, fixture)
    print('PASS compatibility checks; no Android/gameplay claim')


if __name__ == '__main__':
    main()


#!/usr/bin/env python3
"""Verify the recorded PoA source checkpoint, without loading game data."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath


def verify(root, manifest):
    errors = []
    for name, expected in manifest['files'].items():
        rel = PurePosixPath(name)
        if rel.is_absolute() or '..' in rel.parts or '\\' in name:
            raise ValueError('Unsafe manifest path')
        path = root.joinpath(*rel.parts)
        if not path.is_file() or path.is_symlink():
            errors.append({'path': name, 'error': 'missing or not a regular file'})
            continue
        with path.open('rb') as stream:
            actual = hashlib.file_digest(stream, 'sha256').hexdigest()
        if path.stat().st_size != expected['bytes'] or actual != expected['sha256']:
            errors.append({'path': name, 'error': 'source differs from checkpoint'})
    return errors


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[3])
    args = parser.parse_args()
    manifest = json.loads((args.root / 'docs/POA-SOURCE-LOCK.json').read_text())
    errors = verify(args.root, manifest)
    print(json.dumps({'checkpoint': manifest['checkpoint'], 'filesChecked': len(manifest['files']),
                      'passed': not errors, 'errors': errors}, indent=2))
    return bool(errors)


if __name__ == '__main__':
    raise SystemExit(main())

"""Verify extracted PoA packages and static dependency closure; never execute them."""
import argparse
import collections
import hashlib
import json
from pathlib import Path, PurePosixPath
import zlib

from disc_packages import normalized_path
from inspect_packages import inspect
from startup_profile import file_index

PACKAGE_EXTENSIONS = {'.u', '.unr', '.utx', '.usx', '.ukx', '.uax', '.umx'}


def reference_hashes(reference):
    result = {}
    for row in reference.get('scriptPackages', []):
        result[('system/' + row['file']).casefold()] = row['sha256']
    for row in reference.get('maps', {}).values():
        result[normalized_path(row['path']).casefold()] = row['sha256']
    return result


def dependency_graph(packages, roots):
    results = {}
    for root in roots:
        pending = [root.casefold()]
        seen, missing, invalid, unknown = set(), set(), set(), set()
        while pending:
            name = pending.pop()
            if name in seen:
                continue
            seen.add(name)
            row = packages.get(name)
            if row is None:
                unknown.add(name)
            elif row['status'] == 'missing':
                missing.add(name)
            elif row['status'] != 'valid':
                invalid.add(name)
            else:
                pending.extend(row['rootDependencies'])
        results[root] = {'visitedPackages': sorted(seen), 'missingPayloads': sorted(missing),
                         'invalidOrUnanchoredPayloads': sorted(invalid),
                         'dependenciesOutsideInventory': sorted(unknown),
                         'staticClosureComplete': not (missing or invalid or unknown),
                         'dynamicLoadsVerified': False}
    return results


def verify_packages(root, inventory, known_hashes=None, roots=()):
    root = Path(root).resolve()
    actual = file_index(root)
    expected, identities = {}, {}
    known_hashes = known_hashes or {}
    for entry in inventory['files']:
        name = normalized_path(entry['path'])
        if PurePosixPath(name).suffix.casefold() not in PACKAGE_EXTENSIONS:
            continue
        key, identity = name.casefold(), PurePosixPath(name).stem.casefold()
        if key in expected or identity in identities:
            raise ValueError('Ambiguous package in inventory: ' + name)
        if not isinstance(entry['size'], int) or entry['size'] < 0:
            raise ValueError('Invalid package size: ' + name)
        if 'crc32' in entry and (not isinstance(entry['crc32'], int) or not 0 <= entry['crc32'] <= 0xffffffff):
            raise ValueError('Invalid declared package CRC: ' + name)
        expected[key], identities[identity] = entry, key
    if not expected:
        raise ValueError('Inventory contains no Unreal packages')
    packages = {}
    for identity, key in sorted(identities.items()):
        entry = expected[key]
        row = {'path': entry['path'], 'status': 'missing', 'rootDependencies': []}
        packages[identity] = row
        path = actual.get(key)
        if path is None:
            continue
        try:
            if path.is_symlink() or not path.resolve().is_relative_to(root):
                raise ValueError('Package path leaves supplied data folder')
            if path.stat().st_size != entry['size']:
                raise ValueError('Package size differs from disc inventory')
            digest, crc = hashlib.sha256(), 0
            with path.open('rb') as source:
                for block in iter(lambda: source.read(1024 * 1024), b''):
                    digest.update(block)
                    crc = zlib.crc32(block, crc)
            sha = digest.hexdigest()
            row.update(actualPath=path.relative_to(root).as_posix(), sha256=sha, crc32=f'{crc:08x}')
            if 'crc32' in entry and crc != entry['crc32']:
                raise ValueError('Package CRC differs from disc inventory')
            if key in known_hashes and sha != known_hashes[key]:
                raise ValueError('Package differs from pinned reference SHA-256')
            model = inspect(path)
            if model['sha256'] != sha:
                raise ValueError('Package changed during verification')
            row.update(version=model['version'], licenseeVersion=model['licenseeVersion'],
                       classCounts=model['classCounts'], bytes=model['bytes'],
                       rootDependencies=sorted({e['name'].casefold() for e in model['imports']
                                                if e['class'].casefold() == 'package' and e['outer'] == 0}),
                       integrityAnchor=('referenceSHA256' if key in known_hashes else
                                        'discDeclaredCRC32' if 'crc32' in entry else None))
            row['status'] = 'valid' if row['integrityAnchor'] else 'unanchored'
        except (ValueError, OSError) as error:
            row.update(status='invalid', error=str(error))
    unlisted = sorted(path.relative_to(root).as_posix() for key, path in actual.items()
                      if path.suffix.casefold() in PACKAGE_EXTENSIONS and key not in expected)
    counts = dict(collections.Counter(row['status'] for row in packages.values()))
    return {'scope': 'extracted package integrity/tables and static imports only',
            'packages': packages, 'summary': counts, 'expectedPackages': len(expected),
            'unlistedPackageFiles': unlisted,
            'presentPayloadsValid': not (counts.get('invalid') or counts.get('unanchored') or unlisted),
            'requiredPackagesComplete': counts.get('valid', 0) == len(expected) and not unlisted,
            'dependencyRoutes': dependency_graph(packages, roots),
            'nativeLayoutsOrAssetPayloadsDeserialized': False,
            'runtimeIntegrated': False, 'playable': False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('game', type=Path)
    parser.add_argument('inventory', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--reference', type=Path, help='Known startup/script SHA-256 reference report')
    parser.add_argument('--root', action='append', default=[], help='Package root to trace; may be repeated')
    args = parser.parse_args()
    known = reference_hashes(json.loads(args.reference.read_text())) if args.reference else {}
    report = verify_packages(args.game, json.loads(args.inventory.read_text()), known, args.root)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: report[k] for k in ('summary', 'expectedPackages', 'requiredPackagesComplete', 'dependencyRoutes')}, indent=2))
    return 0 if report['requiredPackagesComplete'] and all(r['staticClosureComplete'] for r in report['dependencyRoutes'].values()) else 1


if __name__ == '__main__':
    raise SystemExit(main())

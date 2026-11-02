"""Resolve class-owned ObjectProperty constraints from checked retail metadata."""
import argparse
import collections
import json
import pathlib
import struct
from inspect_packages import inspect, Reader
from package_objects import PackageObjects
from make_struct_fixtures import property_tail


def unsupported_argument_flags(flags, role):
    # Return properties also carry CPF_OutParm; that does not make a return
    # value an out argument. Only argument slots need the out/lazy exclusion.
    return role != 2 and bool(flags & (0x100 | 0x200))


def main():
    p = argparse.ArgumentParser(); p.add_argument('game'); p.add_argument('output'); p.add_argument('--function-slots', action='store_true'); p.add_argument('--schemas'); a = p.parse_args()
    slot_fields = {}
    if a.function_slots:
        if not a.schemas: p.error('--function-slots requires --schemas')
        for function in json.loads(pathlib.Path(a.schemas).read_text()):
            for field in function['arguments']:
                slot_fields[(function['package'].lower(), field['exportIndex'])] = (field, 1 | (4 if field['optional'] else 0))
            if function['returnProperty']:
                field = function['returnProperty']; slot_fields[(function['package'].lower(), field['exportIndex'])] = (field, 2)
    packages = [(path, inspect(path), path.read_bytes()) for path in sorted(pathlib.Path(a.game).rglob('*')) if path.suffix.lower() == '.u']
    catalog = PackageObjects([(path.stem, model, data) for path, model, data in packages])
    classes = {key: obj for key, obj in catalog.exports.items() if obj['entry']['class'] == 'Class'}
    ids = {key: i for i, key in enumerate(sorted(classes), 1)}
    parents = {}
    for key, obj in classes.items():
        ref = obj['entry']['superIndex']; parent = catalog.resolve(obj['package'], ref)
        if ref and (parent is None or parent['identity'] not in ids):
            raise ValueError('Unresolved class ancestor prevents constraint fixture')
        parents[ids[key]] = ids[parent['identity']] if parent else 0
    cases, counts = [], collections.Counter()
    for obj in catalog.exports.values():
        if obj['entry']['class'] != 'ObjectProperty': continue
        owner = catalog.resolve(obj['package'], obj['entry']['outer'])
        role = 0
        if a.function_slots:
            slot = slot_fields.get((obj['model']['file'].lower(), obj['index']))
            if owner is None or owner['entry']['class'] != 'Function' or slot is None:
                counts['nonArgumentOrReturnDeferred'] += 1; continue
            field, role = slot
            if unsupported_argument_flags(field['flags'], role):
                counts['outOrLazySlotsDeferred'] += 1; continue
            owner = catalog.resolve(owner['package'], owner['entry']['outer'])
            if owner and owner['entry']['class'] == 'State':
                owner = catalog.resolve(owner['package'], owner['entry']['outer'])
        if owner is None or owner['entry']['class'] != 'Class':
            counts['nonClassOwnerDeferred'] += 1; continue
        dimension, tail = property_tail(obj['data'], obj['model'], obj['entry'])
        r = Reader(tail); target = catalog.resolve(obj['package'], r.compact())
        if dimension != 1 or r.p != len(tail) or target is None or target['identity'] not in ids:
            counts['unresolvedOrUnsupportedLayoutDeferred'] += 1; continue
        cases.append((ids[owner['identity']], ids[target['identity']], role))
        counts['functionObjectSlots' if a.function_slots else 'classOwnedObjectProperties'] += 1
        if role:
            counts['returnSlots' if role == 2 else 'argumentSlots'] += 1
    with open(a.output, 'wb') as out:
        def u32(n): out.write(struct.pack('<I', n))
        u32(len(parents))
        for identity, parent in sorted(parents.items()): u32(identity); u32(parent)
        u32(len(cases))
        for owner, target, role in cases:
            u32(owner); u32(target)
            if a.function_slots: u32(role)
    report = dict(classes=len(classes), functionSlots=a.function_slots, counts=counts, scope=('Original argument/return metadata in synthetic identity functions; original functions/defaults/native behavior not executed' if a.function_slots else 'Original property/class metadata in synthetic setter tests; no retail script/default/native behavior executed'), runtimeIntegrated=False)
    pathlib.Path(a.output).with_suffix('.json').write_text(json.dumps(report, indent=2)); print(report)


if __name__ == '__main__': main()

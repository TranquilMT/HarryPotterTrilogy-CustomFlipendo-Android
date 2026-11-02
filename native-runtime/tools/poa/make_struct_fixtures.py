"""Extract verified direct primitive struct reads into synthetic return harnesses."""
import argparse
import collections
import json
import pathlib
import struct
from inspect_packages import inspect, Reader
from object_payloads import tags, name, function_header
from bytecode import Decoder
from make_typed_fixtures import KINDS
from package_objects import PackageObjects


def property_tail(data, model, prop):
    r = Reader(data[prop['offset']:prop['offset'] + prop['size']])
    tags(r, model['names']); r.compact(); r.compact()
    dimension, flags = r.u32(), r.u32(); name(r, model['names'])
    if flags & 0x20:
        r.read(2)
    return dimension, r.read(len(r.data) - r.p)


def main():
    p = argparse.ArgumentParser(); p.add_argument('game'); p.add_argument('output'); p.add_argument('--default-roots', action='store_true'); a = p.parse_args()
    access = 2 if a.default_roots else 1
    cases, seen, counts = [], set(), collections.Counter()
    packages = [(path, inspect(path), path.read_bytes()) for path in sorted(pathlib.Path(a.game).rglob('*')) if path.suffix.lower() == '.u']
    catalog = PackageObjects([(path.stem, model, data) for path, model, data in packages])
    for path, model, data in packages:
        for export in model['exports']:
            if export['class'] != 'Function':
                continue
            reader, header = function_header(data[export['offset']:export['offset'] + export['size']], model['names'])
            start = reader.p
            decoder = Decoder(reader, model['names'], len(model['imports']), len(model['exports']), header['declaredBytecodeMemorySize']); decoder.decode()
            for token in decoder.tokens:
                if token['opcode'] != 0x36:
                    continue
                r = Reader(reader.data, start + token['serializedOffset']); begin = r.p
                r.read(1); member = r.compact()
                if r.read(1) != bytes([access]):
                    counts['otherRootOccurrencesDeferred'] += 1
                    continue
                root = r.compact(); identity = (path.name, root, member)
                if identity in seen:
                    continue
                seen.add(identity)
                root_object = catalog.resolve(path.stem, root)
                field_object = catalog.resolve(path.stem, member)
                if root_object is None or field_object is None:
                    counts['unresolvedReferencesDeferred'] += 1
                    continue
                prop, field = root_object['entry'], field_object['entry']
                if prop['class'] != 'StructProperty' or field['class'] not in KINDS:
                    counts['nonPrimitiveLayoutDeferred'] += 1
                    continue
                root_owner = catalog.resolve(root_object['package'], prop['outer'])
                if root_owner is None or root_owner['entry']['class'] != 'Class':
                    counts['nonClassRootDeferred'] += 1
                    continue
                dimension, specific = property_tail(root_object['data'], root_object['model'], prop)
                struct_reader = Reader(specific); struct_ref = struct_reader.compact()
                struct_object = catalog.resolve(root_object['package'], struct_ref)
                field_owner = catalog.resolve(field_object['package'], field['outer'])
                if dimension != 1 or struct_reader.p != len(specific) or struct_object is None or field_owner is None or struct_object['entry']['class'] != 'Struct' or struct_object['identity'] != field_owner['identity']:
                    counts['structOwnershipOrDimensionDeferred'] += 1
                    continue
                dimension, specific = property_tail(field_object['data'], field_object['model'], field)
                if dimension != 1 or specific != (b'\0' if field['class'] == 'ByteProperty' else b''):
                    counts['fieldLayoutDeferred'] += 1
                    continue
                cases.append((b'\x04' + reader.data[begin:r.p], model, root, member, KINDS[field['class']]))
                counts['primitiveReadFragments'] += 1
                if root < 0 or member < 0:
                    counts['importedReadFragments'] += 1
    with open(a.output, 'wb') as out:
        def u32(n): out.write(struct.pack('<I', n & 0xffffffff))
        def blob(b): u32(len(b)); out.write(b)
        u32(len(cases))
        for raw, model, root, member, kind in cases:
            blob(raw); u32(len(model['imports'])); u32(len(model['exports'])); u32(len(model['names']))
            for value in model['names']: blob(value.encode('utf-8'))
            u32(root); u32(member); u32(kind)
    report = dict(counts=counts, rootAccess='default' if a.default_roots else 'instance', scope='Original direct struct-read bytes in synthetic return harnesses; property type/struct ownership checked; whole functions not executed', defaults='Host-supplied primitive leaves; no native offsets or retail defaults', runtimeIntegrated=False)
    pathlib.Path(a.output).with_suffix('.json').write_text(json.dumps(report, indent=2)); print(dict(counts))


if __name__ == '__main__':
    main()

"""Select retail non-null object-property contexts into synthetic return harnesses."""
import argparse
import collections
import json
import pathlib
import struct
from inspect_packages import inspect, Reader
from object_payloads import function_header
from bytecode import Decoder
from make_struct_fixtures import property_tail
from make_typed_fixtures import KINDS
from package_objects import PackageObjects


def ancestor(catalog, cls, owner):
    seen = set()
    while cls:
        if cls['identity'] in seen:
            raise ValueError('Cyclic declared object class')
        seen.add(cls['identity'])
        if cls['identity'] == owner['identity']:
            return True
        cls = catalog.resolve(cls['package'], cls['entry']['superIndex'])
    return False


def main():
    p = argparse.ArgumentParser(); p.add_argument('game'); p.add_argument('output'); a = p.parse_args()
    packages = [(path, inspect(path), path.read_bytes()) for path in sorted(pathlib.Path(a.game).rglob('*')) if path.suffix.lower() == '.u']
    catalog = PackageObjects([(path.stem, model, data) for path, model, data in packages])
    cases, seen, counts = [], set(), collections.Counter()
    for path, model, data in packages:
        for export in model['exports']:
            if export['class'] != 'Function': continue
            reader, header = function_header(data[export['offset']:export['offset'] + export['size']], model['names'])
            start = reader.p
            decoder = Decoder(reader, model['names'], len(model['imports']), len(model['exports']), header['declaredBytecodeMemorySize']); decoder.decode()
            for token in decoder.tokens:
                if token['opcode'] != 0x19: continue
                counts['objectContextOccurrences'] += 1
                r = Reader(reader.data, start + token['serializedOffset']); begin = r.p
                r.read(1)
                if r.read(1) != b'\x01': continue
                root = r.compact(); r.read(3); op = r.read(1); wrapped = op == b'\x2d'
                if wrapped: op = r.read(1)
                if op != b'\x01': continue
                field = r.compact(); identity = (path.name, root, field, wrapped)
                if identity in seen: continue
                seen.add(identity)
                source, leaf = catalog.resolve(path.stem, root), catalog.resolve(path.stem, field)
                if source is None or leaf is None:
                    counts['unresolvedPropertiesDeferred'] += 1; continue
                if source['entry']['class'] != 'ObjectProperty' or leaf['entry']['class'] not in KINDS: continue
                source_owner = catalog.resolve(source['package'], source['entry']['outer'])
                field_owner = catalog.resolve(leaf['package'], leaf['entry']['outer'])
                if source_owner is None or source_owner['entry']['class'] != 'Class' or field_owner is None or field_owner['entry']['class'] != 'Class': continue
                dimension, tail = property_tail(source['data'], source['model'], source['entry'])
                target_reader = Reader(tail); target = catalog.resolve(source['package'], target_reader.compact())
                if dimension != 1 or target_reader.p != len(tail) or target is None or target['entry']['class'] != 'Class' or not ancestor(catalog, target, field_owner):
                    counts['declaredTargetClassDeferred'] += 1; continue
                dimension, tail = property_tail(leaf['data'], leaf['model'], leaf['entry'])
                if dimension != 1 or tail != (b'\0' if leaf['entry']['class'] == 'ByteProperty' else b''): continue
                if wrapped and leaf['entry']['class'] != 'BoolProperty': raise ValueError('Boolean context wrapper type mismatch')
                cases.append((b'\x04' + reader.data[begin:r.p], 1 + token['end'] - token['offset'], model, root, field, KINDS[leaf['entry']['class']]))
                counts['primitiveContextFragments'] += 1
    with open(a.output, 'wb') as out:
        def u32(n): out.write(struct.pack('<I', n & 0xffffffff))
        def blob(b): u32(len(b)); out.write(b)
        u32(len(cases))
        for raw, memory, model, root, field, kind in cases:
            blob(raw); u32(memory); u32(len(model['imports'])); u32(len(model['exports'])); u32(len(model['names']))
            for value in model['names']: blob(value.encode('utf-8'))
            u32(root); u32(field); u32(kind)
    report = dict(counts=counts, scope='Original Context/property bytes in synthetic non-null return harnesses; declared object class ancestry checked; whole retail functions not executed', values='Host-created receiver identities and supplied primitive leaves, not original objects/defaults', runtimeIntegrated=False)
    pathlib.Path(a.output).with_suffix('.json').write_text(json.dumps(report, indent=2)); print(dict(counts))


if __name__ == '__main__': main()

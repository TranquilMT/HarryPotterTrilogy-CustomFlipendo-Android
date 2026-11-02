"""Find retail scalar instance getters; defaults in C++ tests are host supplied."""
import argparse
import collections
import json
import pathlib
import struct
from inspect_packages import inspect, Reader
from object_payloads import function_header, tags, name
from bytecode import Decoder
from make_typed_fixtures import KINDS, supported


def main():
    p = argparse.ArgumentParser()
    p.add_argument('game'); p.add_argument('schemas'); p.add_argument('output')
    p.add_argument('--default-getters', action='store_true', help='Select DefaultVariable instead of InstanceVariable')
    a = p.parse_args()
    access = 2 if a.default_getters else 1
    schemas = {(x['package'], x['exportIndex']): x for x in json.loads(pathlib.Path(a.schemas).read_text())}
    cases, counts = [], collections.Counter()
    for path in sorted(pathlib.Path(a.game).rglob('*')):
        if path.suffix.lower() != '.u':
            continue
        model = inspect(path); data = path.read_bytes()
        for index, export in enumerate(model['exports'], 1):
            if export['class'] != 'Function':
                continue
            schema = schemas[(path.name, index)]
            result = schema['returnProperty']
            if schema['arguments'] or not result or not supported(result):
                continue
            r, header = function_header(data[export['offset']:export['offset'] + export['size']], model['names'])
            start = r.p
            decoder = Decoder(r, model['names'], len(model['imports']), len(model['exports']), header['declaredBytecodeMemorySize'])
            decoder.decode()
            memory = decoder.expanded
            wrapped = memory[:3] == bytes([4, 0x2d, access])
            if wrapped:
                if len(memory) < 7 or result['type'] != 'BoolProperty':
                    continue
                position = 3
            elif memory[:2] == bytes([4, access]) and len(memory) >= 6:
                position = 2
            else:
                continue
            ref = struct.unpack_from('<i', memory, position)[0]
            if not 0 < ref <= len(model['exports']):
                counts['importedOrInvalidPropertyDeferred'] += 1
                continue
            prop = model['exports'][ref - 1]
            if prop['class'] != result['type'] or not 0 < prop['outer'] <= len(model['exports']):
                counts['propertyTypeOrOwnerDeferred'] += 1
                continue
            if model['exports'][prop['outer'] - 1]['class'] != 'Class':
                counts['nonClassPropertyDeferred'] += 1
                continue
            pr = Reader(data[prop['offset']:prop['offset'] + prop['size']])
            tags(pr, model['names']); pr.compact(); pr.compact()
            dimension, flags = pr.u32(), pr.u32(); name(pr, model['names'])
            if flags & 0x20:
                pr.read(2)
            specific = pr.read(len(pr.data) - pr.p)
            if dimension != 1 or specific != (b'\0' if prop['class'] == 'ByteProperty' else b''):
                counts['unsupportedPropertyLayoutDeferred'] += 1
                continue
            cases.append((bytes(r.data[start:r.p]), header['declaredBytecodeMemorySize'],
                          len(model['imports']), len(model['exports']), model['names'], ref, KINDS[prop['class']]))
            counts['retailGetters'] += 1
            if wrapped:
                counts['wrappedBoolGetters'] += 1
    with open(a.output, 'wb') as out:
        def u32(n): out.write(struct.pack('<I', n))
        def blob(b): u32(len(b)); out.write(b)
        u32(len(cases))
        for raw, memory, imports, exports, names, ref, kind in cases:
            blob(raw); u32(memory); u32(imports); u32(exports); u32(len(names))
            for text in names:
                blob(text.encode('utf-8'))
            u32(ref); u32(kind)
    summary = dict(counts=counts, propertyAccess='default' if a.default_getters else 'instance', defaults='Explicit host test values, not retail class defaults',
                   scope='Direct scalar instance getters only; no live objects/Android', runtimeIntegrated=False)
    pathlib.Path(a.output).with_suffix('.json').write_text(json.dumps(summary, indent=2))
    print(dict(counts))


if __name__ == '__main__':
    main()

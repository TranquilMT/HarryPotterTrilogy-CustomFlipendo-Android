"""Extract retail DefaultVariable leaf reads into synthetic return harnesses.

Does not execute whole original functions or load original class defaults.
"""
import argparse
import collections
import json
import pathlib
import struct
from inspect_packages import inspect, Reader
from object_payloads import function_header, tags, name
from bytecode import Decoder
from make_typed_fixtures import KINDS


def main():
    p = argparse.ArgumentParser()
    p.add_argument('game'); p.add_argument('output')
    a = p.parse_args()
    cases, seen, counts = [], set(), collections.Counter()
    for path in sorted(pathlib.Path(a.game).rglob('*')):
        if path.suffix.lower() != '.u':
            continue
        model = inspect(path); data = path.read_bytes()
        for export in model['exports']:
            if export['class'] != 'Function':
                continue
            reader, header = function_header(data[export['offset']:export['offset'] + export['size']], model['names'])
            start = reader.p
            decoder = Decoder(reader, model['names'], len(model['imports']), len(model['exports']), header['declaredBytecodeMemorySize'])
            decoder.decode()
            for token in decoder.tokens:
                if token['opcode'] != 2:
                    continue
                counts['defaultReadOccurrences'] += 1
                ref = token['reference']; identity = (path.name, ref)
                if identity in seen:
                    continue
                seen.add(identity)
                if not 0 < ref <= len(model['exports']):
                    counts['importedPropertiesDeferred'] += 1
                    continue
                prop = model['exports'][ref - 1]
                if prop['class'] not in KINDS or not 0 < prop['outer'] <= len(model['exports']) or model['exports'][prop['outer'] - 1]['class'] != 'Class':
                    counts['nonPrimitiveOrNonClassPropertyDeferred'] += 1
                    continue
                r = Reader(data[prop['offset']:prop['offset'] + prop['size']])
                tags(r, model['names']); r.compact(); r.compact()
                dimension, flags = r.u32(), r.u32(); name(r, model['names'])
                if flags & 0x20:
                    r.read(2)
                specific = r.read(len(r.data) - r.p)
                if dimension != 1 or specific != (b'\0' if prop['class'] == 'ByteProperty' else b''):
                    counts['unsupportedLayoutDeferred'] += 1
                    continue
                leaf = Reader(reader.data, start + token['serializedOffset'])
                begin = leaf.p
                if leaf.read(1) != b'\x02' or leaf.compact() != ref:
                    raise ValueError('Default read serialization mismatch')
                raw = b'\x04' + leaf.data[begin:leaf.p]
                cases.append((raw, model, ref, KINDS[prop['class']]))
                counts['primitiveReadFragments'] += 1
    with open(a.output, 'wb') as out:
        def u32(n): out.write(struct.pack('<I', n))
        def blob(b): u32(len(b)); out.write(b)
        u32(len(cases))
        for raw, model, ref, kind in cases:
            blob(raw); u32(6); u32(len(model['imports'])); u32(len(model['exports'])); u32(len(model['names']))
            for value in model['names']:
                blob(value.encode('utf-8'))
            u32(ref); u32(kind)
    report = dict(counts=counts, scope='Original DefaultVariable leaf bytes in synthetic return harnesses; whole retail functions not executed',
                  defaults='Host-supplied test values; retail default objects not loaded', runtimeIntegrated=False)
    pathlib.Path(a.output).with_suffix('.json').write_text(json.dumps(report, indent=2))
    print(dict(counts))


if __name__ == '__main__':
    main()

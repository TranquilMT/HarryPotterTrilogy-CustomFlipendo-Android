"""Audit PoA v129 struct headers, bytecode and record tails without executing them."""
import argparse
from collections import Counter
from pathlib import Path
from struct import unpack_from

from bytecode import Decoder
from inspect_packages import Reader, inspect
from object_payloads import name, tags


def reference_entry(model, reference):
    if not reference:
        return None
    entries = model['exports'] if reference > 0 else model['imports']
    return entries[abs(reference) - 1]


def audit_delegate_property(reader, model):
    names = model['names']
    tags(reader, names)
    reader.compact()  # BaseField
    reader.compact()  # Next
    reader.u32()  # ArrayDimension
    flags = reader.u32()
    name(reader, names)
    if flags & 0x20:
        reader.read(2)  # ReplicationOffset
    signature = reference_entry(model, reader.compact())
    if signature is None or signature['class'] != 'Function':
        raise ValueError('Invalid delegate signature')
    if reader.p != len(reader.data):
        raise ValueError('Unconsumed delegate declaration bytes')


def audit_delegate_values(reader, model, counts):
    for tag in tags(reader, model['names']):
        if not tag['name'].endswith('__Delegate'):
            continue
        if tag['type'] != 7:
            raise ValueError('Invalid delegate value tag')
        offset = tag['offset']
        value = Reader(reader.data[offset:offset + tag['size']])
        reference_entry(model, value.compact())
        name(value, model['names'])
        if value.p != len(value.data):
            raise ValueError('Unconsumed delegate value bytes')
        counts['Tagged delegate values'] += 1


def audit(system):
    counts = Counter()
    for path in sorted(system.iterdir()):
        if path.suffix.lower() != '.u':
            continue
        model = inspect(path)
        if model['version'] != 129:
            continue
        data = path.read_bytes()
        for export in model['exports']:
            kind = export['class']
            if not export['size']:
                continue
            offset = export['offset']
            reader = Reader(data[offset:offset + export['size']])
            names = model['names']
            try:
                if kind == 'DelegateProperty':
                    audit_delegate_property(reader, model)
                    counts['Delegate declarations'] += 1
                    continue
                if kind not in ('Class', 'Struct', 'State', 'Function'):
                    continue
                if kind != 'Class':
                    tags(reader, names)
                for _ in range(4):
                    reader.compact()
                name(reader, names)
                cpp_text = reader.compact()
                if cpp_text:
                    entries = model['exports'] if cpp_text > 0 else model['imports']
                    target = entries[abs(cpp_text) - 1]
                    if target['class'] != 'TextBuffer' or target['name'] != 'CppText':
                        raise ValueError('Invalid CppText target')
                    counts['CppText references'] += 1
                reader.u32()
                reader.u32()
                script_size = reader.u32()
                if script_size > 64 * 1024 * 1024:
                    raise ValueError('Invalid script length')
                counts[kind] += 1
                if script_size:
                    decoder = Decoder(reader, names, len(model['imports']),
                                      len(model['exports']), script_size)
                    decoder.decode()
                    counts['Script bodies'] += 1
                    for token in decoder.tokens:
                        opcode = token['opcode']
                        if opcode == 0x37:
                            counts['Dynamic-array length tokens'] += 1
                        elif opcode in (0x40, 0x41):
                            counts['Array insertion tokens' if opcode == 0x40 else 'Array removal tokens'] += 1
                        elif opcode == 0x43:
                            reference, = unpack_from('<i', decoder.expanded, token['offset'] + 1)
                            target = reference_entry(model, reference)
                            if target is None or target['class'] != 'DelegateProperty':
                                raise ValueError('Invalid delegate call property')
                            counts['Delegate calls'] += 1
                        elif opcode == 0x45:
                            counts['Delegate assignments'] += 1
                if kind == 'Function':
                    reader.read(3)  # NativeFuncIndex and OperatorPrecedence
                    if reader.u32() & 0x40:
                        reader.read(2)  # ReplicationOffset
                if kind in ('Class', 'State'):
                    reader.read(22)  # UState fields
                if kind == 'Class':
                    reader.read(4 + 16)
                    dependencies = reader.compact()
                    if dependencies < 0 or dependencies > len(reader.data) - reader.p:
                        raise ValueError('Invalid dependency count')
                    for _ in range(dependencies):
                        reader.compact()
                        reader.read(8)
                    imports = reader.compact()
                    if imports < 0 or imports > len(reader.data) - reader.p:
                        raise ValueError('Invalid import count')
                    for _ in range(imports):
                        reader.compact()
                    reader.compact()  # ClassWithin
                    name(reader, names)  # ClassConfigName
                    categories = reader.compact()
                    if categories < 0 or categories > len(reader.data) - reader.p:
                        raise ValueError('Invalid category count')
                    for _ in range(categories):
                        name(reader, names)
                    audit_delegate_values(reader, model, counts)
                    if not script_size:
                        counts['Script-free class tails'] += 1
                if reader.p != len(reader.data):
                    raise ValueError(f'Unconsumed {kind} bytes')
            except (IndexError, ValueError) as exc:
                raise ValueError(f"{path.name}/{export['name']}: {exc}") from exc
    return counts


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('system', type=Path)
    result = audit(parser.parse_args().system)
    if not result:
        parser.error('No PoA v129 records found')
    for label, count in sorted(result.items()):
        print(f'{label}: {count}')

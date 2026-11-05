"""Audit PoA v129 struct headers and script-free class tails without executing them."""
import argparse
from collections import Counter
from pathlib import Path

from inspect_packages import Reader, inspect
from object_payloads import name, tags


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
            if not export['size'] or kind not in ('Class', 'Struct', 'State', 'Function'):
                continue
            offset = export['offset']
            reader = Reader(data[offset:offset + export['size']])
            names = model['names']
            try:
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
                if kind == 'Class' and not script_size:
                    reader.read(22 + 4 + 16)
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
                    tags(reader, names)
                    if reader.p != len(reader.data):
                        raise ValueError('Unconsumed class bytes')
                    counts['Script-free class tails'] += 1
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

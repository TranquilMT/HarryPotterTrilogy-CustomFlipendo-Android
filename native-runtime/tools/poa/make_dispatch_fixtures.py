"""Generate bounded C++ resolver fixtures from dispatch_metadata.py output."""
import argparse
import json
import struct


def main():
    p = argparse.ArgumentParser()
    p.add_argument('metadata')
    p.add_argument('output')
    a = p.parse_args()
    m = json.load(open(a.metadata))
    if not m['complete']:
        raise ValueError('Unresolved metadata cannot produce complete fixtures')
    scopes = m['scopes']
    ids = {name: i for i, name in enumerate(sorted(scopes), 1)}
    functions = {name: i for i, name in enumerate(sorted({f for s in scopes.values()
                 for f in s['functions'].values()}), 1)}
    checks = []

    def lookup(scope, name):
        while scope:
            s = scopes[scope]
            if name in s['functions']:
                return functions[s['functions'][name]]
            scope = s['parent']
        return 0

    for identity, s in scopes.items():
        cls = identity if s['kind'] == 'Class' else s['owner']
        state = identity if s['kind'] == 'State' else None
        names = set()
        for start in (cls, state):
            while start:
                names.update(scopes[start]['functions'])
                start = scopes[start]['parent']
        for name in sorted(names):
            class_target = lookup(cls, name)
            virtual_target = (lookup(state, name) if state else 0) or class_target
            checks.append((ids[cls], ids.get(state, 0), name, virtual_target, class_target))
    with open(a.output, 'wb') as out:
        def number(n):
            out.write(struct.pack('<I', n))
        def text(s):
            b = s.encode('ascii'); number(len(b)); out.write(b)
        number(len(scopes))
        for identity, s in scopes.items():
            number(s['kind'] == 'State'); number(ids[identity]); number(ids.get(s['parent'], 0))
            number(len(s['functions']))
            for name, target in s['functions'].items():
                text(name); number(functions[target])
        number(len(checks))
        for cls, state, name, virtual, glob in checks:
            number(cls); number(state); text(name); number(virtual); number(glob)
    print('Scopes', len(scopes), 'functions', len(functions), 'lookup cases', len(checks))


if __name__ == '__main__':
    main()

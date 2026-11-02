"""Audit lexical self-call lookup; explicit receiver calls remain deferred.

A successful lookup is not dynamic receiver compatibility or executable code.
"""
import argparse
import collections
import json
import pathlib
from inspect_packages import inspect
from object_payloads import function_header
from bytecode import Decoder
from dispatch_metadata import build_metadata, key

# Conservatively defer calls nested inside class/object contexts or delegates.
CONTEXT_OPS = {0x12, 0x19, 0x43}


def lookup(scopes, start, name):
    seen = set()
    while start:
        if start in seen:
            raise ValueError('Cyclic lookup ancestry')
        seen.add(start)
        if start not in scopes:
            return None
        scope = scopes[start]
        if name in scope['functions']:
            return scope['functions'][name]
        start = scope['parent']
    return None


def classify_call(scopes, owner, token, tokens):
    offset = token['offset']
    if any(t['opcode'] in CONTEXT_OPS and t['offset'] < offset < t['end']
           for t in tokens):
        return 'deferredReceiverContext', None
    scope = scopes[owner]
    cls = owner if scope['kind'] == 'Class' else scope['owner']
    name = key(token['functionName'])
    target = None
    if token['opcode'] == 0x1b and scope['kind'] == 'State':
        target = lookup(scopes, owner, name)
    target = target or lookup(scopes, cls, name)
    return ('lexicalTargetFound' if target else 'requiresDynamicReceiverOrMissing'), target


def audit(paths):
    models = [(p, inspect(p)) for p in paths]
    metadata = build_metadata([(p.stem, m) for p, m in models])
    counts = collections.Counter()
    packages = []
    errors = []
    for p, m in models:
        data = p.read_bytes()
        package_counts = collections.Counter()
        for e in m['exports']:
            if e['class'] != 'Function':
                continue
            try:
                owner_parts = []
                ref = e['outer']
                seen = set()
                while ref:
                    if ref in seen or not 0 < ref <= len(m['exports']):
                        raise ValueError('Invalid function owner chain')
                    seen.add(ref)
                    outer = m['exports'][ref - 1]
                    owner_parts.append(key(outer['name']))
                    ref = outer['outer']
                owner = key(p.stem) + '.' + '.'.join(reversed(owner_parts))
                r, header = function_header(data[e['offset']:e['offset'] + e['size']], m['names'])
                decoder = Decoder(r, m['names'], len(m['imports']), len(m['exports']),
                                  header['declaredBytecodeMemorySize'])
                decoder.decode()
                r.read(2); r.read(1); flags = r.u32()
                if flags & 0x40:
                    r.read(2)
                if r.p != len(r.data):
                    raise ValueError('Unconsumed function export bytes')
                package_counts['decodedFunctions'] += 1
                for token in decoder.tokens:
                    if token['opcode'] not in (0x1b, 0x38):
                        continue
                    classification, _ = classify_call(metadata['scopes'], owner, token, decoder.tokens)
                    package_counts[classification] += 1
            except Exception as error:
                errors.append(dict(package=p.name, function=e['name'], error=str(error)))
        counts.update(package_counts)
        packages.append(dict(package=p.name, sha256=m['sha256'], counts=dict(package_counts)))
    return dict(counts=dict(counts), packages=packages, errors=errors,
                unresolvedMetadataRelationships=len(metadata['unresolved']), runtimeIntegrated=False,
                scope='Lexical scope lookup only; dynamic receiver/state changes and gameplay unverified')


def main():
    p = argparse.ArgumentParser()
    p.add_argument('game'); p.add_argument('output')
    a = p.parse_args()
    paths = sorted(p for p in pathlib.Path(a.game).rglob('*') if p.suffix.lower() == '.u')
    result = audit(paths)
    pathlib.Path(a.output).write_text(json.dumps(result, indent=2))
    print(result['counts'], 'errors', len(result['errors']))


if __name__ == '__main__':
    main()

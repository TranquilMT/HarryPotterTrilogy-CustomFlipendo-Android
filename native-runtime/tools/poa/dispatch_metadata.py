"""Resolve dispatch ownership/inheritance from UE2 import/export tables.

Does not deserialize class defaults, state bytecode, or instantiate receivers.
"""
import argparse
import json
import pathlib
from inspect_packages import inspect


def key(name):
    if not name or not name.isascii():
        raise ValueError('Unverified dispatch identifier')
    return name.lower()


def build_metadata(packages):
    objects = {}
    references = {}
    for package, model in packages:
        package_key = key(package)
        if package_key in references:
            raise ValueError('Duplicate package')
        paths = {0: package_key}
        active = set()

        def path(ref):
            if ref in paths:
                return paths[ref]
            if ref in active:
                raise ValueError('Cyclic object ownership')
            if len(active) >= 128:
                raise ValueError('Excessive object ownership depth')
            table = model['exports'] if ref > 0 else model['imports']
            index = ref - 1 if ref > 0 else -ref - 1
            if not 0 <= index < len(table):
                raise ValueError('Object reference outside table')
            active.add(ref)
            obj = table[index]
            outer = obj['outer']
            # A root import names the external package, not the current package.
            prefix = path(outer) if outer else (package_key if ref > 0 else '')
            result = (prefix + '.' if prefix else '') + key(obj['name'])
            paths[ref] = result
            active.remove(ref)
            return result

        for ref in range(-len(model['imports']), len(model['exports']) + 1):
            path(ref)
        references[package_key] = paths
        for ref, obj in enumerate(model['exports'], 1):
            if obj['class'] not in ('Class', 'State', 'Function'):
                continue
            identity = path(ref)
            if identity in objects:
                raise ValueError('Duplicate object identity: ' + identity)
            objects[identity] = dict(obj, identity=identity, package=package_key,
                                     exportIndex=ref, owner=path(obj['outer']),
                                     parent=path(obj['superIndex']) if obj['superIndex'] else None)

    scopes = {}
    unresolved = []
    for identity, obj in objects.items():
        kind = obj['class']
        if kind not in ('Class', 'State', 'Function'):
            continue
        owner = objects.get(obj['owner'])
        if kind == 'State' and (not owner or owner['class'] != 'Class'):
            raise ValueError('State owner is not a class: ' + identity)
        if kind == 'Function' and (not owner or owner['class'] not in ('Class', 'State')):
            raise ValueError('Function owner is not a dispatch scope: ' + identity)
        parent = obj['parent']
        if parent and parent not in objects:
            unresolved.append(dict(object=identity, target=parent, relation='super'))
        elif parent and objects[parent]['class'] != kind:
            raise ValueError('Inheritance kind mismatch: ' + identity)
        if kind in ('Class', 'State'):
            scopes[identity] = dict(kind=kind, owner=obj['owner'], parent=parent, functions={})

    for identity, obj in objects.items():
        if obj['class'] == 'Function':
            functions = scopes[obj['owner']]['functions']
            name = key(obj['name'])
            if name in functions:
                raise ValueError('Duplicate dispatch function')
            functions[name] = identity

    for identity, scope in scopes.items():
        seen = set()
        current = identity
        while current in scopes:
            if current in seen:
                raise ValueError('Cyclic scope inheritance: ' + identity)
            seen.add(current)
            current = scopes[current]['parent']
        if scope['kind'] == 'State' and scope['parent'] in scopes:
            owner = scope['owner']
            permitted = set()
            while owner in scopes:
                if owner in permitted:
                    raise ValueError('Cyclic class inheritance')
                permitted.add(owner)
                owner = scopes[owner]['parent']
            if scopes[scope['parent']]['owner'] not in permitted:
                if owner:
                    unresolved.append(dict(object=identity, target=owner,
                                           relation='stateOwnershipAncestor'))
                else:
                    raise ValueError('State super belongs to unrelated class: ' + identity)

    return dict(scopes=scopes, unresolved=unresolved,
                complete=not unresolved, runtimeSupported=False)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('game')
    parser.add_argument('output')
    args = parser.parse_args()
    paths = sorted(p for p in pathlib.Path(args.game).rglob('*') if p.suffix.lower() == '.u')
    models = [(p.stem, inspect(p)) for p in paths]
    result = build_metadata(models)
    result['packages'] = [dict(name=p.name, sha256=m['sha256'], version=m['version'])
                          for p, (_, m) in zip(paths, models)]
    pathlib.Path(args.output).write_text(json.dumps(result, indent=2))
    print('Scopes', len(result['scopes']), 'unresolved metadata relationships', len(result['unresolved']))


if __name__ == '__main__':
    main()

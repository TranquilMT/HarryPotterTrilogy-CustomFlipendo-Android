"""Resolve package object identities across inspected tables, preserving class kind."""
class PackageObjects:
    def __init__(self, packages):
        self.packages = {}
        self.exports = {}
        for name, model, data in packages:
            key = name.lower()
            if key in self.packages:
                raise ValueError('Duplicate package identity')
            self.packages[key] = dict(model=model, data=data, paths={})
        for package, context in self.packages.items():
            for index, entry in enumerate(context['model']['exports'], 1):
                key = (self.path(package, index), entry['class'].lower())
                if key in self.exports:
                    raise ValueError('Ambiguous object identity and kind')
                self.exports[key] = dict(context, entry=entry, package=package, index=index, identity=key)

    def path(self, package, reference, active=None):
        package = package.lower()
        context = self.packages[package]
        if reference in context['paths']:
            return context['paths'][reference]
        active = set() if active is None else active
        if reference in active or len(active) >= 128:
            raise ValueError('Cyclic or excessive object ownership')
        table = context['model']['exports'] if reference > 0 else context['model']['imports']
        index = reference - 1 if reference > 0 else -reference - 1
        if not reference or not 0 <= index < len(table):
            raise ValueError('Object reference outside package tables')
        active.add(reference)
        entry = table[index]
        prefix = self.path(package, entry['outer'], active) if entry['outer'] else (package if reference > 0 else '')
        result = (prefix + '.' if prefix else '') + entry['name'].lower()
        active.remove(reference)
        context['paths'][reference] = result
        return result

    def resolve(self, package, reference):
        if not reference:
            return None
        package = package.lower()
        model = self.packages[package]['model']
        table = model['exports'] if reference > 0 else model['imports']
        index = reference - 1 if reference > 0 else -reference - 1
        if not 0 <= index < len(table):
            raise ValueError('Object reference outside package tables')
        entry = table[index]
        return self.exports.get((self.path(package, reference), entry['class'].lower()))

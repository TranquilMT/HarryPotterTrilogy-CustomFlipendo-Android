import unittest
from package_objects import PackageObjects


def obj(name, kind='Class', outer=0):
    return dict(name=name, **{'class': kind}, outer=outer)


def model(exports=(), imports=()):
    return dict(exports=list(exports), imports=list(imports))


class PackageObjectTests(unittest.TestCase):
    def test_imported_struct_field(self):
        a = model([obj('Vector', 'Struct'), obj('X', 'FloatProperty', 1)])
        b = model(imports=[obj('Core', 'Package'), obj('Vector', 'Struct', -1), obj('X', 'FloatProperty', -2)])
        catalog = PackageObjects([('Core', a, b'a'), ('Game', b, b'b')])
        field = catalog.resolve('GAME', -3)
        self.assertEqual(field['identity'], ('core.vector.x', 'floatproperty'))
        self.assertEqual(field['data'], b'a')

    def test_same_path_different_kind(self):
        catalog = PackageObjects([('P', model([obj('A'), obj('A', 'Texture')]), b'')])
        self.assertEqual(catalog.resolve('p', 1)['entry']['class'], 'Class')
        self.assertEqual(catalog.resolve('p', 2)['entry']['class'], 'Texture')

    def test_missing_import_is_not_success(self):
        catalog = PackageObjects([('P', model(imports=[obj('Absent', 'Package'), obj('A', outer=-1)]), b'')])
        self.assertIsNone(catalog.resolve('p', -2))

    def test_invalid_reference(self):
        catalog = PackageObjects([('P', model(), b'')])
        with self.assertRaisesRegex(ValueError, 'outside package'):
            catalog.resolve('p', -1)
        self.assertIsNone(catalog.resolve('p', 0))

    def test_cycle(self):
        with self.assertRaisesRegex(ValueError, 'Cyclic'):
            PackageObjects([('P', model([obj('A', outer=1)]), b'')])

    def test_duplicate_kind_or_package(self):
        with self.assertRaisesRegex(ValueError, 'Ambiguous'):
            PackageObjects([('P', model([obj('A'), obj('a')]), b'')])
        with self.assertRaisesRegex(ValueError, 'Duplicate package'):
            PackageObjects([('P', model(), b''), ('p', model(), b'')])


if __name__ == '__main__':
    unittest.main()

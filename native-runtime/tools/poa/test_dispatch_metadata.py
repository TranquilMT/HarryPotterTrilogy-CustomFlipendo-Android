import unittest
from dispatch_metadata import build_metadata


def obj(name, kind='Class', outer=0, parent=0):
    return dict(name=name, **{'class': kind}, outer=outer, superIndex=parent)


def package(exports, imports=()):
    return dict(exports=exports, imports=list(imports))


class DispatchMetadataTests(unittest.TestCase):
    def test_cross_package_inheritance(self):
        core = package([obj('Object'), obj('Compute', 'Function', 1)])
        engine = package([obj('Actor', parent=-2), obj('Idle', 'State', 1),
                          obj('Compute', 'Function', 2)],
                         [obj('Core', 'Package'), obj('Object', outer=-1)])
        result = build_metadata([('Core', core), ('Engine', engine)])
        self.assertTrue(result['complete'])
        self.assertEqual(result['scopes']['engine.actor']['parent'], 'core.object')
        self.assertEqual(result['scopes']['engine.actor.idle']['functions'],
                         {'compute': 'engine.actor.idle.compute'})

    def test_asset_and_class_can_share_path(self):
        result = build_metadata([('P', package([obj('A', 'Texture'), obj('A')]))])
        self.assertEqual(list(result['scopes']), ['p.a'])

    def test_unresolved_is_explicit(self):
        result = build_metadata([('P', package([obj('A', parent=-2)],
                   [obj('Missing', 'Package'), obj('Base', outer=-1)]))])
        self.assertFalse(result['complete'])
        self.assertEqual(result['unresolved'][0]['target'], 'missing.base')

    def test_bad_outer(self):
        with self.assertRaisesRegex(ValueError, 'outside table'):
            build_metadata([('P', package([obj('A', outer=4)]))])

    def test_cyclic_outer(self):
        with self.assertRaisesRegex(ValueError, 'Cyclic object'):
            build_metadata([('P', package([obj('A', outer=1)]))])

    def test_cyclic_class_super(self):
        with self.assertRaisesRegex(ValueError, 'Cyclic scope'):
            build_metadata([('P', package([obj('A', parent=2), obj('B', parent=1)]))])

    def test_kind_mismatch(self):
        with self.assertRaisesRegex(ValueError, 'kind mismatch'):
            build_metadata([('P', package([obj('A'), obj('S', 'State', 1, 1)]))])

    def test_bad_function_owner(self):
        with self.assertRaisesRegex(ValueError, 'Function owner'):
            build_metadata([('P', package([obj('Fn', 'Function')]))])

    def test_bad_state_owner(self):
        with self.assertRaisesRegex(ValueError, 'State owner'):
            build_metadata([('P', package([obj('S', 'State')]))])

    def test_unrelated_state_super(self):
        with self.assertRaisesRegex(ValueError, 'unrelated class'):
            build_metadata([('P', package([obj('A'), obj('B'),
                       obj('Idle', 'State', 1), obj('Idle', 'State', 2, 3)]))])

    def test_inherited_state_super(self):
        result = build_metadata([('P', package([obj('A'), obj('B', parent=1),
                     obj('Idle', 'State', 1), obj('Idle', 'State', 2, 3)]))])
        self.assertTrue(result['complete'])
        self.assertEqual(result['scopes']['p.b.idle']['parent'], 'p.a.idle')

    def test_missing_native_ancestor_defers_state_ownership(self):
        model = package([obj('Base'), obj('NativeChild', parent=-2),
                         obj('Idle', 'State', 1), obj('Idle', 'State', 2, 3)],
                        [obj('Native', 'Package'), obj('Parent', outer=-1)])
        result = build_metadata([('P', model)])
        self.assertFalse(result['complete'])
        self.assertIn('stateOwnershipAncestor', [x['relation'] for x in result['unresolved']])

    def test_case_collision(self):
        with self.assertRaisesRegex(ValueError, 'Duplicate object'):
            build_metadata([('P', package([obj('A'), obj('a')]))])

    def test_package_collision(self):
        with self.assertRaisesRegex(ValueError, 'Duplicate package'):
            build_metadata([('P', package([])), ('p', package([]))])

    def test_non_ascii(self):
        with self.assertRaisesRegex(ValueError, 'Unverified'):
            build_metadata([('P', package([obj('é')]))])


if __name__ == '__main__':
    unittest.main()

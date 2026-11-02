import copy
import hashlib
from pathlib import Path
import tempfile
import unittest
import zlib

from test_startup_profile import package
from verify_game_data import dependency_graph, verify_packages


class GameDataVerificationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.path = self.root / 'System/Core.u'
        package(self.path, [('Class', 'Object')])
        data = self.path.read_bytes()
        self.row = {'path': 'system/core.u', 'size': len(data), 'source': 'archive', 'crc32': zlib.crc32(data)}
        self.inventory = {'files': [self.row]}
        self.sha = hashlib.sha256(data).hexdigest()

    def test_complete_data_has_real_hash_and_preserved_actual_path(self):
        report = verify_packages(self.root, self.inventory, roots=['CORE'])
        self.assertTrue(report['requiredPackagesComplete'])
        self.assertEqual(report['packages']['core']['sha256'], self.sha)
        self.assertEqual(report['packages']['core']['actualPath'], 'System/Core.u')
        self.assertTrue(report['dependencyRoutes']['CORE']['staticClosureComplete'])
        self.assertFalse(report['dependencyRoutes']['CORE']['dynamicLoadsVerified'])
        self.assertFalse(report['playable'])

    def test_missing_package_does_not_invalidate_present_files_or_claim_completeness(self):
        self.inventory['files'].append(dict(self.row, path='textures/Absent.utx'))
        report = verify_packages(self.root, self.inventory, roots=['Absent'])
        self.assertTrue(report['presentPayloadsValid'])
        self.assertFalse(report['requiredPackagesComplete'])
        self.assertEqual(report['dependencyRoutes']['Absent']['missingPayloads'], ['absent'])

    def test_same_size_corruption_is_caught_before_table_inspection(self):
        data = bytearray(self.path.read_bytes())
        data[-1] ^= 1
        self.path.write_bytes(data)
        report = verify_packages(self.root, self.inventory)
        self.assertIn('CRC differs', report['packages']['core']['error'])
        self.assertFalse(report['presentPayloadsValid'])

    def test_pinned_sha_is_checked_even_with_matching_declared_crc(self):
        report = verify_packages(self.root, self.inventory, {'system/core.u': '0' * 64})
        self.assertIn('pinned reference SHA-256', report['packages']['core']['error'])

    def test_standalone_without_integrity_anchor_remains_unverified(self):
        self.row.pop('crc32')
        self.row['source'] = 'disc'
        unanchored = verify_packages(self.root, self.inventory)
        self.assertEqual(unanchored['packages']['core']['status'], 'unanchored')
        self.assertFalse(unanchored['requiredPackagesComplete'])
        verified = verify_packages(self.root, self.inventory, {'system/core.u': self.sha})
        self.assertTrue(verified['requiredPackagesComplete'])

    def test_crc_match_does_not_make_malformed_package_valid(self):
        self.path.write_bytes(b'X' * self.row['size'])
        self.row['crc32'] = zlib.crc32(self.path.read_bytes())
        report = verify_packages(self.root, self.inventory)
        self.assertIn('Invalid package signature', report['packages']['core']['error'])

    def test_ambiguous_package_basenames_and_unsafe_inventory_paths_are_rejected(self):
        for name in ['Textures/Core.utx', '../Core.u']:
            inventory = copy.deepcopy(self.inventory)
            inventory['files'].append(dict(self.row, path=name))
            with self.subTest(name=name), self.assertRaises(ValueError):
                verify_packages(self.root, inventory)

    def test_unlisted_packages_prevent_a_complete_data_claim(self):
        package(self.root / 'System/Unexpected.u', [('Class', 'Extra')])
        report = verify_packages(self.root, self.inventory)
        self.assertEqual(report['unlistedPackageFiles'], ['System/Unexpected.u'])
        self.assertFalse(report['requiredPackagesComplete'])

    def test_dependency_cycles_terminate_and_missing_transitive_nodes_are_reported(self):
        nodes = {'a': {'status': 'valid', 'rootDependencies': ['b']},
                 'b': {'status': 'valid', 'rootDependencies': ['a', 'c']},
                 'c': {'status': 'missing', 'rootDependencies': []}}
        report = dependency_graph(nodes, ['A'])['A']
        self.assertEqual(report['visitedPackages'], ['a', 'b', 'c'])
        self.assertEqual(report['missingPayloads'], ['c'])
        self.assertFalse(report['staticClosureComplete'])
        nodes['c'] = {'status': 'valid', 'rootDependencies': []}
        self.assertTrue(dependency_graph(nodes, ['a'])['a']['staticClosureComplete'])

    def test_dependency_outside_inventory_is_not_silently_treated_as_builtin(self):
        report = dependency_graph({'a': {'status': 'valid', 'rootDependencies': ['nativeunknown']}}, ['a'])['a']
        self.assertEqual(report['dependenciesOutsideInventory'], ['nativeunknown'])
        self.assertFalse(report['staticClosureComplete'])


if __name__ == '__main__':
    unittest.main()

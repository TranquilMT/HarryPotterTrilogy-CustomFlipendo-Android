import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from build_preflight import check_inputs, source_identity


class SourcePreflightTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.project = Path(self.temp.name)
        self.native = self.project / 'native-runtime'
        self.builder = self.native / 'android/build_linux.py'
        self.builder.parent.mkdir(parents=True)
        self.builder.write_bytes(b'fixture source')
        (self.project / 'docs').mkdir()
        self.lock = self.project / 'docs/POA-SOURCE-LOCK.json'
        self.manifest = {'checkpoint': 'test-source', 'files': {
            'native-runtime/android/build_linux.py': {'bytes': 14, 'sha256': hashlib.sha256(b'fixture source').hexdigest()}}}
        self.lock.write_text(json.dumps(self.manifest))

    @patch('build_preflight.shutil.which', return_value=None)
    def test_source_archive_can_supply_verified_identity_without_git(self, _):
        identity = source_identity(self.native)
        self.assertTrue(identity.startswith('checkpoint:test-source;lock-sha256:'))
        self.assertIn(hashlib.sha256(self.lock.read_bytes()).hexdigest(), identity)

    @patch('build_preflight.shutil.which', return_value=None)
    def test_modified_source_cannot_keep_old_checkpoint_identity(self, _):
        self.builder.write_bytes(b'changed source')
        with self.assertRaisesRegex(ValueError, 'differs from checkpoint'):
            source_identity(self.native)

    @patch('build_preflight.shutil.which', return_value=None)
    def test_missing_lock_does_not_fabricate_source_revision(self, _):
        self.lock.unlink()
        with self.assertRaisesRegex(ValueError, 'No Git revision'):
            source_identity(self.native)

    @patch('build_preflight.shutil.which', return_value=None)
    def test_manifest_cannot_reference_outside_source_tree(self, _):
        self.manifest['files']['../outside'] = self.manifest['files']['native-runtime/android/build_linux.py']
        self.lock.write_text(json.dumps(self.manifest))
        with self.assertRaisesRegex(ValueError, 'Unsafe source checkpoint path'):
            source_identity(self.native)

    @patch('build_preflight.shutil.which', return_value=None)
    def test_missing_inputs_are_reported_together_without_building(self, _):
        report = check_inputs(self.native, {})
        self.assertTrue(next(c for c in report['checks'] if c['input'] == 'sourceIdentity')['ready'])
        missing = {c['input'] for c in report['checks'] if not c['ready']}
        self.assertTrue({'keystore', 'password_file', 'previous_apk', 'hp1_data', 'hp2_data', 'cmake', 'ninja'} <= missing)
        self.assertFalse(report['allPreflightChecksPassed'])
        self.assertFalse(report['apkBuilt'])


if __name__ == '__main__':
    unittest.main()

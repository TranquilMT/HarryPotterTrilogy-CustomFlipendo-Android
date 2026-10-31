import io
import tempfile
import unittest
import zipfile
from pathlib import Path
from release_checks import SYSTEM, hp2_inventory, verify_embedded_hp2

class ReleaseChecks(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        files = ['System/' + n for n in SYSTEM] + ['Sounds/AllDialog.uax', 'Maps/Test.unr', 'Textures/Test.utx']
        for name in files:
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(bytes.fromhex('c1832a9e4c000000'))
    def tearDown(self):
        self.temp.cleanup()
    def apk(self, inventory, extra=None, corrupt=None):
        data = io.BytesIO()
        with zipfile.ZipFile(data, 'w') as z:
            for row in inventory:
                z.writestr('assets/hp2data/' + row['path'], b'bad' if row['path'] == corrupt else (self.root / row['path']).read_bytes())
            if extra:
                z.writestr(extra, b'bad')
        return zipfile.ZipFile(io.BytesIO(data.getvalue()))
    def test_complete_inventory_survives_packaging(self):
        inventory = hp2_inventory(self.root)
        with self.apk(inventory) as apk:
            verify_embedded_hp2(apk, inventory)
    def test_missing_game_data_rejected(self):
        (self.root / 'Sounds/AllDialog.uax').unlink()
        with self.assertRaises(ValueError): hp2_inventory(self.root)
    def test_ue2_is_not_hp2(self):
        (self.root / 'System/HGame.u').write_bytes(bytes.fromhex('c1832a9e81000000'))
        with self.assertRaises(ValueError): hp2_inventory(self.root)
    def test_partial_recovery_rejected(self):
        (self.root / 'System/Core.u').write_bytes(b'')
        with self.assertRaises(ValueError): hp2_inventory(self.root)
    def test_case_collision_rejected(self):
        (self.root / 'System/core.u').write_bytes(b'bad')
        with self.assertRaises(ValueError): hp2_inventory(self.root)
    def test_hd_and_private_key_never_packaged(self):
        inventory = hp2_inventory(self.root)
        for name in ['assets/bundled-hd/hp1/pack.zip', 'assets/texpacks/pack.zip', 'assets/release.jks', 'assets/password.txt']:
            with self.subTest(name=name), self.apk(inventory, extra=name) as apk:
                with self.assertRaises(ValueError): verify_embedded_hp2(apk, inventory)
    def test_corrupt_embedded_file_rejected(self):
        inventory = hp2_inventory(self.root)
        with self.apk(inventory, corrupt='System/HGame.u') as apk:
            with self.assertRaises(ValueError): verify_embedded_hp2(apk, inventory)
    def test_missing_embedded_file_rejected(self):
        inventory = hp2_inventory(self.root)
        with self.apk(inventory[:-1]) as apk:
            with self.assertRaises(ValueError): verify_embedded_hp2(apk, inventory)

if __name__ == '__main__': unittest.main()

import io
import tempfile
import unittest
import zipfile
from pathlib import Path
from release_checks import hp1_inventory, hp2_inventory, verify_bundled_games


class BothGamesTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.roots = [Path(self.temp.name) / name for name in ('hp1', 'hp2')]
        header = bytes.fromhex('c1832a9e4c000000')
        for root, classes in zip(self.roots, [
            ('HP.exe', 'Core.u', 'Engine.u', 'HarryPotter.u', 'HPBase.u', 'HPPuzzle.u', 'HPMenu.u'),
            ('Game.exe', 'Core.u', 'Engine.u', 'HGame.u', 'HProps.u', 'HPModels.u', 'HPSounds.u'),
        ]):
            for name in [*(f'System/{c}' for c in classes), 'System/Default.ini', 'System/DefUser.ini',
                         'Maps/Test.unr', 'Textures/Test.utx', 'Sounds/AllDialog.uax']:
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(header)
        self.rows = [hp1_inventory(self.roots[0]), hp2_inventory(self.roots[1])]

    def tearDown(self):
        self.temp.cleanup()

    def apk(self, omit=None):
        data = io.BytesIO()
        with zipfile.ZipFile(data, 'w') as apk:
            for root, prefix, rows in zip(self.roots, ('assets/gamedata/', 'assets/hp2data/'), self.rows):
                if prefix == omit:
                    continue
                for row in rows:
                    apk.write(root / row['path'], prefix + row['path'])
                apk.writestr(prefix + '.stamp', '0' * 64)
        return zipfile.ZipFile(data)

    def test_both_payloads_pass(self):
        with self.apk() as apk:
            verify_bundled_games(apk, *self.rows)

    def test_either_missing_game_fails(self):
        for prefix in ('assets/gamedata/', 'assets/hp2data/'):
            with self.subTest(prefix=prefix), self.apk(omit=prefix) as apk:
                with self.assertRaises(ValueError):
                    verify_bundled_games(apk, *self.rows)

    def test_engine_only_fails(self):
        data = io.BytesIO()
        with zipfile.ZipFile(data, 'w') as apk:
            apk.writestr('lib/arm64-v8a/libmain.so', b'engine')
        with zipfile.ZipFile(data) as apk, self.assertRaises(ValueError):
            verify_bundled_games(apk, *self.rows)

    def test_missing_configuration_fails(self):
        rows = [row for row in self.rows[0] if row['path'] != 'System/Default.ini']
        with self.apk() as apk, self.assertRaises(ValueError):
            verify_bundled_games(apk, rows, self.rows[1])

    def test_missing_hp1_package_fails_before_packaging(self):
        (self.roots[0] / 'System/Engine.u').unlink()
        with self.assertRaises(ValueError):
            hp1_inventory(self.roots[0])


if __name__ == '__main__':
    unittest.main()

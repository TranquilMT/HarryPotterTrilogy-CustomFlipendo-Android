import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from startup_profile import audit_startup, file_index, map_path, one, profile_identity, read_ini


def package(path, entries):
    """Minimal package tables for original-config routing tests; no engine payloads."""
    names = list(dict.fromkeys(['Engine', 'Class'] + [x for pair in entries for x in pair]))
    def compact(value):
        if not -63 <= value <= 63:
            raise ValueError('Test fixture compact index out of range')
        return bytes([(128 if value < 0 else 0) | abs(value)])
    name_bytes = b''.join(compact(len(n) + 1) + n.encode() + b'\0' + struct.pack('<I', 0) for n in names)
    kinds = list(dict.fromkeys(kind for kind, _ in entries if kind != 'Class'))
    imports = b''.join(compact(names.index('Engine')) + compact(names.index('Class')) + struct.pack('<i', 0) + compact(names.index(k)) for k in kinds)
    exports = b''
    for kind, name in entries:
        ref = 0 if kind == 'Class' else -kinds.index(kind) - 1
        exports += compact(ref) + compact(0) + struct.pack('<i', 0) + compact(names.index(name)) + struct.pack('<I', 0) + compact(0)
    header = struct.pack('<IHH7I', 0x9e2a83c1, 129, 0, 0, len(names), 36,
                         len(entries), 36 + len(name_bytes) + len(imports), len(kinds), 36 + len(name_bytes))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + name_bytes + imports + exports)


class StartupProfileTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        system = self.root / 'System'
        package(system / 'HGame.u', [('Class', n) for n in ['HGame', 'HPGUIController', 'HPWorldInfo', 'HarryController', 'HPHUD']])
        package(system / 'kwGame.u', [('Class', 'KWConsole')])
        for name in ['Entry.unr', 'First.unr', 'Alternate.unr']:
            package(self.root / 'Maps' / name, [('Level', 'MyLevel'), ('PlayerStart', 'Start')])
        (system / 'hppoa.exe').write_bytes(b'fixture executable identity only')
        (system / 'DefUser.ini').write_text('[Engine.Input]\nLeftMouse=Cast\nSpace=Jump\nEnter=Use\nTab=Map\nEscape=Menu\nMiddleMouse=Zoom\nShift=Walk\n')
        self.config = ('[URL]\nMap=Unused.unr\nLocalMap=entry.unr\n'
                       '[Engine.Engine]\nDefaultGame=HGame.HGame\nConsole=KWGame.KWConsole\n'
                       'GUIController=HGame.HPGUIController\nWorldInfo=HGame.HPWorldInfo\n'
                       'LevelLoadingPlayerControllerClass=HGame.HarryController\n'
                       'LevelLoadingHUDClass=HGame.HPHUD\nLanguage=int\n'
                       '[HGame.HPPageFrontend]\nFirstLevel=First.unr\n'
                       '[Engine.GameEngine]\n;ServerPackages=AbsentEditor\nServerPackages=HGame\n')
        self.write_config()

    def write_config(self, value=None):
        (self.root / 'System' / 'Default.ini').write_text(self.config if value is None else value)

    def test_uses_localmap_and_original_firstlevel_preserving_actual_case(self):
        report = audit_startup(self.root)
        self.assertEqual(report['route']['startupMap'], 'entry.unr')
        self.assertEqual(report['maps']['startupMap']['path'], 'Maps/Entry.unr')
        self.assertEqual(report['route']['firstStoryMap'], 'First.unr')
        self.assertEqual(report['serverPackages'], ['HGame'])
        self.assertFalse(report['liveStartupVerified'])

    def test_changed_configuration_changes_selected_map(self):
        self.write_config(self.config.replace('FirstLevel=First.unr', 'FirstLevel=Alternate.unr'))
        self.assertEqual(audit_startup(self.root)['route']['firstStoryMap'], 'Alternate.unr')

    def test_missing_original_map_is_rejected(self):
        (self.root / 'Maps' / 'First.unr').unlink()
        with self.assertRaisesRegex(ValueError, 'Required original file missing'):
            audit_startup(self.root)

    def test_missing_or_wrong_kind_startup_class_is_rejected(self):
        package(self.root / 'System' / 'kwGame.u', [('Function', 'KWConsole')])
        with self.assertRaisesRegex(ValueError, 'Startup class unavailable'):
            audit_startup(self.root)

    def test_configured_server_package_must_exist(self):
        self.write_config(self.config.replace('ServerPackages=HGame', 'ServerPackages=Absent'))
        with self.assertRaisesRegex(ValueError, 'server package missing'):
            audit_startup(self.root)

    def test_missing_and_duplicate_singleton_settings_are_rejected(self):
        for source in ['[URL]\n', '[URL]\nLocalMap=A.unr\nLocalMap=B.unr\n']:
            with self.subTest(source=source), self.assertRaisesRegex(ValueError, 'Missing or ambiguous'):
                one(read_ini(source.encode()), 'URL', 'LocalMap')

    def test_comments_repeated_keys_and_utf16_are_preserved(self):
        ini = read_ini('[Engine.GameEngine]\n;ServerPackages=Ignored\nServerPackages=A\nServerPackages=B\n'.encode('utf-16'))
        self.assertEqual(ini['engine.gameengine']['serverpackages'], ['A', 'B'])

    def test_traversal_and_unhandled_travel_options_are_rejected(self):
        for name in ['../First.unr', 'First.unr?game=Other.Game', 'C:\\First.unr', 'First.unr/Other']:
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, 'Unsupported map'):
                map_path(name)

    def test_case_colliding_paths_are_rejected(self):
        (self.root / 'System' / 'default.ini').write_bytes(b'collision')
        with self.assertRaisesRegex(ValueError, 'Case-colliding'):
            file_index(self.root)

    def test_same_executable_with_changed_or_missing_scripts_is_separate(self):
        reference = {'id': 'hp3-pc-na', 'exeSHA256': 'exe', 'startupEvidence': {
            'scriptPackages': [{'file': 'Core.u', 'sha256': 'original'}]}}
        self.assertEqual(profile_identity('exe', [{'file': 'CORE.u', 'sha256': 'original'}], reference), 'hp3-pc-na')
        changed = profile_identity('exe', [{'file': 'Core.u', 'sha256': 'changed'}], reference)
        missing = profile_identity('exe', [], reference)
        self.assertTrue(changed.startswith('hp3-pc-unrecognized-'))
        self.assertNotEqual(changed, missing)

    def test_profile_cli_separates_unknown_executable_identity(self):
        tools = Path(__file__).resolve().parent
        audit, output = self.root / 'audit', self.root / 'profile'
        subprocess.run([sys.executable, str(tools / 'inspect_packages.py'), str(self.root), str(audit)], check=True, capture_output=True)
        subprocess.run([sys.executable, str(tools / 'build_profile.py'), str(self.root), str(audit), str(output)], check=True, capture_output=True)
        profiles = list(output.glob('hp3-pc-unrecognized-*.json'))
        self.assertEqual(len(profiles), 1)
        profile = json.loads(profiles[0].read_text())
        self.assertFalse(profile['runtimeSupported'])
        self.assertEqual(profile['firstStoryMap'], 'First.unr')
        self.assertFalse((output / 'hp3-pc-na.json').exists())

    def test_profile_cli_rejects_stale_package_inspection(self):
        tools = Path(__file__).resolve().parent
        audit, output = self.root / 'audit', self.root / 'profile'
        subprocess.run([sys.executable, str(tools / 'inspect_packages.py'), str(self.root), str(audit)], check=True, capture_output=True)
        # Still a valid map, but no longer the file inspected by the saved audit.
        package(self.root / 'Maps' / 'First.unr', [('Level', 'ChangedLevel'), ('PlayerStart', 'Start')])
        result = subprocess.run([sys.executable, str(tools / 'build_profile.py'), str(self.root), str(audit), str(output)], capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Stale package audit', result.stderr)
        self.assertEqual(list(output.glob('hp3-*.json')), [])


if __name__ == '__main__':
    unittest.main()

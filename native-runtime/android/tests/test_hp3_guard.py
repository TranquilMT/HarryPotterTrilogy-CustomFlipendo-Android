"""Host checks for the HP3 experimental guard.

HP3 (Prisoner of Azkaban, UE2) must stay disabled by default. These checks are
static: they verify the build configuration and source guard without compiling
the engine or enabling HP3.
"""
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]


class TestHp3Guard(unittest.TestCase):
    def test_hp3_cmake_default_off(self):
        cmake = (ROOT / 'native-runtime' / 'src' / 'hp3' / 'hp3.cmake').read_text(encoding='utf-8')
        self.assertIn('option(HP3_ENABLED', cmake)
        self.assertIn('"Enable experimental HP3/UE2 support (disabled by default)" OFF', cmake)

    def test_hp3_header_default_zero(self):
        header = (ROOT / 'native-runtime' / 'src' / 'hp3' / 'HP3.h').read_text(encoding='utf-8')
        self.assertIn('#ifndef HP3_ENABLED', header)
        self.assertIn('#define HP3_ENABLED 0', header)

    def test_hp3_is_enabled_false_by_default(self):
        header = (ROOT / 'native-runtime' / 'src' / 'hp3' / 'HP3.h').read_text(encoding='utf-8')
        self.assertIn('inline bool IsEnabled() { return false; }', header)

    def test_flipendo_includes_hp3_guarded(self):
        flip = (ROOT / 'native-runtime' / 'src' / 'flipendo.cmake').read_text(encoding='utf-8')
        self.assertIn('include(${CMAKE_CURRENT_LIST_DIR}/hp3/hp3.cmake)', flip)

    def test_no_hp3_enabled_define_in_manifest(self):
        manifest = (ROOT / 'native-runtime' / 'android' / 'AndroidManifest.xml').read_text(encoding='utf-8')
        self.assertNotIn('HP3_ENABLED', manifest)


if __name__ == '__main__':
    unittest.main()
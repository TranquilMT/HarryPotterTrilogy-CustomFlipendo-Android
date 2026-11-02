import unittest
from make_object_constraint_fixtures import unsupported_argument_flags


class ObjectSlotMetadataTests(unittest.TestCase):
    def test_retail_return_flags_are_not_an_out_argument(self):
        self.assertFalse(unsupported_argument_flags(0x580, 2))

    def test_out_argument_remains_excluded(self):
        self.assertTrue(unsupported_argument_flags(0x180, 1))

    def test_lazy_argument_remains_excluded(self):
        self.assertTrue(unsupported_argument_flags(0x280, 1))

    def test_optional_plain_argument_is_supported(self):
        self.assertFalse(unsupported_argument_flags(0x90, 5))


if __name__ == '__main__':
    unittest.main()

import struct
import unittest

from ue2_packages import UE_MAGIC, parse_ue2_header, is_ue2_package


def make_header(file_version=129, licensee=0, name_count=2, name_offset=100,
                export_count=1, export_offset=200, import_count=1, import_offset=300,
                depends_offset=0, generation_count=1, size=400):
    data = bytearray(size)
    struct.pack_into('<I', data, 0, UE_MAGIC)
    struct.pack_into('<I', data, 4, file_version | (licensee << 16))
    struct.pack_into('<I', data, 8, 0)
    struct.pack_into('<ii', data, 12, name_count, name_offset)
    struct.pack_into('<ii', data, 20, export_count, export_offset)
    struct.pack_into('<ii', data, 28, import_count, import_offset)
    struct.pack_into('<i', data, 36, depends_offset)
    struct.pack_into('<i', data, 40, generation_count)
    return bytes(data)


class TestUe2Packages(unittest.TestCase):
    def test_valid_header(self):
        h = parse_ue2_header(make_header())
        self.assertEqual(h['fileVersion'], 129)
        self.assertEqual(h['nameCount'], 2)
        self.assertEqual(h['exportCount'], 1)
        self.assertEqual(h['importCount'], 1)
        self.assertTrue(is_ue2_package(make_header()))

    def test_bad_magic_rejected(self):
        data = bytearray(make_header())
        struct.pack_into('<I', data, 0, 0xDEADBEEF)
        with self.assertRaises(ValueError):
            parse_ue2_header(bytes(data))
        self.assertFalse(is_ue2_package(bytes(data)))

    def test_truncated_rejected(self):
        with self.assertRaises(ValueError):
            parse_ue2_header(b'\x00' * 10)

    def test_ue1_version_rejected(self):
        with self.assertRaises(ValueError):
            parse_ue2_header(make_header(file_version=68))

    def test_negative_count_rejected(self):
        data = make_header(name_count=-1)
        with self.assertRaises(ValueError):
            parse_ue2_header(data)

    def test_offset_out_of_range_rejected(self):
        data = make_header(name_offset=999999)
        with self.assertRaises(ValueError):
            parse_ue2_header(data)

    def test_depends_offset_parsed(self):
        h = parse_ue2_header(make_header(depends_offset=350, generation_count=2))
        self.assertEqual(h['dependsOffset'], 350)
        self.assertEqual(h['generationCount'], 2)

    def test_bad_depends_offset_rejected(self):
        with self.assertRaises(ValueError):
            parse_ue2_header(make_header(depends_offset=999999))

    def test_negative_generation_rejected(self):
        with self.assertRaises(ValueError):
            parse_ue2_header(make_header(generation_count=-1))

    def test_depends_offset_parsed(self):
        h = parse_ue2_header(make_header(depends_offset=350, generation_count=2))
        self.assertEqual(h['dependsOffset'], 350)
        self.assertEqual(h['generationCount'], 2)

    def test_bad_depends_offset_rejected(self):
        with self.assertRaises(ValueError):
            parse_ue2_header(make_header(depends_offset=999999))

    def test_negative_generation_rejected(self):
        with self.assertRaises(ValueError):
            parse_ue2_header(make_header(generation_count=-1))

    def test_is_ue2_false_for_garbage(self):
        self.assertFalse(is_ue2_package(b'not a package'))
        self.assertFalse(is_ue2_package(b''))


if __name__ == '__main__':
    unittest.main()
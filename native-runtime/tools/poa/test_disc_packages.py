import io
import pathlib
import struct
import tempfile
import unittest
import zipfile
from disc_packages import Disc, Extent, full_inventory, record, script_inventory


def entry(name, extent, length, directory=False):
    size = 33 + len(name) + (len(name) % 2 == 0)
    out = bytearray(size)
    out[0] = size
    struct.pack_into('<I', out, 2, extent); struct.pack_into('>I', out, 6, extent)
    struct.pack_into('<I', out, 10, length); struct.pack_into('>I', out, 14, length)
    out[25] = 2 if directory else 0
    out[32] = len(name); out[33:33 + len(name)] = name
    return bytes(out)


def image(raw=False, archive_files=None):
    archive = io.BytesIO()
    with zipfile.ZipFile(archive, 'w') as z:
        for name, data in (archive_files if archive_files is not None else [('system/HGame.u', b'fixture script')]):
            z.writestr(name, data)
    packed = archive.getvalue()
    b = bytearray(32 * 2048)
    root = entry(b'\0', 20, 2048, True)
    for sector, kind in [(16, 1), (17, 255)]:
        start = sector * 2048; b[start:start + 7] = bytes([kind]) + b'CD001\1'
    b[16 * 2048 + 156:16 * 2048 + 156 + len(root)] = root
    records = root + entry(b'\1', 20, 2048, True) + entry(b'System', 21, 2048, True) + entry(b'compressed.zip;1', 23, len(packed))
    b[20 * 2048:20 * 2048 + len(records)] = records
    records = entry(b'\0', 21, 2048, True) + entry(b'\1', 20, 2048, True) + entry(b'kwGame.u;1', 22, 8)
    b[21 * 2048:21 * 2048 + len(records)] = records
    b[22 * 2048:22 * 2048 + 8] = b'fixture!'
    b[23 * 2048:23 * 2048 + len(packed)] = packed
    if raw:
        return b''.join(b'\0' * 16 + b[i:i + 2048] + b'\0' * 288 for i in range(0, len(b), 2048))
    return b


class DiscInventoryTests(unittest.TestCase):
    def full(self, data):
        with tempfile.TemporaryDirectory() as tmp:
            path = pathlib.Path(tmp) / 'disc.bin'
            path.write_bytes(data)
            with Disc(path) as disc:
                return full_inventory(disc)

    def test_complete_inventory_in_both_sector_formats(self):
        for raw in (False, True):
            with self.subTest(raw=raw):
                report = self.full(image(raw))
                self.assertEqual(report['summary']['standaloneFiles'], 2)
                self.assertEqual(report['summary']['archivedFiles'], 1)
                self.assertEqual(report['summary']['unrealPackages'], 2)
                self.assertFalse(report['payloadsReadOrHashesVerified'])

    def test_inventory_classifies_assets_and_preserves_original_spelling(self):
        report = self.full(image(archive_files=[('Music\\Theme.ogg', b'a'), ('maps/First.unr', b'b'), ('system/Help.int', b'c'), ('Movies/Intro.bik', b'd')]))
        rows = {r['path']: r for r in report['files']}
        self.assertEqual(rows['Music/Theme.ogg']['category'], 'music')
        self.assertEqual(rows['Music/Theme.ogg']['originalPath'], 'Music\\Theme.ogg')
        self.assertEqual(report['summary']['categories']['localization'], 1)
        self.assertEqual(report['summary']['categories']['movie'], 1)

    def test_inventory_rejects_unsafe_member_and_directory_names(self):
        for name in ['../Escape.u', 'C:\\Escape.u', '/Absolute.u', '../folder/']:
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, 'Unsafe archive filename'):
                self.full(image(archive_files=[(name, b'bad')]))

    def test_inventory_rejects_case_colliding_archive_members(self):
        with self.assertRaisesRegex(ValueError, 'case-colliding'):
            self.full(image(archive_files=[('system/Core.u', b'a'), ('System/core.u', b'b')]))

    def test_inventory_reports_cross_source_conflict_without_choosing_winner(self):
        report = self.full(image(archive_files=[('system/KWGame.u', b'other')]))
        self.assertEqual(len(report['crossSourcePathConflicts']['system/kwgame.u']), 2)
        self.assertEqual(report['summary']['unrealPackages'], 2)

    def test_inventory_rejects_member_offset_outside_archive(self):
        data = image()
        central = data.index(b'PK\x01\x02')
        struct.pack_into('<I', data, central + 42, 0x7fffffff)
        with self.assertRaisesRegex(ValueError, 'outside payload area'):
            self.full(data)

    def test_metadata_inventory_does_not_claim_corrupt_payload_is_verified(self):
        data = image()
        header = data.index(b'PK\x03\x04')
        name, extra = struct.unpack_from('<HH', data, header + 26)
        data[header + 30 + name + extra] ^= 1
        report = self.full(data)
        self.assertTrue(all(not r['payloadHashVerified'] for r in report['files']))

    def test_hybrid_layout_in_both_sector_formats(self):
        for raw in (False, True):
            with self.subTest(raw=raw), tempfile.TemporaryDirectory() as tmp:
                path = pathlib.Path(tmp) / 'disc.bin'; path.write_bytes(image(raw))
                with Disc(path) as disc:
                    results = script_inventory(disc)
                    self.assertEqual({(x['source'], x['path']) for x in results},
                                     {('disc', 'System/kwGame.u'), ('archive', 'system/HGame.u')})
                    self.assertEqual(disc.at(22 * 2048, 8), b'fixture!')
                    view = Extent(disc, 22 * 2048, 8)
                    view.seek(-2, 2); self.assertEqual(view.read(), b'e!')
                    view.seek(0); self.assertEqual(view.read(3), b'fix')
                    with self.assertRaises(ValueError):
                        disc.at(disc.logical_size, 1)

    def test_bad_format(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = pathlib.Path(tmp) / 'bad.bin'; path.write_bytes(b'bad')
            with self.assertRaisesRegex(ValueError, 'Unsupported'):
                Disc(path)

    def test_mismatched_extent(self):
        b = bytearray(entry(b'a.u', 22, 8)); b[6] ^= 1
        with self.assertRaisesRegex(ValueError, 'Inconsistent'):
            record(b, False)

    def test_unsafe_name(self):
        for name in (b'../evil.u', b'..', b'a\\evil.u'):
            with self.assertRaisesRegex(ValueError, 'Unsafe'):
                record(entry(name, 22, 8), False)

    def test_joliet_name(self):
        result = record(entry('kwGame.u;1'.encode('utf-16-be'), 22, 8), True)
        self.assertEqual(result['name'], 'kwGame.u')

    def test_repeated_directory(self):
        b = image(); record_bytes = entry(b'Again', 20, 2048, True)
        b[21 * 2048:21 * 2048 + len(record_bytes)] = record_bytes
        # Clear rest of this directory so only its repeated root link remains.
        b[21 * 2048 + len(record_bytes):22 * 2048] = b'\0' * (2048 - len(record_bytes))
        with tempfile.TemporaryDirectory() as tmp:
            path = pathlib.Path(tmp) / 'cycle.iso'; path.write_bytes(b)
            with Disc(path) as disc, self.assertRaisesRegex(ValueError, 'Repeated'):
                script_inventory(disc)


if __name__ == '__main__':
    unittest.main()

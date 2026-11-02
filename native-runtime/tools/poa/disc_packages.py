"""Inventory scripts both outside and inside compressed.zip on a local PC disc.

Supports 2048-byte ISO and Mode 1 2352-byte BIN; never runs installer code.
"""
import argparse
import collections
import io
import json
import pathlib
import struct
import zipfile


class Disc(io.RawIOBase):
    def __init__(self, path):
        self.file = open(path, 'rb')
        self.size = self.file.seek(0, 2)
        self.position = 0
        self.stride, self.prefix = 2048, 0
        self.file.seek(16 * 2048 + 1)
        if self.file.read(5) != b'CD001':
            self.stride, self.prefix = 2352, 16
            self.file.seek(16 * 2352 + 17)
            if self.file.read(5) != b'CD001':
                self.file.close()
                raise ValueError('Unsupported disc sector format')
        self.logical_size = self.size // self.stride * 2048

    def close(self):
        self.file.close()
        super().close()

    def seekable(self):
        return True

    def readable(self):
        return True

    def tell(self):
        return self.position

    def seek(self, offset, whence=0):
        base = {0: 0, 1: self.position, 2: self.logical_size}[whence]
        target = base + offset
        if target < 0:
            raise ValueError('Negative disc seek')
        self.position = target
        return target

    def read(self, size=-1):
        remaining = max(0, self.logical_size - self.position)
        size = remaining if size < 0 else min(size, remaining)
        result = bytearray()
        while size:
            sector, inside = divmod(self.position, 2048)
            amount = min(size, 2048 - inside)
            self.file.seek(sector * self.stride + self.prefix + inside)
            block = self.file.read(amount)
            if len(block) != amount:
                raise ValueError('Truncated disc sector')
            result.extend(block)
            self.position += amount
            size -= amount
        return bytes(result)

    def at(self, offset, size):
        if offset < 0 or size < 0 or offset + size > self.logical_size:
            raise ValueError('Extent outside disc')
        self.seek(offset)
        data = self.read(size)
        if len(data) != size:
            raise ValueError('Truncated disc extent')
        return data


def record(data, joliet):
    if len(data) < 34 or data[0] != len(data):
        raise ValueError('Invalid directory record')
    extent, length = struct.unpack_from('<I', data, 2)[0], struct.unpack_from('<I', data, 10)[0]
    if extent != struct.unpack_from('>I', data, 6)[0] or length != struct.unpack_from('>I', data, 14)[0]:
        raise ValueError('Inconsistent ISO extent')
    if data[25] & 128:
        raise ValueError('Multi-extent files are unsupported')
    name = data[33:33 + data[32]]
    if len(name) != data[32]:
        raise ValueError('Truncated directory name')
    if name in (b'\0', b'\1'):
        return None
    name = name.decode('utf-16-be' if joliet else 'ascii')
    name = name.rsplit(';', 1)[0]
    if not name or name in ('.', '..') or any(c in name for c in '/\\\0:'):
        raise ValueError('Unsafe disc filename')
    return dict(name=name, offset=extent * 2048, size=length, directory=bool(data[25] & 2))


def inventory(disc):
    selected = None
    for sector in range(16, 48):
        block = disc.at(sector * 2048, 2048)
        if block[1:6] != b'CD001' or block[6] != 1:
            raise ValueError('Invalid volume descriptor')
        if block[0] == 255:
            break
        joliet = block[0] == 2 and block[88:91] in (b'%/@', b'%/C', b'%/E')
        if block[0] == 1 and selected is None or joliet:
            selected = (block[156:156 + block[156]], joliet)
    else:
        raise ValueError('Unterminated volume descriptors')
    if selected is None:
        raise ValueError('No filesystem descriptor')
    root, joliet = selected
    pending = [('', struct.unpack_from('<I', root, 2)[0] * 2048,
                struct.unpack_from('<I', root, 10)[0], 0)]
    visited, files = set(), []
    while pending:
        path, offset, length, depth = pending.pop()
        if depth > 64 or len(visited) > 10000 or length > 16 * 1024 * 1024:
            raise ValueError('Excessive directory graph')
        if offset in visited:
            raise ValueError('Repeated directory extent')
        visited.add(offset)
        data = disc.at(offset, length)
        pos = 0
        while pos < length:
            size = data[pos]
            if not size:
                pos = (pos // 2048 + 1) * 2048
                continue
            if pos + size > length or pos % 2048 + size > 2048:
                raise ValueError('Directory record crosses boundary')
            item = record(data[pos:pos + size], joliet)
            pos += size
            if item is None:
                continue
            name = (path + '/' if path else '') + item['name']
            if item['directory']:
                pending.append((name, item['offset'], item['size'], depth + 1))
            else:
                if item['offset'] + item['size'] > disc.logical_size:
                    raise ValueError('File outside disc')
                files.append(dict(path=name, offset=item['offset'], size=item['size']))
    return files


class Extent(io.RawIOBase):
    def __init__(self, disc, offset, size):
        self.disc, self.offset, self.size, self.position = disc, offset, size, 0

    def seekable(self):
        return True

    def tell(self):
        return self.position

    def seek(self, offset, whence=0):
        target = {0: 0, 1: self.position, 2: self.size}[whence] + offset
        if target < 0:
            raise ValueError('Negative extent seek')
        self.position = target
        return target

    def read(self, size=-1):
        remaining = max(0, self.size - self.position)
        size = remaining if size < 0 else min(size, remaining)
        data = self.disc.at(self.offset + self.position, size)
        self.position += size
        return data


def script_inventory(disc):
    result = []
    for item in inventory(disc):
        if item['path'].lower().endswith('.u'):
            result.append(dict(item, source='disc'))
        if item['path'].lower().endswith('.zip'):
            with zipfile.ZipFile(Extent(disc, item['offset'], item['size'])) as archive:
                for entry in archive.infolist():
                    if not entry.is_dir() and entry.filename.lower().endswith('.u'):
                        result.append(dict(path=entry.filename, size=entry.file_size,
                                           source='archive', archive=item['path'], crc32=entry.CRC))
    return result


def normalized_path(name):
    name = name.replace('\\', '/')
    parts = name.split('/')
    if not name or any(p in ('', '.', '..') or ':' in p or '\0' in p for p in parts):
        raise ValueError('Unsafe archive filename: ' + name)
    return '/'.join(parts)


def category(name):
    path = pathlib.PurePosixPath(name.casefold())
    suffix = path.suffix
    kinds = {'.u': 'script', '.unr': 'map', '.utx': 'texture', '.usx': 'staticMesh',
             '.ukx': 'animation', '.uax': 'soundPackage', '.umx': 'musicPackage',
             '.bik': 'movie', '.ini': 'configuration', '.zip': 'archive',
             '.dll': 'platformBinary', '.exe': 'platformBinary', '.so': 'platformBinary'}
    if suffix in kinds:
        return kinds[suffix]
    if suffix in {'.ogg', '.wav', '.mp3'}:
        return 'music' if 'music' in path.parts else 'audio'
    if suffix in {'.int', '.rus', '.jap', '.tha', '.cze', '.pol', '.hun', '.kor', '.deu', '.fra', '.ita', '.esp'}:
        return 'localization'
    return 'other'


def full_inventory(disc):
    """List real directory/central-directory metadata without reading asset payloads."""
    rows = []
    for item in inventory(disc):
        path = normalized_path(item['path'])
        rows.append(dict(item, path=path, originalPath=item['path'], source='disc',
                         category=category(path), payloadHashVerified=False))
        if not path.casefold().endswith('.zip'):
            continue
        with zipfile.ZipFile(Extent(disc, item['offset'], item['size'])) as archive:
            seen = set()
            for entry in archive.infolist():
                if entry.is_dir():
                    normalized_path(entry.filename.rstrip('/\\'))
                    continue
                name = normalized_path(entry.filename)
                key = name.casefold()
                if key in seen:
                    raise ValueError('Duplicate or case-colliding archive path: ' + name)
                seen.add(key)
                if entry.header_offset < 0 or entry.header_offset + 30 + entry.compress_size > archive.start_dir:
                    raise ValueError('Archive member outside payload area: ' + name)
                rows.append(dict(path=name, originalPath=entry.filename, size=entry.file_size,
                                 source='archive', archive=path, category=category(name),
                                 compressedSize=entry.compress_size, compression=entry.compress_type,
                                 localHeaderOffset=entry.header_offset, crc32=entry.CRC,
                                 encrypted=bool(entry.flag_bits & 1),
                                 symlink=(entry.external_attr >> 16) & 0o170000 == 0o120000,
                                 payloadHashVerified=False))
    by_path = collections.defaultdict(list)
    for row in rows:
        by_path[row['path'].casefold()].append({'source': row['source'], 'archive': row.get('archive')})
    packages = {'script', 'map', 'texture', 'staticMesh', 'animation', 'soundPackage', 'musicPackage'}
    return {'scope': 'directory and ZIP central-directory metadata only',
            'payloadsReadOrHashesVerified': False, 'files': rows,
            'summary': {'standaloneFiles': sum(r['source'] == 'disc' for r in rows),
                        'archivedFiles': sum(r['source'] == 'archive' for r in rows),
                        'declaredBytesBySource': {kind: sum(r['size'] for r in rows if r['source'] == kind)
                                                  for kind in ('disc', 'archive')},
                        'categories': dict(collections.Counter(r['category'] for r in rows)),
                        'unrealPackages': sum(r['category'] in packages for r in rows)},
            'crossSourcePathConflicts': {name: sources for name, sources in by_path.items() if len(sources) > 1}}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('disc')
    parser.add_argument('output')
    parser.add_argument('--all-files', action='store_true', help='Inventory every standalone/archive file; payload hashes remain unverified')
    args = parser.parse_args()
    with Disc(args.disc) as disc:
        result = full_inventory(disc) if args.all_files else script_inventory(disc)
    pathlib.Path(args.output).write_text(json.dumps(result, indent=2))
    if args.all_files:
        print(json.dumps(result['summary'], indent=2))
    else:
        print('Script packages:', len(result), 'standalone:', sum(x['source'] == 'disc' for x in result))


if __name__ == '__main__':
    main()

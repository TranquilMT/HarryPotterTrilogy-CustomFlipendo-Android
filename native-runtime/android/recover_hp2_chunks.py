"""Extract selected English HP2 installer files from verified-size disc chunks.

Inputs are the disc-byte chunks, a selected InstallShield inventory and its
original cabinet MD5 values. No game code or Windows installer is executed.
"""
import argparse
import hashlib
import json
import pathlib
import struct
import zlib

CHUNK = 524288
CAB_STARTS = {1: 34848 * 2048, 2: 36768 * 2048}
DESTINATIONS = {2: 'Maps', 3: 'Music', 4: 'Textures', 5: 'System', 6: 'CutScenes', 13: 'Sounds'}

class MissingChunk(Exception):
    pass

def read_chunks(directory, offset, size):
    result = bytearray()
    while size:
        base = offset // CHUNK * CHUNK
        path = directory / (str(base) + '.bin')
        if not path.is_file() or path.stat().st_size != CHUNK:
            raise MissingChunk(str(base))
        count = min(size, CHUNK - offset % CHUNK)
        with path.open('rb') as source:
            source.seek(offset % CHUNK)
            block = source.read(count)
        if len(block) != count:
            raise MissingChunk(str(base))
        result.extend(block)
        offset += count
        size -= count
    return bytes(result)

def inflate_file(data):
    output = bytearray()
    position = 0
    while position < len(data):
        if position + 2 > len(data):
            raise ValueError('Truncated InstallShield compression block')
        size = struct.unpack_from('<H', data, position)[0]
        position += 2
        if size == 0 or position + size > len(data):
            raise ValueError('Invalid InstallShield compression block size')
        decoder = zlib.decompressobj(-15)
        block = decoder.decompress(data[position:position + size] + b'\0', 65537)
        if not decoder.eof or len(block) > 65536 or decoder.unconsumed_tail:
            raise ValueError('Invalid InstallShield deflate block')
        output.extend(block)
        position += size
    return bytes(output)

def recover(chunks, rows, destination):
    completed, missing = [], []
    for row in rows:
        relative = pathlib.PurePosixPath(row['path'].lstrip('/'))
        if '..' in relative.parts or row['group'] not in DESTINATIONS:
            raise ValueError('Unsafe recovery inventory path')
        target = destination / DESTINATIONS[row['group']] / relative
        if target.is_file() and target.stat().st_size == row['bytes'] and hashlib.md5(target.read_bytes()).hexdigest() == row['md5']:
            completed.append(target.relative_to(destination).as_posix())
            continue
        try:
            compressed = read_chunks(chunks, CAB_STARTS[row['volume']] + row['offset'], row['compressed'])
        except MissingChunk:
            missing.append(row['index'])
            continue
        if row['flags'] != 4:
            raise ValueError('Unexpected compression/obfuscation flags')
        data = inflate_file(compressed)
        if len(data) != row['bytes'] or hashlib.md5(data).hexdigest() != row['md5']:
            raise ValueError('Original cabinet checksum mismatch: ' + row['path'])
        target.parent.mkdir(parents=True, exist_ok=True)
        temporary = target.with_name(target.name + '.recovering')
        temporary.write_bytes(data)
        temporary.replace(target)
        completed.append(target.relative_to(destination).as_posix())
    try:
        data = read_chunks(chunks, 316641 * 2048, 17559707)
        if data[:4] != bytes.fromhex('c1832a9e') or not 60 <= int.from_bytes(data[4:6], 'little') < 100:
            raise ValueError('Invalid separate HP2 HGame.u disc file')
        target = destination / 'System/HGame.u'
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
        completed.append('System/HGame.u')
    except MissingChunk:
        missing.append('HGame.u')
    return {'complete': not missing, 'verifiedFiles': len(completed), 'missingFiles': len(missing)}

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('chunks', type=pathlib.Path)
    parser.add_argument('inventory', type=pathlib.Path)
    parser.add_argument('output', type=pathlib.Path)
    args = parser.parse_args()
    print(json.dumps(recover(args.chunks, json.loads(args.inventory.read_text()), args.output)))

"""Validate Unreal Engine 2 package headers (Prisoner of Azkaban).

This module is an isolated format inspector. It does not execute engine code and
does not claim any runtime support. It only checks the stable leading fields of
an Unreal package so corrupt or non-UE2 inputs are rejected before any deeper
parsing.
"""
import struct

UE_MAGIC = 0x9E2A83C1
UE2_MIN_VERSION = 100  # PoA observed version is 129


def parse_ue2_header(data):
    """Return a dict of leading UE2 package fields, or raise ValueError.

    The first 40 bytes are common to UE1 and UE2. UE2 packages carry a
    version >= UE2_MIN_VERSION. The depends table offset (UE2+) and the
    generation count are parsed when present; UE1 packages report 0.
    Additional fields exist but are not required for the minimal header check.
    """
    if not isinstance(data, (bytes, bytearray)):
        raise ValueError('package data must be bytes')
    if len(data) < 40:
        raise ValueError('package too short for header')
    magic, version, flags = struct.unpack_from('<III', data, 0)
    if magic != UE_MAGIC:
        raise ValueError('bad package magic: 0x%08X' % magic)
    file_version = version & 0xFFFF
    licensee_version = (version >> 16) & 0xFFFF
    if file_version < UE2_MIN_VERSION:
        raise ValueError('not a UE2 package (file version %d)' % file_version)
    name_count, name_offset = struct.unpack_from('<ii', data, 12)
    export_count, export_offset = struct.unpack_from('<ii', data, 20)
    import_count, import_offset = struct.unpack_from('<ii', data, 28)
    depends_offset = struct.unpack_from('<i', data, 36)[0]
    generation_count = struct.unpack_from('<i', data, 40)[0] if len(data) >= 44 else 0
    for label, value in (('nameCount', name_count), ('exportCount', export_count),
                        ('importCount', import_count), ('generationCount', generation_count)):
        if value < 0:
            raise ValueError('negative %s' % label)
    for label, off, count in (('name', name_offset, name_count),
                              ('export', export_offset, export_count),
                              ('import', import_offset, import_count)):
        if count and (off < 0 or off >= len(data)):
            raise ValueError('%s table offset out of range' % label)
    if depends_offset and (depends_offset < 0 or depends_offset >= len(data)):
        raise ValueError('depends table offset out of range')
    return {
        'magic': magic,
        'fileVersion': file_version,
        'licenseeVersion': licensee_version,
        'flags': flags,
        'nameCount': name_count,
        'nameOffset': name_offset,
        'exportCount': export_count,
        'exportOffset': export_offset,
        'importCount': import_count,
        'importOffset': import_offset,
        'dependsOffset': depends_offset,
        'generationCount': generation_count,
    }


def is_ue2_package(data):
    try:
        parse_ue2_header(data)
        return True
    except (ValueError, struct.error):
        return False
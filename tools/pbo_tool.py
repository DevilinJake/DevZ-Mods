"""Unpack and pack DayZ PBO files (uncompressed, with the product/prefix header).

Usage:
    python pbo_tool.py unpack <file.pbo> <out_dir>
    python pbo_tool.py pack <src_dir> <file.pbo> [--prefix NAME]

unpack writes the files plus a $PBOPREFIX$ file holding the prefix.
pack reads the prefix from --prefix or <src_dir>/$PBOPREFIX$ (that file is not packed).
"""
import hashlib
import os
import struct
import sys
import time

PREFIX_FILE = '$PBOPREFIX$'


def unpack(pbo_path, out_dir):
    data = open(pbo_path, 'rb').read()
    pos = 0
    props = {}
    entries = []

    def read_cstr():
        nonlocal pos
        end = data.index(b'\0', pos)
        s = data[pos:end].decode('utf-8', 'replace')
        pos = end + 1
        return s

    while True:
        name = read_cstr()
        mime, orig, _res, _ts, size = struct.unpack('<5I', data[pos:pos + 20])
        pos += 20
        if name == '' and mime == 0x56657273:  # 'Vers' product entry
            while True:
                key = read_cstr()
                if key == '':
                    break
                props[key] = read_cstr()
            continue
        if name == '':
            break
        if orig and orig != size:
            raise ValueError(f'{name} is compressed; not supported')
        entries.append((name, size))

    for name, size in entries:
        target = os.path.join(out_dir, *name.split('\\'))
        os.makedirs(os.path.dirname(target), exist_ok=True)
        with open(target, 'wb') as f:
            f.write(data[pos:pos + size])
        pos += size

    if 'prefix' in props:
        with open(os.path.join(out_dir, PREFIX_FILE), 'w') as f:
            f.write(props['prefix'])
    print(f'Unpacked {len(entries)} files, prefix "{props.get("prefix", "")}"')


def pack(src_dir, pbo_path, prefix=None):
    if prefix is None:
        prefix = open(os.path.join(src_dir, PREFIX_FILE)).read().strip()

    files = []
    for root, _dirs, names in os.walk(src_dir):
        for n in names:
            full = os.path.join(root, n)
            rel = os.path.relpath(full, src_dir)
            if rel == PREFIX_FILE:
                continue
            files.append((rel.replace('/', '\\'), full))
    files.sort(key=lambda f: f[0].lower())

    header = bytearray()
    header += b'\0' + struct.pack('<5I', 0x56657273, 0, 0, 0, 0)
    for key, value in (('product', 'dayz ugc'), ('prefix', prefix)):
        header += key.encode() + b'\0' + value.encode() + b'\0'
    header += b'\0'

    blobs = []
    now = int(time.time())
    for rel, full in files:
        blob = open(full, 'rb').read()
        blobs.append(blob)
        header += rel.encode() + b'\0' + struct.pack('<5I', 0, len(blob), 0, now, len(blob))
    header += b'\0' + struct.pack('<5I', 0, 0, 0, 0, 0)

    body = bytes(header) + b''.join(blobs)
    with open(pbo_path, 'wb') as f:
        f.write(body + b'\0' + hashlib.sha1(body).digest())
    print(f'Packed {len(files)} files into {pbo_path} (prefix "{prefix}")')


if __name__ == '__main__':
    if len(sys.argv) >= 4 and sys.argv[1] == 'unpack':
        unpack(sys.argv[2], sys.argv[3])
    elif len(sys.argv) >= 4 and sys.argv[1] == 'pack':
        prefix = sys.argv[sys.argv.index('--prefix') + 1] if '--prefix' in sys.argv else None
        pack(sys.argv[2], sys.argv[3], prefix)
    else:
        print(__doc__)
        sys.exit(1)

#!/usr/bin/env python3
"""Compile the exact watch reader on the host with ASan/UBSan; verify all data."""
import pathlib
import struct
import subprocess
import tempfile
import zlib

ROOT = pathlib.Path(__file__).resolve().parents[1]
SRC = ROOT / 'src/c'
blob = (ROOT / 'artifacts/readings.pmr').read_bytes()
count = struct.unpack_from('<I', blob, 8)[0]
raw = b''.join(zlib.decompress(blob[o:o+n]) for o, n in
               (struct.unpack_from('<II', blob, 40+i*8) for i in range(count)))
with tempfile.TemporaryDirectory() as folder:
    folder = pathlib.Path(folder)
    (folder / 'raw').write_bytes(raw)
    flags = ['-std=c99', '-g', '-O1', '-Wall', '-Wextra',
             '-Wno-unused-parameter', '-fsanitize=address,undefined', '-I', str(SRC)]
    sources = [ROOT/'tests/reader_test.c', SRC/'pmr.c']
    sources += [SRC/'vendor/tinf'/f for f in ['tinflate.c', 'tinfzlib.c', 'adler32.c']]
    objects = []
    for i, source in enumerate(sources):
        obj = folder / f'{i}.o'
        # Match watch release settings for the library; keep harness assertions.
        defines = ['-DNDEBUG'] if 'vendor' in source.parts else []
        subprocess.run(['cc', *flags, *defines, '-c', str(source), '-o', str(obj)], check=True)
        objects.append(str(obj))
    subprocess.run(['cc', '-fsanitize=address,undefined', *objects, '-o', str(folder/'test')], check=True)
    subprocess.run([str(folder/'test'), str(ROOT/'artifacts/readings.pmr'), str(folder/'raw')], check=True)

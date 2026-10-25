#!/usr/bin/env python3
"""Exercise the watch's display text preparation under ASan/UBSan."""
import pathlib
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as folder:
    binary = pathlib.Path(folder) / 'display_test'
    subprocess.run([
        'gcc', '-std=c99', '-g', '-O1', '-Wall', '-Wextra', '-Werror',
        '-fsanitize=address,undefined', '-I', str(ROOT / 'src/c'),
        str(ROOT / 'tests/display_test.c'), str(ROOT / 'src/c/display.c'),
        '-o', str(binary),
    ], check=True)
    subprocess.run([str(binary)], check=True)

#!/usr/bin/env python3
"""Validate launcher summaries and real local-date expirations under sanitizers."""
import pathlib
import subprocess
import tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
SRC=ROOT/'src/c'
with tempfile.TemporaryDirectory() as tmp:
    sources=[ROOT/'tests/glance_test.c',SRC/'glance.c',SRC/'calendar.c',SRC/'pmr.c']
    sources += [SRC/'vendor/tinf'/f for f in ('tinflate.c','tinfzlib.c','adler32.c')]
    objects=[]
    for i,source in enumerate(sources):
        obj=str(pathlib.Path(tmp)/f'{i}.o')
        flags=['-DNDEBUG'] if 'vendor' in source.parts else []
        subprocess.run(['cc','-std=c99','-D_POSIX_C_SOURCE=200809L','-g','-O1','-Wall','-Wextra','-Wno-unused-parameter','-fsanitize=address,undefined','-I',str(SRC),*flags,'-c',str(source),'-o',obj],check=True)
        objects.append(obj)
    binary=str(pathlib.Path(tmp)/'glance-test')
    subprocess.run(['cc','-fsanitize=address,undefined',*objects,'-o',binary],check=True)
    subprocess.run([binary,str(ROOT/'resources/calendar.bin')],check=True)

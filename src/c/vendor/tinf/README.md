# tinf

Vendored `tinf.h`, `tinflate.c`, `tinfzlib.c`, and `adler32.c` from
https://github.com/jibsen/tinf/tree/57ffa1f1d5e3dde19011b2127bd26d01689b694b/src
(retrieved 2026-10-08).

Copyright 2003–2019 Joergen Ibsen. The zlib-style license is retained in each
source file. No network access or allocation is required by the decoder.
The PMR reader supplies a complete independent zlib chunk and a 16 KiB output
buffer, which also serves as that chunk's back-reference history. Huffman tables
and decoding state use the stack. The zlib wrapper validates Adler-32.

Local change: `tinflate.c` casts two bounded pointer differences to unsigned
for the Pebble compiler’s sign-comparison warnings. The source marks this change.

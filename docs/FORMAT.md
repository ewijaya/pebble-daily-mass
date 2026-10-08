# PMR2 resource format

All integers are unsigned, little-endian. Text bytes remain normalized UTF-8
from the extractor; there are no typography replacements in the database.
PMR1 is superseded; rebuild the resource and app together.

## Resource header (40 bytes)

| Offset | Value |
| --- | --- |
| 0 | Four bytes `PMR2` |
| 4 | Raw chunk size: 16,384 |
| 8 | Chunk count |
| 12 | Total decompressed payload bytes |
| 16 | Page count |
| 20 | Unique segment count |
| 24 | Page-reference array offset in raw payload |
| 28 | Segment-index offset in raw payload |
| 32 | Text area offset in raw payload |
| 36 | Flags, must be zero |

The header is followed by one 8-byte record per chunk: absolute resource offset
and compressed length. Each chunk is an independent zlib stream with Adler-32.
Release packing uses Zopfli 0.4.3 with 15 iterations to produce smaller zlib
streams; development packing can use Python zlib level 9. Both decode with the
same watch code and memory buffers. The final chunk may be shorter. The writer and reader cap compressed chunks at
16,448 bytes. Chunk directories stay in flash; records are read as needed.

## Decompressed payload

1. Sorted page records, each 12 bytes: numeric source part (519 means
   `text/part0519.html`), first reference ordinal, reference count.
2. A flat uint32 array of segment IDs in page order.
3. Segment records, each 8 bytes: absolute raw-payload offset, UTF-8 byte count.
4. Concatenated text bytes, without terminators. Physical text storage is sorted
   lexicographically by UTF-8 bytes to put similar passages in the same chunks.
   Segment IDs and page order do not change; record offsets locate each text.

All indexes are compressed along with the text. A page lookup uses binary
search; reading a segment requires its index record and relevant text chunks.
Records, text, and UTF-8 sequences may cross chunks. The reader assembles bytes
before passing a complete NUL-terminated string to Pebble.

Reading boundaries are currently reviewed ranges of local page-segment positions
for one sample page. PMR2 does **not** encode semantic roles or date rules for all
pages. Local `sample-audit.json` records the four reviewed reading ranges and
roles. Unreviewed pages remain preserved for future work.

## Memory and errors

`Pmr` owns one 16,384-byte output/history buffer and one 16,448-byte compressed
input buffer, plus metadata. It never loads the whole resource. The reader UI
owns a separate fixed 8,192-byte text buffer. An oversized reading is rejected;
it is never silently truncated. This cap applies to the sample UI, not the
format or the host validation buffers.

`tinf` uses stack-allocated Huffman tables, not an additional 32 KiB history
window. ARM GCC stack-usage output reports 1,632 bytes for `tinf_uncompress`;
its deepest decoding helper path adds up to 88 bytes, plus the wrapper and
reader call chain. Reserve at least 3 KiB for that chain, in addition to SDK
callbacks/other stack use; this is a static estimate, not stack high-water
measurement.

The reader checks header layout, index ranges, resource read lengths, buffer
capacity, zlib checksums and exact output chunk lengths. Errors invalidate the
chunk cache. Reading assembly clears its output on failure. These checks do
not constitute a general hostile-file security audit.

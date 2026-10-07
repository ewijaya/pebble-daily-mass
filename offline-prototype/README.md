# Offline Pocket Missal storage prototype

The extracted English-side reading database fits in an Emery (Pebble Time 2)
resource pack with the installed SDK 4.33.1. The minimal C probe builds successfully
with Pebble Tool 5.0.40 on arm64 macOS. This is an extraction and packaging
experiment, not yet a daily reading app.

## Results

| Measurement | Bytes |
| --- | ---: |
| Database including page references and headings | 932,485 |
| Built Pebble resource pack | 936,593 |
| SDK firmware resource limit | 1,048,576 |
| Remaining resource space | 111,983 |
| SDK App Store allowance | 262,144 |

The build emits an App Store size warning. Personal sideloading still needs an
installation test. Successful compilation does not establish firmware acceptance
or runtime performance on a physical device.

## Extraction

`tools/extract.py` uses only the Python standard library. It inspects all 2,000
EPUB HTML pages, detects 1,006 reading pages by their reading headings, removes
the Latin left column, and retains English paragraphs and surrounding headings,
instructions, references and navigation. Inline text and explicit line breaks
are preserved; whitespace is normalized. Exact repeated segments are stored once.
This deliberately conservative prototype retains material a polished reader can
later omit.

`output/readings.json` contains the source-page-to-segment mappings and texts.
`output/calendar-inventory.json` records all source pages, opening text, detected
month/day labels, ranks, cycle labels, and links. These are source observations,
not validated calendar rules or a date-to-reading schedule. Source links allow
later resolution of saints' entries into proper or common readings.

`output/sample-october-7.txt` is a readable extraction of the EPUB's Our Lady of
the Rosary reading page. It does not assert those readings must replace weekday
readings on every October 7.

Detection is heuristic. A secondary scan found five unselected pages mentioning
reading labels; their details are in `output/review-candidates.json`. Calendar
coverage, cross-reference resolution, reading alternatives, and selection rules
need a separate audit before claiming complete liturgical coverage. Extraction
preserves source errors; it does not correct scripture citations or rubrics.

## Binary format

`output/readings.pmr` begins with little-endian `<4sIII>`: magic `PMR1`, raw chunk
size (16,384), chunk count, total uncompressed payload size. An array of `<II>`
entries gives absolute compressed offsets and lengths. Each chunk is an independent
zlib stream. A watch implementation can read compressed ranges via
`resource_load_byte_range` and decompress one chunk at a time.

The uncompressed payload contains a uint32 JSON byte length, page-reference JSON,
a uint32 segment count, and length-prefixed UTF-8 segments (uint32 lengths).
This format proves storage feasibility. A production reader should replace the
JSON page index with a compact binary index and add direct segment offsets, so it
can retrieve a reading without loading the whole payload. A zlib decoder needs
its own history/workspace in addition to the chunk buffer; total runtime memory
has not been measured.

## Verification

The extractor decompresses every chunk, compares the entire recovered payload,
parses the stored metadata and segments independently, and reconstructs all 1,006
pages for exact comparison with the normalized extracted text. This verifies the
storage round trip, not translation accuracy or completeness against the EPUB.

`storage-probe/build/storage-probe.pbw` is the built PT2 probe. Its source reads
the resource header and displays its size when run. It has no decoder or reader
and has not been run in an emulator or on a watch. The probe's reported RAM use
(1,066 bytes) does not include the future reader/decompressor.

## Reproduce

From the workspace containing the EPUB:

```sh
python3 offline-prototype/tools/extract.py ./*.epub --out offline-prototype/output
cp offline-prototype/output/readings.pmr offline-prototype/storage-probe/resources/readings.pmr
cd offline-prototype/storage-probe
pebble build
```

`output/report.json` includes the source SHA-256 and extraction measurements.
The current report also records this build's resource size; rerunning the
extractor regenerates the extraction-only report.

## Next implementation step

Add a binary lookup index and a bounded-memory watch decoder, then display one
known reading on the emulator and physical PT2. After that, implement the EPUB
calendar mapping and verify dates around Advent, Easter and coinciding feasts.
Country-specific calendars remain outside this first version's scope.

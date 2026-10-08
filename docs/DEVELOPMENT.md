# Daily Roman Missal development guide

The complete extracted library is bundled in a Pebble Time 2 (Emery) resource.
The current menu opens four audited reading sections for Thursday, October 8,
2026 (Ordinary Week 27, Year II), manually checked against USCCB.
Up/Down turn pages with overlap; hold Up/Down to jump to the top/bottom.
Select opens Large/Extra Large bold font options and jump commands. The font
preference persists across app restarts. Source paragraphs have an 8-pixel gap, smaller than a full blank line. Each reading includes its citation and source summary;
Back returns to the menu. The menu shows the fixed date; it does not advance automatically. Date selection
is not implemented.

The app display name is **Daily Roman Missal**. Menu startup defers the database
until a reading is chosen, retaining the validated handle afterward. The physical
text layout is sorted to improve compression; reading order and wording are unchanged.

## Current measurements

Tested with Pebble Tool 5.0.40, SDK 4.33.1, on arm64 macOS, 2026-10-08.

| Measurement | Bytes |
| --- | ---: |
| PMR2 database | 799,048 |
| Built resource pack | 803,312 |
| Firmware resource limit | 1,048,576 |
| Resource headroom | 245,264 |
| SDK App Store allowance | 262,144 |
| Build-reported RAM footprint | 52,006 |
| Build-reported available heap | 79,066 |
| Decoder context (ARM, static buffers included) | 32,884 |
| UI reading text buffer (static, included in footprint) | 8,192 |
| Earlier regular-font preview heap (historical) | 83,320 |

The build's footprint includes code/static data, not runtime UI allocations or
peak stack use. See [FORMAT.md](FORMAT.md) for stack estimates and buffer budgets.
The App Store size warning remains. Emulator and physical PT2 installation work. The physical watch opened the
October 8 First Reading successfully, reporting 83,320 bytes free heap. Broader
physical responsiveness, disconnected operation and battery impact remain untested.

## Extraction and audit

`tools/extract.py` inspects 2,000 EPUB HTML pages, detects 1,006 reading pages,
removes Latin columns, and retains normalized English paragraphs, headings,
instructions, references and navigation. It deduplicates 13,163 text segments.
The new binary indexes and texts occupy 2,406,002 raw bytes in 147 independent
16 KiB zlib chunks.

See [EXTRACTION-AUDIT.md](EXTRACTION-AUDIT.md) for the five excluded candidate
pages and the audited boundaries/roles for the sample. The extractor regenerates
`review-candidates.json`; changed or new candidate text requires review. All
remaining reading boundaries and alternatives remain heuristic/unclassified.

Local outputs also include `readings.json`, `calendar-inventory.json`,
`sample-audit.json`, `october8-audit.json`, `sample-october-7.txt`, `readings.pmr`,
and `report.json`.
The calendar inventory records source observations/links, not calendar rules.
`report.json` contains extraction measurements only; build/runtime measurements
are documented here rather than manually appended to the generated report.

## Retrieval and display

The main menu uses a deep red date strip, red season/citation text, bold black
reading titles and white-on-red selection. The reader options menu uses the
same selection colors. All four sample readings fit on screen. Date and
celebration remain pinned to the sample. A 25×25 transparent black Chi-Rho
launcher icon is declared as MENU_ICON. Its original SVG and PNG are in
`resources/images`; regenerate with `python3 tools/make_icon.py`
(requires Pillow). No new font or online asset dependency is added to the app.

[PMR2](FORMAT.md) replaces PMR1's JSON with binary page, reference and segment
indexes. `src/c/pmr.c` reads resource ranges through a callback,
decompresses only needed chunks, validates lengths/checksums, and assembles a
requested page-segment range. It never loads the full database into RAM.

The decoder uses [tinf](../src/c/vendor/tinf/README.md), with license,
pinned source revision and two compiler portability casts documented locally.
The UI uses a ScrollLayer with explicit page/jump button handlers and
GOTHIC_24_BOLD / GOTHIC_28_BOLD. Page overlap exceeds one line; offsets clamp
at both ends. A footer shows the page count and Select shortcut. Source paragraphs are
measured and drawn separately with 8-pixel gaps; internal single line breaks
are preserved. A deep red title strip and smaller red citations/summaries distinguish
reading roles from the black scripture body. These roles are mapped only for
the four audited October 8 selections; Psalm responses and Gospel incipits
remain black. Header and citation scroll with the reading.
The UI supports at most 64 paragraph boxes, with an explicit
error on overflow. The unsupported
response sign `℟` is displayed as `R` (retaining its following period); database
text remains unchanged. Curly quotation marks, apostrophes and en dashes are
preserved. Other source-wide glyph coverage has not been audited.

The current UI exposes the four audited ranges on `text/part1549.html`. The
earlier Rosary sample audit (`text/part0519.html`) is retained. The 8 KiB display buffer fails explicitly on oversized readings instead of
truncating. Page navigation is implemented for the buffered reading. Streaming arbitrary
long readings beyond the 8 KiB buffer remains future work.

The October 8 EPUB acclamation reads “Open your hearts” where USCCB reads
“Open our hearts”; the preview intentionally retains the EPUB wording for the
user’s accuracy review. The canticle also combines two middle stanzas under one
response, while USCCB separates them. See [the audit](EXTRACTION-AUDIT.md).

## Verification

- Stronger compression: uncompressed bytes match the previous pack exactly; full
  ASan/UBSan tests pass for all segments/pages and error cases. No watch code or
  memory budget changes. Emulator first-reading preparation measured 142 ms
  (previous sample 155 ms), subsequent Gospel 88 ms (previous 89 ms).

- Startup update: all 13,163 segments and 1,006 complete pages passed the full
  ASan/UBSan reader tests after physical text reordering; 132 segments cross
  chunks. Extraction round trip remains exact. PT2 first-menu draw after main()
  improved from 203 ms to 12 ms; firmware loading is outside this measurement.
  Renamed optimized build installed successfully on the PT2. Emulator confirms
  the full launcher name/icon, both font sizes and the complete sample text.

- Main menu and icon: verified all four visible rows, first/last selection,
  entering readings, Back returning to the same row, red options selection,
  and Chi-Rho in both selected/unselected emulator launcher rows.

- Missal-style update: verified red title strips/references/summaries, black
  scripture bodies, both sizes, all four reading headers, scrolling and final
  lines. Emulator heap settles at 78,420 bytes.

- Bold-reader update: built and tested both font sizes, page overlap and bounds,
  top/bottom shortcuts and menu commands, all four readings, and font persistence
  across a full app restart. The Acclamation fits one page even in Extra Large.
  Subsequent spacing revision verifies 8-pixel paragraph gaps and preserved
  internal line breaks in all four readings, with both font sizes and final
  lines checked. Runtime heap settles at 78,948 bytes.
  Other measurements below retain historical evidence for the regular-font build.

- Python pack/unpack reconstructs all 1,006 extracted pages exactly.
- Host C tests compile the same reader and decoder with AddressSanitizer and
  UndefinedBehaviorSanitizer, compare all 13,163 segments and all 1,006 complete
  pages byte-for-byte, and exercise irregular reads over every chunk boundary.
- 133 segments cross chunks; the longest individual segment is 1,783 bytes.
- Host checks cover insufficient capacity, missing pages, invalid indexes/ranges,
  corrupt magic/flags/chunk directory/checksum, and short resource reads.
- Emery build and emulator installation pass. Runtime reading logs and local
  screenshots verify all four sample views, the last lines of First Reading,
  Psalm and Gospel, and reopening the Gospel at the top. The final Psalm/Gospel
  tests used disconnected emulated Bluetooth; heap remained 83,320 bytes across
  Gospel reopening. The acclamation fits on one screen.
- Physical PT2 (firmware v4.38.4) sideload succeeded on 2026-10-08. The First
  Reading opened from the bundle: 652 UTF-8 bytes, 720-pixel text height, ten
  chunk inflations, 83,320 bytes free heap. Artifact identity and observed results
  are in ignored `artifacts/pt2-installation.json`. The user is trying it on hardware.
- No exhaustive physical usability, disconnected-operation, battery, liturgical
  or source-completeness audit.

## Reproduce

From the repository root (Python 3.9 or later, with `uv` for the pinned build-time compressor):

```sh
uv run --with zopfli==0.4.3 python tools/extract.py data/source/*.epub --out artifacts --compression zopfli
python3 tests/test_reader.py
cp artifacts/readings.pmr resources/readings.pmr
pebble build
pebble install --emulator emery --logs build/daily-roman-missal.pbw
```

Zopfli is used only while packing on the computer; the watch keeps its existing
zlib decoder and 16 KiB chunks. For a faster development pack using only Python’s
standard library, run the extractor with `python3` and omit `--compression zopfli`;
that creates a larger compatible database.

Host tests require a C compiler with ASan/UBSan and take a few minutes. They keep
harness assertions enabled and disable the decoder's internal assertions to
match the watch release build; explicit decoder error checks remain enabled.

The PBW is `build/daily-roman-missal.pbw`. No phone companion, network
fetch or runtime JSON parser is used. Font-size persistence is implemented; no date resolver is implemented. Calendar rules, country adjustments, and distribution remain
separate milestones.

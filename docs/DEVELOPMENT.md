# Daily Roman Missal development guide

The complete extracted library is bundled in a Pebble Time 2 (Emery) resource.
The menu selects readings for the local watch date using the General Roman
Calendar, 2020–2037. It shows season/week and celebration names/ranks. Day details
lists full names and optional memorials. Hold Select for date shortcuts, or hold
Up/Down on the main menu to step through days. Each launch opens today's menu. Evening preparation offers tomorrow's day readings.
See [CALENDAR.md](CALENDAR.md) for mapping rules, scope and reproduction.
Up/Down page through text; holds jump to the whole reading's beginning/end.
Select opens Large/Extra Large fonts and jump commands. The font preference
persists, with Extra Large as the default. Source paragraphs retain an 8-pixel gap. Midnight updates the menu;
an open reading stays unchanged until the menu reappears.

The project is branded **Daily Roman Missal**. Installed app metadata uses
**Roman Missal** for displayName, shortName and longName. The phone installer
uses longName as its locker title, which becomes the watch launcher label;
changing only shortName works in the emulator but not through the phone.
The standard SDK configuration now generates both names without a custom override.
The UUID remains unchanged.
Menu startup defers the database
until a reading is chosen, retaining the validated handle afterward. The physical
text layout is sorted to improve compression; reading order and wording are unchanged.

## Current measurements

Version 1.2.0. Tested with Pebble Tool 5.0.40, SDK 4.33.1, on arm64 macOS, 2026-10-08.

| Measurement | Bytes |
| --- | ---: |
| PMR2 database | 799,578 |
| Calendar resource | 131,845 |
| Built resource pack | 937,965 |
| Firmware resource limit | 1,048,576 |
| Resource headroom | 110,611 |
| Legacy SDK warning threshold | 262,144 |
| Build-reported RAM footprint | 63,138 |
| Build-reported available heap, before allocations | 67,934 |
| Decoder context (ARM, static buffers included) | 32,884 |
| UI reading text buffer (static, included in footprint) | 8,192 |
| Earlier regular-font preview heap (historical) | 83,320 |

The calendar decoder allocates about 33 KiB of the heap. The build's footprint
includes code/static data, not this allocation, runtime UI allocations or
peak stack use. See [FORMAT.md](FORMAT.md) for stack estimates and buffer budgets.
The legacy SDK size warning remains; the current store already accepted the
previous complete bundle. Calendar-reader free heap in the 1.2.0 emulator is
about 31,000 bytes in the tested scenarios. Main-to-menu draw remains about 50 ms, excluding firmware loading. Earlier physical-install figures below predate the calendar. Broader
disconnected-operation and battery testing remains outstanding.

## Extraction and audit

`tools/extract.py` inspects 2,000 EPUB HTML pages, detects 1,006 reading pages,
removes Latin columns, and retains normalized English paragraphs, headings,
instructions, references and navigation. Including the owner-provided Gospel
supplement, the bundled library has 13,168 text segments and 1,006 source pages.
It uses independently compressed 16 KiB zlib chunks.

See [EXTRACTION-AUDIT.md](EXTRACTION-AUDIT.md) for the five excluded candidate
pages and the audited boundaries/roles for the sample. The extractor regenerates
`review-candidates.json`; changed or new candidate text requires review. `build_calendar.py` now classifies selected sections by structural EPUB headings
and column roles, with explicit exceptional mappings and regression checks. This
is not a claim of a complete human audit of the source.

Local outputs also include `readings.json`, `calendar-inventory.json`,
`sample-audit.json`, `october8-audit.json`, `sample-october-7.txt`, `readings.pmr`,
and `report.json`.
The calendar inventory records source observations/links, not calendar rules.
`report.json` contains extraction measurements only; build/runtime measurements
are documented here rather than manually appended to the generated report.

## Retrieval and display

The date strip, season/celebration text, highlights and reading title strips use the
selected celebration's liturgical color. White days use gold with black lettering;
other colors use dark fills with white lettering. The reader and date options
menus share this palette. References and introductory rubrics stay red, with
black Scripture on white pages. Selected menu references use contrasting text.
See [CALENDAR.md](CALENDAR.md) for the color policy. The main menu scrolls to accommodate Sunday readings and
Day details. Date and celebration update automatically. A 25×25 transparent black Chi-Rho
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
are preserved. A liturgical-color title strip and smaller red citations/summaries distinguish
reading roles from the black scripture body. Roles follow the source
column and summary classes; Psalm responses and Gospel incipits
remain black. Header and citation scroll with the reading.
The UI supports at most 64 paragraph boxes, with an explicit
error on overflow. The unsupported
response sign `℟` is displayed as `R` (retaining its following period); database
text remains unchanged. Curly quotation marks, apostrophes and en dashes are
preserved. Other source-wide glyph coverage has not been audited.

The 8 KiB display buffer holds one bounded part of a reading. The calendar
compiler splits longer readings between source paragraphs and preserves the full
selected form. Navigation crosses part boundaries; the footer shows the part.
The earlier Rosary sample audit (`text/part0519.html`) remains as historical evidence.

The October 8 EPUB acclamation reads “Open your hearts” where USCCB reads
“Open our hearts”; the preview intentionally retains the EPUB wording for the
user’s accuracy review. The canticle also combines two middle stanzas under one
response, while USCCB separates them. See [the audit](EXTRACTION-AUDIT.md).

## Calendar verification

`python3 tests/test_calendar.py` passes ASan/UBSan checks for all 6,575 dates and
2,521 reading parts, including exact resource references, UI buffer limits, bad
dates and bounds. Regressions cover cycles, transfers, precedence, General Roman
fixed feast dates and alternative boundaries. Emulator evidence is stored in
`artifacts/calendar-validation/`.

## Earlier reader verification (before the calendar)

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
npm ci --prefix tools/calendar --ignore-scripts
node tools/calendar/generate.cjs
python3 tools/build_calendar.py
cp artifacts/calendar.bin resources/calendar.bin
python3 tests/test_calendar.py
python3 tests/test_glance.py
python3 tests/test_planner.py
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
fetch or runtime JSON parser is used. Font-size persistence and General Roman Calendar date selection are implemented.
National calendars and the remaining public-release work are separate milestones.


## Date planning (1.2.0)

`planner.c` contains civil-date conversion, next-Sunday/feast searches and the
16:00 evening shortcut rule. Holy-day scans read just the rank byte in each date
record, avoiding repeated Mass/reading decompression.

The owner requested removal of saved-place resume. Launch always opens today's
menu; each reading starts at its first part and top. Persistence key 1 remains
the font preference (Extra Large by default). Retired bookmark key 2 is deleted
on launch if present. There are no bookmark writes, save timers or resume UI.
Browsed dates remain fixed across midnight during the current session only.

`tests/test_planner.py` checks all civil-date/next-Sunday conversions, holy-day
precedence, evening boundaries, DST and calendar limits under ASan/UBSan.
The physical PT2 install and relaunch were verified without a Resume prompt.
The emulator rejected transfers, so its new UI checks remain pending.
Removal/update evidence is in `artifacts/remove-resume/`; earlier bookmark
screenshots in `artifacts/reader-planning/` are historical.

For phone installation, query the active UUID and close Missal if it is running
before invoking the normal phone installer. A prior phone error occurred even
while the new app was already running; check runtime/screenshots before retrying.

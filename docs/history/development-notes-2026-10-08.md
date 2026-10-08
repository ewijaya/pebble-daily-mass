# Archived development notes

Paths below describe the layout before the project reorganization. Use the root
README and HANDOFF for current paths and status.

# Daily Roman Missal — current handoff

Updated October 8, 2026 (Asia/Tokyo). This section supersedes the historical
October 7 memo preserved below.

## Current state

The first offline reader milestone is implemented and tested in the Emery
emulator. At the user’s subsequent request, the app now opens a preview pinned
to **Thursday, October 8, 2026 — Ordinary Week 27, Year II**. Select First Reading,
Psalm, Acclamation or Gospel;
Up/Down turn pages with overlap; hold Up/Down for top/bottom. Select opens
Large / Extra Large bold font options and jump commands. Font size persists.
Source paragraphs are separated by an 8-pixel gap. Back returns to the menu. It shows the manually selected October 8 readings and does not automatically
advance dates; it must not be described as a finished daily app.

The full extracted library remains bundled. PMR2 replaces PMR1 with compressed
binary indexes; the portable C reader uses range reads and one 16 KiB decompressed
chunk at a time. It validates ranges, lengths and zlib Adler-32 checksums. The
sample UI has an 8 KiB text buffer and returns an explicit error on overflow.

The five secondary extraction candidates have been reviewed: rubrics, a funeral
reading index, or explanatory text, with no omitted scripture reading body found.
The Rosary page’s four ranges/15 English paragraphs and the October 8 page’s
four ranges/nine English paragraphs were audited against XHTML. General reading classification, alternatives, source
completeness and calendar resolution are still unfinished.

## Further compression update (October 8)

The user asked for another launch-speed improvement after confirming the earlier
change felt faster. Release packing now supports Zopfli 0.4.3 / 15 iterations,
selected with `extract.py --compression zopfli`. Default zlib remains available
for quick standard-library development builds. The format, raw payload, all
reading text, C decoder, UI and RAM footprint are unchanged.

- Database: 837,191 → 799,048 bytes; resource pack: 841,455 → 803,312 (4.53% smaller).
- PBW: 815,519 bytes. Build identity and baseline PBW are preserved under ignored
  `output/startup-faster/`.
- Python round trip and comparison against the previous pack prove byte-identical
  uncompressed data. All 13,163 segments / 1,006 complete pages passed ASan/UBSan,
  including 132 cross-chunk segments and corruption/bounds/short-read checks.
- Physical PT2 sideload succeeded; the new build is installed.
- Emery build/install passed. First reading: database validation 81 ms, total
  preparation 142 ms (previous sample 91 / 155 ms). Subsequent Gospel 88 ms
  (previous 89 ms). These are emulator samples, not physical launcher benchmarks.
- Before-update PT2 remote-launch log receipt times varied 1,104–1,696 ms across
  three trials; app main-to-menu remained 12 ms. Phone transport and preceding
  watchface shutdown make these unsuitable for claiming a precise small speedup.
- After-update PT2: three remote-launch receipts at 1,111 / 1,119 / 1,131 ms;
  main-to-menu at 7 / 12 / 8 ms. These overlap the baseline; no reliable
  end-to-end launcher speedup is established. The resource reduction is confirmed.
  Measurement connections are closed; PT2 is left on the app menu, emulator on Gospel.

## Files to read

- `README.md`, `offline-prototype/README.md`: current behavior and reproduction.
- `offline-prototype/EXTRACTION-AUDIT.md`: reviewed pages and sample boundaries.
- `offline-prototype/FORMAT.md`: PMR2 layout, memory budget and error behavior.
- `offline-prototype/tools/extract.py`, `pmr.py`, `audit.py`: extraction, packing,
  round-trip verification, regenerated candidate reports and sample audit.
- `offline-prototype/storage-probe/src/c/pmr.{h,c}`: portable resource reader.
- `offline-prototype/storage-probe/src/c/storage-probe.c`: sample menu and reader.
- `offline-prototype/storage-probe/src/c/vendor/tinf/README.md`: pinned decoder
  source, retained license and two marked compiler portability casts.
- `offline-prototype/tests/test_reader.py`, `reader_test.c`: full-library C tests.

No agent delegation was used. The changes from this session are local and have
not been committed or pushed. `HANDOFF.md` was already untracked when this session
started. Do not accidentally add the ignored EPUB, extracted text, screenshots,
or binaries to Git. The repository remains private.

## Verified results

- Python serialization round trip: all 1,006 pages / 13,163 unique segments.
- C reader: every segment and every complete page matches independent Python
  decompression, under AddressSanitizer and UndefinedBehaviorSanitizer.
- 133 segments cross chunks; longest individual segment is 1,783 bytes.
- Checked capacity errors, missing pages, invalid ranges/IDs, corrupt header,
  chunk-directory/checksum failures and short reads.
- Host tests passed both with decoder assertions enabled initially and with
  the final decoder's release configuration. Reproduction script uses release
  decoder configuration plus active test-harness assertions.
- Final Emery build and emulator installation passed.
- Visually checked all four sample reading screens and final lines of First
  Reading, Psalm and Gospel. Acclamation fits on one screen.
- Reopening the Gospel returns to the top, with the same reported free heap.
- Final Psalm/Gospel tests ran with emulated Bluetooth disconnected. No companion
  or network API is used. This is not a physical-watch airplane-mode test.
- GOTHIC_24 lacks U+211F; the UI displays the response sign as `R.` while preserving
  the source bytes in PMR2. Sample curly quotes, apostrophes and en dashes render.

| Measurement | Bytes |
| --- | ---: |
| Raw PMR2 payload | 2,406,002 |
| PMR2 resource, 147 chunks | 799,048 |
| Built resource pack | 803,312 |
| Remaining firmware resource headroom | 245,264 |
| Build-reported RAM footprint | 52,018 |
| Build-reported available heap | 79,054 |
| Decoder context on ARM | 32,884 |
| Static UI text buffer (included in footprint) | 8,192 |
| Historical regular-font Psalm/Gospel runtime heap | 83,320 |

Decoder stack usage is estimated from ARM GCC `.su` files (see FORMAT.md), not
measured high-water. The SDK still warns about exceeding the 262,144-byte store
allowance. Physical PT2 installation and First Reading execution now pass (see latest
sideload evidence below); broader performance and long-reading UI remain untested.

## Local reproduction and evidence

```sh
uv run --with zopfli==0.4.3 python offline-prototype/tools/extract.py ./*.epub --out offline-prototype/output --compression zopfli
python3 offline-prototype/tests/test_reader.py
cp offline-prototype/output/readings.pmr offline-prototype/storage-probe/resources/readings.pmr
cd offline-prototype/storage-probe
pebble build
pebble install --emulator emery --logs
```

The build is now current: `offline-prototype/storage-probe/build/storage-probe.pbw`.
The packer requires Python 3.9+; the installed smaller build uses the pinned
build-time Zopfli compressor shown above. Omitting that flag uses standard-library
zlib for a larger development pack. Host sanitizer
tests require `cc` and take a few minutes. `report.json` is extraction-only;
build/runtime measurements are in the documentation, not manually injected.

Screenshots remain ignored in `offline-prototype/output/`: `launcher.png` (the
sample menu), `first-reading-top.png`, `first-reading-bottom.png`,
`psalm-top-fixed.png`, `psalm-bottom.png`, `acclamation.png`, `gospel-top.png`,
`gospel-bottom.png`, and `gospel-reopened.png`. Earlier `menu.png`, `installed.png`
and `psalm-top.png` are intermediate captures, not the final reader evidence.

The emulator is intentionally left running for the user’s October 8 accuracy
review. The CLI log follower is stopped; the emulator’s normal companion bridge
remains. Do not close the emulator without considering the user’s active review.

## Latest accuracy-preview update

The user asked to see today’s readings in the emulator. At 2026-10-08 in
Asia/Tokyo, verified against https://bible.usccb.org/bible/readings/100826.cfm:
Galatians 3:1–5; Luke 1:69–70, 71–72, 73–75; Acts 16:14b; Luke 11:5–13.
The app now selects `text/part1549.html` with ranges (first/count): 3/3, 6/5,
11/2, 13/5. `audit_october8` checks those boundaries; the extractor writes
`output/october8-audit.json`. PMR2 bytes and decoder are unchanged; extraction
round trip and final Emery build passed. The previous full-library C tests cover
this page as well.

Two reference differences were explained to the user and preserved for review:
EPUB acclamation has “your hearts” instead of USCCB “our hearts”; the EPUB also
combines the middle canticle stanzas under one refrain. Full First Reading and
Gospel bodies agree apart from punctuation/formatting. No national-calendar
rules were added. The source remains unchanged.

Current screenshots: `output/october8-menu.png`,
`output/october8-first-reading.png`. The emulator is left on the First Reading;
Back opens the dated reading menu. The older Rosary screenshots remain historical
evidence. This pinned preview stays on October 8 even if emulator time advances.

## Physical PT2 sideload — October 8, 10:27 JST

The user explicitly requested a sideload for a hands-on trial. Installed the
current `build/storage-probe.pbw` via `pebble install --cloudpebble --logs` on the
physical PT2 (previously identified as firmware v4.38.4, obelix). The installer
reported success. The physical watch opened First Reading, with log values:
`ok=1 bytes=652 height=720 inflations=10 heap_free=83320`.

The app remains installed and running for the user to assess. Current preview
is still manually pinned to October 8. Hardware acceptance and initial reading
execution are now verified; complete scrolling/offline/battery/usability testing
is still pending user feedback. Ignored `output/pt2-installation.json` records
the exact PBW SHA-256 and evidence. No public publishing or repository push.

## Release intent (latest user decision)

The user explicitly authorized sideloading this preview onto their physical PT2
for a hands-on trial, and wants a Pebble App Store release in the near term.
Public distribution is now a product requirement; the current oversized resource
experiment is not the final distribution plan. Preserve the offline-on-watch
requirement. Before choosing a packaging change, verify the current store limits
(the installed SDK warns at 256 KiB) and evaluate full-library compression,
store-compatible content packaging and calendar metadata together. The user explicitly confirmed “Text distribution approved” on October 8, 2026;
record this as the current distribution decision. Do not publish automatically
from this sideload authorization.

## Readability and navigation update — October 8

User feedback: regular text was faint; requested Large/Extra Large, smaller
paragraph gaps, pages and fast top/bottom navigation. User also explicitly
confirmed text distribution approval.

Implemented in `storage-probe.c`: black-on-white GOTHIC_24_BOLD (Large, default)
and GOTHIC_28_BOLD (Extra Large), persisted under integer key 1. Select while
reading opens the size/jump menu. Tap Up/Down for pages with 32/36 px overlap;
hold 700 ms for top/bottom. Back remains native. Footer shows page count.
The first bold preview collapsed repeated newlines to one newline; the user
found that too cramped. It is superseded by the paragraph-spacing update below.
Size changes retain approximate progress on the new page grid. Opening a
reading starts at the top. This is still the October 8 preview and 8 KiB buffer.

Local SDK 4.33.1 confirmed fonts, persistence, ScrollLayer offsets and click
semantics. Explicit handlers replace the native scroll provider to avoid repeat
versus long-click conflicts. Build passes (existing resource allowance / linker
RWX warnings remain). Emulator verified both sizes, all four readings, last
lines, forward/backward pages, long-click jumps, menu jump commands, saved XL
across complete app restart, and single-page Acclamation without spurious overflow.
Historical runtime/screenshot entries above describe the earlier regular font.
Updated emulator screenshots are ignored `output/reader-v2-*.png`.
Physical PT2 update via CloudPebble also reported “App install succeeded”; exact
artifact identity is in ignored `output/pt2-reader-v2-installation.json`.
Final build: 48,064-byte footprint, 83,008-byte available heap, resource pack
950,833 bytes unchanged. Emulator reader heap settled at 81,216 bytes.

## Paragraph-spacing feedback — October 8

The user found the collapsed paragraphs too cramped. The reader now splits the
original double-newline separators into paragraph drawing boxes, with an 8-pixel
gap. Internal single newlines (citations, psalm lines) remain unchanged. The
source database is unchanged. Layout is measured for the selected bold font;
only paragraphs intersecting the viewport are drawn. At most 64 paragraph boxes
are supported in the existing 8 KiB buffer; overflow shows an explicit error.
Final build succeeds: footprint 50,404 bytes, available heap 80,668 bytes,
resources unchanged at 950,833 bytes. Emulator runtime heap settles at 78,948.
Verified both fonts, paragraph separation, internal line breaks, all four
readings, paging and jumps, full final lines, and one-page XL Acclamation.
Evidence: ignored `output/reader-v3-*.png`. Physical PT2 sideload succeeded;
artifact identity is in `output/pt2-reader-v3-installation.json`. Emulator is left
on First Reading in Large for visual review. No publication, push, or commit.

The user also asked whether memorials/feasts are named. Clarified that the current
October 8 heading is fixed. Calendar selection must display the actual celebration
name and rank (memorial, feast, solemnity) alongside the date; do not claim this
is already automatic or silently choose a country-specific calendar.

## Missal-inspired styling — October 8

The user approved a title strip and requested Daily Roman Missal / rubric-style
coloring. The reader now uses SDK `GColorDarkCandyAppleRed` (#AA0000): white
uppercase title in a full-width red strip, smaller red source citation and
introductory summary, black bold scripture body. Large uses 18 px bold rubric
text and 24 px body; Extra Large uses 24 px rubric and 28 px body. The strip
scrolls away with the reading. Eight-pixel paragraph gaps and navigation remain.

Role styling is explicitly scoped to the audited page: paragraph 0 is the source
heading/citation (the heading moves to the strip); paragraph 1 is an introductory
summary only for First Reading and Gospel. Psalm response and Gospel incipit
stay black. Errors fall back to ordinary black text. Do not generalize these
indices to arbitrary calendar pages without semantic reading metadata.

Built with local SDK 4.33.1 after checking color/font/drawing declarations.
Resource pack remains 950,833 bytes, footprint 50,934 bytes, available heap
80,138 bytes. Emulator heap settles at 78,420 bytes. Verified both sizes,
all four headers/citations, summary versus scripture roles, scroll-away strips,
page navigation and final First Reading/Gospel lines. The XL Acclamation still
fits one screen. Screenshots: ignored `output/reader-v4-*.png`. Physical PT2
sideload succeeded; First Reading opened successfully (652 bytes, 10 inflations,
78,568-byte free heap, 10:54:46 JST). Artifact identity is recorded in ignored
`output/pt2-reader-v4-installation.json`. Emulator remains on First Reading, Large.

## Main menu styling and naming discussion — October 8

The user requested the main menu match the Roman Missal reading style. It now
uses a custom MenuLayer: red date strip with white uppercase date, red season
line, black 24 px bold reading names and red 18 px bold citations, white-on-red
selection. Font/jump options use the same red selection. Explicit Up/Down
handlers select the next row with MenuRowAlignNone and no animation, retaining
all four rows in place. Select opens the chosen reading; native Back is retained.
Bottom padding is disabled and 44-pixel rows keep all four readings visible. Highlight drawing uses the SDK per-cell state
for correct selection animation. The menu remains pinned to October 8.

The user is considering renaming to “Daily Roman Missal” and wants the Roman
Catholic identity emphasized. Suggested Daily Roman Missal as strongest match,
with Roman Missal Daily and Missale Romanum as alternatives. No final name was
selected, so displayName, UUID, repository and folder remain unchanged. Do not
treat naming suggestions as permission for a repository rename or publication.

## Chi-Rho launcher icon — October 8

User explicitly requested Chi-Rho (☧) as launcher icon. Created an original
25×25 black transparent glyph using explicit stroke geometry, without font
assets; `tools/make_icon.py` (Pillow) reproduces PNG and SVG in
`storage-probe/resources/images/chi-rho.{png,svg}`. Package resource MENU_ICON
has menuIcon=true, bitmap type and PNG storage. Name remains Daily Mass pending
a final naming choice. App UUID remains unchanged.

SDK resource schema and icon metadata generator were checked. Existing local
SDK inspection in the user's KasugaBus project documented the actual 25-pixel
launcher limit and monochrome tint; confirmed our symbol by actual Emery
launcher screenshots both selected/unselected. The icon adds 156 bytes; current
resource pack 950,989, firmware headroom 97,587, RAM footprint 51,730, available
heap 79,342. Screenshots are ignored `output/chi-rho-launcher-*.png`.

The main menu update also passed first/last selection, four rows visible,
reading opening, Back preserving selection, and themed reading options. Native
menu bottom padding and scroll alignment were disabled so selecting Gospel does
not hide First Reading. If future menus exceed four rows, restore appropriate
scrolling rather than reusing this fixed preview navigation.
Two physical main-menu update attempts reported install failed (no detailed
reason available in phone status); watch ping subsequently returned Pong.
The combined main-menu/Chi-Rho build is the current artifact. Physical sideload
then succeeded; Gospel opened in XL at 11:05:46 JST (1,131 bytes, 16 inflations,
77,720 free heap). Exact artifact identity is recorded in ignored
`output/pt2-menu-icon-installation.json`. Emulator is left in the launcher with
Daily Mass/Chi-Rho selected. Physical launcher appearance has not been captured.
A final navigation adjustment uses MenuRowAlignNone; screenshots
`menu-v5-gospel-final.png`, `menu-v5-return-final.png`, and `menu-v5-final.png`
verify all four rows stay visible after selection and Back. This final build
also sideloaded successfully; exact identity is in ignored
`output/pt2-menu-icon-final-installation.json`. Current build footprint 51,730
bytes, available heap 79,342, resource pack 950,989.
No rename, repository push, commit, or public publication was performed.

## Rename and launch performance — latest user decision

The user explicitly selected **Daily Roman Missal** for the app name and reported
slow launcher-to-app startup compared with Orationes/Popeye. `pebble.displayName`
is now Daily Roman Missal, keeping UUID, repository and project directory intact.
The Chi-Rho is retained. Earlier naming-pending entries are historical.

Measured the previous initialization on physical PT2: settings 0 ms, obtaining
the database resource handle 176 ms, first menu draw 203 ms after entering main.
`output/startup-investigation/physical-baseline.json` records the receipt timestamps
as well (1,638 ms request-to-log includes phone/Bluetooth/shutdown/firmware latency;
do not equate that with direct launcher interaction time).

Changes:
- Defer resource handle lookup/PMR initialization until the first reading. Cache
  the ready database for later readings. Main menu never touches the big resource.
  All integrity checks remain; the first reading pays the deferred validation cost.
- Lexicographically order physical text bytes in PMR2; index records keep original
  segment IDs, page order and exact source bytes. Smaller zlib streams with the same
  16 KiB buffers: DB 946,725 → 837,191; resource pack 950,989 → 841,455 bytes.
- Retain small timing logs for first menu draw, initial database open and reading
  preparation so future launch reports can be distinguished from firmware loading.

Extraction round trip passed all 1,006 pages. Full ASan/UBSan C tests passed
13,163 segments, all 1,006 complete pages, 132 cross-chunk segments, malformed data,
capacity and short-read cases. Largest segment still 1,783 bytes. Emulator menu
first draw 17 ms; first reading with validation prepared in 155 ms, subsequent
Gospel in 89 ms. Both font sizes and complete sample text visually checked.

Current build: resource pack 841,455, RAM footprint 52,018, available heap 79,054.
No increase to decompression workspace. The SDK App Store allowance warning remains.

Primary firmware source inspected (downloaded under ignored startup-investigation):
https://github.com/coredevices/PebbleOS/blob/main/fw/process_management/app_manager.c
https://github.com/coredevices/PebbleOS/blob/main/fw/resource/resource_storage.c
https://github.com/coredevices/PebbleOS/blob/main/fw/applib/applib_resource.c
It checks the full resource pack before app execution, and resource_get_handle
validates the selected resource. App timing excludes the pre-main firmware stage.

Final optimized physical PT2 sideload succeeded. A controlled stop/start of our
app measured **12 ms** from main() entry to first menu draw versus **203 ms**
before; `physical-optimized.json` records it. Request-to-log receipt was 1,084 ms
versus 1,638 ms, but these include variable transport and previous-app shutdown,
so do not claim those are precise launcher timings. Firmware resource validation
still runs; opening the first reading now performs the deferred resource check.
Exact PBW identity: `output/startup-investigation/installation.json` (853,663 bytes).
Full name and Chi-Rho verified in emulator launcher (`renamed-launcher.png`).
No publication, repository rename, commit or push. Physical app stays installed.

The old emulator repeatedly NACKed binary installs even after restarting. Its
flash was preserved as `output/startup-investigation/qemu_spi_flash-before-reset.bin`
and the SDK created a fresh emulator flash; installations then succeeded. Physical
watch data was not reset. Emulator now contains the renamed app for review.

## Next useful work

1. Collect the user’s hands-on feedback from the installed PT2 preview. Verify
   complete scrolling, opening latency and reopening without a phone; initial
   sideload/resource acceptance and First Reading execution already passed.
2. Generalize reading boundaries and optional/short/long alternatives across the
   source; preserve citations, rubrics and navigation distinctions. The current
   UI ranges are deliberately limited to one audited page.
3. Support long-reading UI without truncation or excessive layout work. Keep
   random access/bounded buffers and consider paging for Passion readings.
4. Build the EPUB-based calendar resolver, including linked propers/commons,
   cycles, Easter/season/week calculation, precedence and transfers, then add
   Today and previous/next day. Validate against authoritative rules. Country
   adjustments remain deferred; do not silently add Japan/Indonesia/etc.

Follow the local `pebble-sdk-inspector` skill before unfamiliar Pebble work.
Toolchain paths, source identity and user decisions in the historical memo below
remain valid unless superseded above. Do not follow its obsolete PMR1 format or
its statement that no decoder/reader/emulator test exists.

<details>
<summary>Historical handoff — October 7, 2026 (superseded where noted above)</summary>

# Daily Mass — AI agent handoff

Prepared October 7, 2026 (Asia/Tokyo).

## Start here

Continue development of **Daily Mass**, an offline Catholic daily Mass readings
app for **Pebble Time 2**. The completed work is an EPUB extraction and storage
prototype. There is no working reading app or date-selection engine yet.

Read this memo, the root README, `offline-prototype/README.md`, and the extractor
before editing. The next useful milestone is displaying one complete extracted
reading on the watch using bounded-memory decompression and indexed retrieval.

## User decisions and intent

- App name: **Daily Mass**. Earlier name suggestions were not selected.
- Local folder and GitHub repository must both be named `pebble-daily-mass`.
- The user wants readings stored on the watch, usable without a connected phone
  or internet. Do not revert to phone-assisted daily downloads by default.
- Start with English readings from the supplied EPUB and its celebrations.
- Use the EPUB's calendar coverage initially. Country-specific adjustments are
  deferred. The user lives in Japan and often visits Indonesia, Singapore, and
  Malaysia; this does not authorize adding those calendars now.
- Intended flow discussed: open today's date/celebration, select First Reading,
  Psalm, Second Reading where applicable, or Gospel, then scroll. Previous/next
  day navigation was proposed. Detailed UI choices remain open.
- The immediate implementation authorized before naming work was extraction and
  the storage prototype. That milestone is complete. This handoff supports the
  user's next session; no background development is running.

## Workspace and repository

- Current path: `/Users/e_wijaya_ap/Desktop/pebble-daily-mass`
- Previous path: `/Users/e_wijaya_ap/Desktop/TEMP` — renamed; do not use it.
- GitHub: https://github.com/ewijaya/pebble-daily-mass
- Visibility: **private**. Keep it private unless the user requests otherwise.
- Branch: `main`; remote: `origin` (HTTPS).
- Latest development commit before this memo: `355554e` (probe documentation).
- Initial implementation commit: `903b5a8`.
- Code was committed and pushed. No uncommitted changes existed before this memo.
- GitHub CLI is authenticated as `ewijaya`.
- No AGENTS.md was found during the initial workspace check; check again in the
  next session in case instructions have changed.

The original EPUB, extracted reading data, and build outputs are intentionally
ignored by Git. They remain in this local folder. A fresh clone alone cannot
reproduce the build without the EPUB. Do not add these files to Git or publish
text artifacts as part of routine commits.

## Local source and outputs

Source EPUB in repository root:

`Pocket Missal [v.04.X.2026] - Pocket Missal Project_3fb5ebc6-b703-4dca-a513-52bd4a3ea44e.epub`

SHA-256:
`a88c1e4917d3a6c81cd6f2eda709aad7ef4486c80d45e027c26f2aa6f6ae78c5`

Important files:

| Path | Purpose |
| --- | --- |
| `offline-prototype/tools/extract.py` | Standard-library Python extractor, packer, round-trip checks |
| `offline-prototype/output/readings.json` | Extracted segments and source-page mappings; ignored |
| `offline-prototype/output/readings.pmr` | Chunk-compressed database; ignored |
| `offline-prototype/output/calendar-inventory.json` | Source dates, rank/cycle labels, opening text and links; ignored |
| `offline-prototype/output/review-candidates.json` | Five unselected pages mentioning reading labels; ignored |
| `offline-prototype/output/report.json` | Extraction and manually appended build measurements; ignored |
| `offline-prototype/output/sample-october-7.txt` | English-side Our Lady of the Rosary sample; ignored |
| `offline-prototype/storage-probe/src/c/storage-probe.c` | Minimal resource header/size display |
| `offline-prototype/storage-probe/package.json` | Emery-only app, raw resource declaration, display name Daily Mass |
| `offline-prototype/storage-probe/resources/readings.pmr` | Local copy used by build; ignored |
| `offline-prototype/storage-probe/build/storage-probe.pbw` | Successfully built probe; ignored |

The PBW predates the later display-name/source-label change to Daily Mass. Rebuild
before installation so the binary reflects the current source. The technical
probe directory/package name remains `storage-probe`; the app display name is
Daily Mass. Renaming the probe is not necessary for the next milestone.

## Confirmed findings

The EPUB contains English and Latin side-by-side text, seasonal readings,
Sunday cycles A/B/C, weekday cycles I/II, dated saints' entries, memorials,
feasts, solemnities and common readings. Example: October 7, Our Lady of the
Rosary, has its own reading page at `text/part0519.html`.

It is not a pre-resolved civil-date schedule. Actual selection still requires
Easter computation, season/week calculation, cycle selection, precedence,
transfers and correct handling of memorials and optional readings. The existence
of a saint's reading page does not mean it always replaces weekday readings.
No exhaustive liturgical completeness or source-accuracy audit has been done.

Extraction measurements:

| Item | Result |
| --- | ---: |
| HTML pages inspected | 2,000 |
| Detected reading pages | 1,006 |
| Unique text segments | 13,163 |
| Uncompressed payload | 2,410,171 bytes |
| Compressed PMR database | 932,485 bytes |
| Independently compressed chunks | 148 |
| Maximum raw chunk | 16,384 bytes |
| Built Pebble resource pack | 936,593 bytes |
| SDK Emery firmware resource limit | 1,048,576 bytes |
| Remaining resource headroom | 111,983 bytes |
| SDK App Store resource allowance | 262,144 bytes |

A PT2 build succeeded with the entire database bundled. The SDK warns that the
resource size exceeds the App Store allowance. This establishes packaging
feasibility for a sideloading experiment, not actual installation, runtime, or
App Store acceptance. Do not promise a public store release under this design.

## Extraction and format details

The extractor detects reading pages using paragraph headings with class
`centertextred` beginning with First reading, Second reading, Gospel, or
Responsorial Psalm. It removes `div.left` Latin columns and traverses paragraph,
heading, list and standalone span content. It keeps English-side surrounding
instructions/navigation as well as readings. Whitespace is normalized and
explicit line breaks retained. Exact repeated segments are deduplicated.

This is conservative, heuristic extraction. It does not yet assign every segment
a semantic role or split all reading alternatives correctly. Validate boundaries,
links, missing text and selection metadata before claiming complete coverage.

PMR1 format, all integers little-endian:

1. `<4sIII>` header: `PMR1`, raw chunk size, chunk count, total raw payload length.
2. One `<II>` record per chunk: absolute compressed byte offset and length.
3. Independent zlib-compressed chunks.
4. Decompressed payload: uint32 metadata length; UTF-8 JSON page mappings;
   uint32 segment count; repeated uint32 byte length + UTF-8 segment bytes.

The production watch format should replace JSON with a compact binary lookup
index and add direct segment offsets. Do not load the 2.4 MB payload into RAM.
Resource range reads are supported. A decoder also needs history/workspace beyond
its output buffer; budget this explicitly within the 128 KiB app memory limit.

## Verification completed and limits

- All chunks were decompressed and compared to the original serialized payload.
- Metadata and segments were decoded, and all 1,006 pages reconstructed and
  compared with their normalized extracted text.
- Native C probe compiled and bundled successfully for `emery`.
- Probe code can read the PMR header and display its resource size when run.
- No emulator or physical watch execution occurred.
- No watch-side decompressor, date resolver, reading interface or runtime memory
  test exists yet.
- The build's 1,066-byte RAM footprint applies only to the tiny probe. It says
  nothing about final reader/decompressor memory use.
- Secondary review-candidate scanning and build metrics were added manually.
  Running the extractor overwrites report.json with extraction-only measurements
  and does not regenerate review-candidates.json. Automate these if retained.

## Toolchain and reproduction

Host: arm64 macOS; shell: zsh; Python 3 available.
Pebble CLI: `/Users/e_wijaya_ap/.local/bin/pebble`
Pebble Tool: 5.0.40; active SDK: 4.33.1.
PT2 platform: **emery**, 200 × 228 color display. Gabbro is Pebble Round 2.

SDK root:
`/Users/e_wijaya_ap/Library/Application Support/Pebble SDK/SDKs/4.33.1`

Platform resource/memory limits:
`sdk-core/pebble/common/tools/pebble_sdk_platform.py`

Header:
`sdk-core/pebble/emery/include/pebble.h`

Use the applicable skill before further Pebble API/toolchain work:
`/Users/e_wijaya_ap/.agents/skills/pebble-sdk-inspector/SKILL.md`.
Inspect command help before composing unfamiliar emulator/install commands.

From the repository root:

```sh
python3 offline-prototype/tools/extract.py ./*.epub --out offline-prototype/output
cp offline-prototype/output/readings.pmr offline-prototype/storage-probe/resources/readings.pmr
cd offline-prototype/storage-probe
pebble build
```

The existing probe has no phone companion or network dependency.

## Recommended remaining work, in order

1. Audit extraction boundaries and the five review candidates. Preserve source
   references and distinguish heading, citation, English body, rubric, navigation
   and optional reading alternatives. Keep evidence of what is excluded.
2. Design a compact random-access binary page/reading/segment index. Include its
   size, calendar metadata and resource wrapper in the full size budget.
3. Implement a compatible, bounded-memory decompressor on PT2. Check any added
   library's license and record attribution. Read only required resource chunks.
4. Build a vertical slice: choose one known reading, load/decompress it, display
   its citation and full text with Up/Down scrolling. Test long readings, Unicode,
   line wrapping, chunk boundaries and memory use in emulator and on hardware.
5. Build the EPUB-based calendar resolver. Resolve linked propers and commons;
   distinguish mandatory and optional selections. Check representative weekdays,
   all Sunday cycles, Advent/year transitions, Easter/Holy Week, fixed feasts,
   leap years and coinciding celebrations against authoritative rules.
6. Add Today, reading selection and previous/next-day navigation. Use local date
   and refresh it appropriately. Keep country-specific calendars deferred.
7. Verify fully offline operation, persistence/reopening behavior and real-device
   responsiveness. Only then discuss broader distribution.

Do not describe steps 1–7 as completed or infer correctness from the packer's
lossless checks alone.

## App landscape and prior discussion

The user asked whether similar apps make this redundant. Store research found:

- Catholic Daily: daily readings via Universalis, today plus six days; old listing.
  https://apps.repebble.com/catholic-daily_56da609222f7b305b7000051
- Catholic Rosary: Rosary plus Mass readings fetched from Evangelizo.
  https://apps.repebble.com/catholic-rosary_7f62b260bb574a4d927c382e
- Roman Calendar: liturgical day display.
  https://apps.repebble.com/roman-calendar_5704811c0239f13390000018
- Orémus: prayer guide; readings described as a future plan.
  https://apps.repebble.com/or-mus_56db86f7eeaab1cad5000045

These descriptions were not functionally tested. We suggested trying the two
reading apps before substantial additional work; no user test results arrived.
The proposed differentiator is a full bundled reading library with offline date
selection and a PT2-focused reading experience. Do not claim it is the first or
only such app.

The ten earlier suggested names had no exact current store-index matches when
checked. **Daily Mass was chosen afterward and has not itself been checked for
an exact title collision.** Do not confuse those findings.

The user's screenshot showed their existing `pebble-orationes`, `pebble-pietas`,
`kasugabus-pebble` and `popeye-gw` repositories. Their code was not inspected or
reused here. Reuse may be worth investigating if the user wants consistency,
but do not assume these repos are present locally or modify them casually.

## Final handoff state

No agents, servers, emulator sessions or pending tasks are running. The user
requested this memo to move to another AI-agent session. Preserve the local EPUB
and generated artifacts when opening the renamed project.

</details>

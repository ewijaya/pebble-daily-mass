# Daily Roman Missal — current handoff

Updated October 8, 2026 (Asia/Tokyo). Read this file and README before continuing.

## Current state

The app is an offline Roman Catholic readings app for Pebble Time 2 (Emery),
with the finalized name **Daily Roman Missal** and an original Chi-Rho launcher
icon. The user approved text distribution, the missal-style UI, and the current
launch speed. The next requested work was project naming and organization.

The reader still displays a manually pinned **Thursday, October 8, 2026 —
Ordinary Week 27, Year II** preview. It does not automatically select today's
readings or resolve feasts/memorials. Do not describe it as ready for publication.
See `docs/RELEASE.md` for remaining work.

## Project organization

The repository root is now the Pebble project. Build with `pebble build` here.

- `package.json`: package `daily-roman-missal`, display name Daily Roman Missal,
  UUID `bfbd18b3-4a25-44b1-99f9-2de4fbe0f076`, version 1.0.0, Emery only.
- `src/c/main.c`: menu, reader, font preferences and navigation.
- `src/c/pmr.{c,h}`: portable bounded-memory resource reader.
- `src/c/vendor/tinf/`: pinned decoder and retained license; see its README.
- `resources/images/`: original Chi-Rho PNG/SVG.
- `tools/`: extraction, audit, packer and icon generator.
- `tests/`: exact C decoder/reader validation with ASan/UBSan.
- `docs/DEVELOPMENT.md`: detailed behavior, measurements and reproduction.
- `docs/FORMAT.md`, `docs/EXTRACTION-AUDIT.md`: data format and audited samples.
- `data/source/`: original local EPUB, ignored.
- `artifacts/`: generated data, screenshots, measurement records, emulator backup;
  all ignored. Former output files are preserved here.
- `artifacts/previous-project-build/`: preserved old build/cache files, not used
  by the new root build.
- `build/daily-roman-missal.pbw`: current build artifact, ignored.
- `docs/history/`: archived notes with historical names/paths; these are not
  current instructions. The full previous handoff is retained there.

The checkout directory and private GitHub remote remain unchanged. No commit,
push, repository visibility change or store publication was requested or done.
Many source files were already untracked before this reorganization; preserve
all work. Keep EPUBs, extracted texts and binaries excluded from Git.

## Behavior and constraints

The full extracted library contains 1,006 pages and 13,163 unique text segments.
PMR2 stores binary indexes and lexically ordered text in 147 independent 16 KiB
zlib chunks. Zopfli 0.4.3 / 15 iterations produces the installed smaller pack;
standard-library zlib remains the faster development packing option. Compression
changes preserve the uncompressed payload byte-for-byte.

The main menu uses a deep red date strip, red season/citations, bold black titles,
and white-on-red selection. Four rows remain visible without native scroll
margin movement. The reader uses red title strips and smaller red references /
introductory summaries, with black bold scripture. Large is GOTHIC_24_BOLD;
Extra Large is GOTHIC_28_BOLD. Paragraph gaps are 8 pixels; source single line
breaks are preserved. Up/Down turn pages with overlap; hold 700 ms for top/bottom.
Select opens font/jump options, Back returns to the menu. Font preference persists
under key 1 (0 Large, 1 Extra Large).

The UI has an 8 KiB text buffer and 64 paragraph boxes with explicit errors on
overflow. The complete library is stored, but only four audited ranges from
`text/part1549.html` are currently selectable: starts {3,6,11,13}, counts {3,5,2,5}.
The first reading is Galatians 3:1–5; Psalm Luke 1:69–75; Acclamation Acts 16:14b;
Gospel Luke 11:5–13. Source rubrics/role mapping are audited only for this page.
The EPUB acclamation says “your hearts” where USCCB says “our hearts”; intentionally
preserved for accuracy review. The response glyph is displayed as R. because
GOTHIC_24 lacks U+211F. Database text is unchanged.

No phone companion or runtime network access is used. Resource lookup/validation
is deferred until opening the first reading, then the handle is reused.

## Measurements and validation before organization

- PMR2 database: 799,048 bytes; resource pack: 803,312 bytes.
- RAM footprint: 52,018 bytes; build-reported free heap: 79,054 bytes.
- Decoder context: 32,884 bytes, including fixed input/output buffers.
- Firmware resource limit: 1,048,576 bytes; SDK store allowance: 262,144 bytes.
  The store warning remains unresolved despite successful PT2 sideloads.
- All segments and complete pages passed host ASan/UBSan tests, including 132
  cross-chunk segments and bounds, capacity, checksum/corruption and short reads.
- Latest compression build installed on PT2 successfully. The user is satisfied
  with launch speed. App main-to-menu measured 7–12 ms; original baseline was
  203 ms. This excludes firmware loading before app entry.
- Latest remote launch receipts were 1,111 / 1,119 / 1,131 ms and overlap the
  previous build's range. Do not claim a definite further total-launch speedup
  from the last 4.53% pack reduction. Phone transport and previous app shutdown
  add variability. Evidence is under `artifacts/startup-faster/`.
- Emulator first-reading preparation: 142 ms including 81 ms initial validation;
  subsequent Gospel: 88 ms. No watch decoder/UI changes in the compression update.

## Organization validation

The root extraction command (including its default artifacts directory) passed
all source round trips, and its PMR2 bytes exactly match the previous resource.
All 13,163 segments / 1,006 pages and malformed-input checks passed the relocated
ASan/UBSan test script. A fresh root build produced
`build/daily-roman-missal.pbw`; its name is set explicitly in wscript because
SDK 4.33.1 otherwise derives the bundle filename from the checkout directory.
Use the explicit PBW path when installing.

The new PBW preserves the UUID, both display names and the byte-identical resource
pack. Emulator installation and First Reading display passed. The source file
rename to main.c slightly reduced log filename storage: RAM footprint 52,006,
free heap 79,066. Resource size remains 803,312 bytes. Evidence:
`artifacts/reorganization-build.json` and `artifacts/reorganized-first-reading.png`.
No further physical sideload was needed for this organizational change; PT2 keeps
the tested compression build. The emulator remains open on First Reading.

## Toolchain and device workflow

Use the local `pebble-sdk-inspector` skill before unfamiliar Pebble commands/APIs.
Pebble Tool 5.0.40, SDK 4.33.1, macOS arm64. SDK is under
`~/Library/Application Support/Pebble SDK/SDKs/4.33.1`.
Physical PT2 firmware v4.38.4, hardware obelix18, connects via CloudPebble.
Sideloading is already authorized. Keep the UUID and app display name stable.

See the root README for extraction/test/build commands. Install from the root:

```sh
pebble install --emulator emery build/daily-roman-missal.pbw
pebble install --cloudpebble build/daily-roman-missal.pbw
```

Do not print broad process command lines: the emulator bridge can include OAuth
credentials. Inspect CLI help before using unfamiliar options. Do not use
`pebble wipe`; it affects more than this app. The old emulator flash backup is
`artifacts/startup-investigation/qemu_spi_flash-before-reset.bin`; the physical
watch was never reset. Preserve that backup.

The complete prior investigation, UI history, source audit and firmware evidence
are in `docs/history/development-notes-2026-10-08.md`. Historical path references
there intentionally describe the old layout; use current paths above.

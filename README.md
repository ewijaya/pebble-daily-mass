# Daily Roman Missal

An offline Catholic Mass readings app for Pebble Time 2 (Emery).
English readings are extracted from a locally supplied Pocket Missal EPUB and
bundled on the watch, with an owner-supplied John 8:12–20 supplement.

## Status

The app automatically selects readings using the watch's local date and the
**General Roman Calendar (2020–2037)**. The menu shows the season/week and
celebration name and rank; Day details includes full names and optional memorials.
Hold Select for Today, Previous/Next day, Next Sunday or Next Holy Day (the next
feast or solemnity). Hold Up/Down on the main menu to step backward/forward
through dates; short presses select readings. The main-menu selector wraps in
both directions, and touch navigation is enabled.

Each launch opens today's menu; reading positions and browsed dates are not saved.
From 4 PM on Saturdays or the eve of a solemnity, an
**Evening Mass** option offers tomorrow's day readings with a notice that separate
vigil texts are not included.

Tap Up/Down for the SDK's short scrolling step
(32 pixels on the tested firmware). Hold either button for 0.5 seconds to page
by 80% of the viewport, repeating every 325 ms until release. Touch swipes use
the watch's native navigation when touch is enabled in system settings.
At the end, a fresh Down tap opens the next reading directly. Up at the start
returns to the previous reading's last page. Held paging stops at each reading's
boundary; release and tap to continue. Long readings continue across all their
parts. The Gospel ends on its final text; Back returns to the menu. There are no
intermediate or completion cards.

Settings is below Day details and also opens with Select in the reader. Large
and Extra Large apply to reading text, menu labels, citations and prompts; Extra
Large is the default. Text size and the Light/Church (dark) theme persist on the
watch. Compact headers leave more
room for larger rows, with full celebration names in Day details.

A quiet 4-pixel crimson ribbon at the reader's right edge grows with progress
through the whole reading, including every part. It drops in over 200 ms when
opened and updates immediately on page turns. It ends just above the bottom edge and
adds no saved-place or resume behavior. Liturgical-color strips and red rubrics
remain, with 8-pixel paragraph gaps. A Chi-Rho identifies **Roman Missal** in the
launcher.

Version 1.3.1 adds a quiet version label at the bottom of Settings,
read automatically from the SDK's build metadata, and a thin crimson rule
between the readings and Day details/Settings. The reader's black page-counter
footer is removed, giving the text the full screen height; the ribbon still
shows progress and Select still opens Settings.

Opening and exiting the app saves seven days of offline App Glance subtitles.
The launcher shows the day's celebration or season/week, advancing at local
midnight. Compact labels include “OT · Wk 27” and “Mem. St. Teresa of Jesus”.
After those slices expire, it says “Open for today's readings”; opening
and exiting replenishes them. Reopen after changing the watch's timezone to
refresh midnight boundaries. Long subtitles use the launcher's native ellipsis.

Version 1.4.0 is publicly available with direct reading navigation and the
wrap, touch and tap/hold controls above.
Calendar selection uses one
reading set per date; national calendars, separate vigil readings and optional-memorial
choices are outside this edition. See [calendar scope and validation](docs/CALENDAR.md).
Download from the [Pebble App Store](https://apps.repebble.com/9e466762014d4b1ab248219f)
or [GitHub Releases](https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.4.0).
The public PBW downloads have matching checksums. The legacy SDK size warning
did not block the store upload.
See [release status](docs/RELEASE.md) for verification and remaining checks.

## Project layout

```text
src/c/             Watch app, resource reader and bundled decoder
resources/         Chi-Rho icon and generated reading database
tools/             EPUB extraction, packing, auditing and icon generation
tests/             Portable C reader validation
docs/              Development guide, format, audits and release checklist
data/source/       Local source EPUB and Gospel supplement (ignored by Git)
release/store/     Listing, icons and native store screenshots
artifacts/         Generated data, screenshots and measurement records (ignored)
build/             Pebble build output (ignored)
```

The Pebble project lives at the repository root. Its package name is
`daily-roman-missal`; the project is branded **Daily Roman Missal**. Installed
app metadata uses **Roman Missal** for both short and full names so the PT2
launcher and phone installer consistently use the compact label. The existing
UUID is preserved so updates replace the same installed app.

## Build and verify

Place the source EPUB and owner-supplied `john-8-12-20.json` in `data/source/`,
then run from the repository root. The supplement contains five paragraphs,
citation and provenance; it fills a documented omission in the EPUB.

```sh
uv run --with zopfli==0.4.3 python tools/extract.py data/source/*.epub --compression zopfli
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
pebble install --emulator emery build/daily-roman-missal.pbw
```

Requires Python 3.9+, Node.js/npm, `uv`, a C compiler with ASan/UBSan, and the Pebble SDK.
See [the development guide](docs/DEVELOPMENT.md) for validation and memory
measurements, [release work](docs/RELEASE.md) for remaining milestones, and
[HANDOFF.md](HANDOFF.md) for continuation notes.

Release workflows live in [missal-release](.agents/skills/missal-release/SKILL.md)
and [missal-appstore](.agents/skills/missal-appstore/SKILL.md), with the maintained
[release procedure](docs/RELEASING.md). They cover GitHub Releases, existing
Pebble drafts, artifact verification and recovery without rebuilding on upload.

The source EPUB, supplement, extracted text and build outputs stay local and
are excluded from Git. A fresh clone requires both source files to reproduce
the build. Selected store screenshots are retained in `release/store/`.

Release preparation covers GitHub and the Pebble App Store. The citation and
selection audit is complete; dedicated physical battery and offline soak
measurements remain unverified.
The user confirmed reading-text distribution approval on October 8, 2026.

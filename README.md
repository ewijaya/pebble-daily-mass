# Daily Roman Missal

An offline Catholic Mass readings app in development for Pebble Time 2 (Emery).
English readings are extracted from a locally supplied Pocket Missal EPUB and
bundled on the watch.

## Status

The first reader milestone works in the Emery emulator: choose the First Reading,
Psalm, acclamation, or Gospel for the pinned October 8, 2026 weekday preview,
then page through the complete text with Up/Down. Hold Up/Down to jump to the
top/bottom. Select opens reading options: Large (24 px bold), Extra Large
(28 px bold), and jump commands. Font size is saved; paragraphs have an 8-pixel gap.
Reading titles use a deep red strip; citations and introductory summaries use
smaller red text, with the scripture body in bold black. The main menu matches:
a red date strip, red season/reference text, and white-on-red selection.
A monochrome Chi-Rho symbol identifies the app in the watch launcher.
Back returns to the reading menu.
The date is manually verified against USCCB. The app uses indexed retrieval and bounded-memory decompression of the bundled
library. It does not yet select readings by date.

Menu initialization on the PT2 measured 12 ms, down from 203 ms; this excludes
firmware loading before the app runs. The database opens when the first reading
is chosen. Repacking preserves all readings while reducing the resource pack
from 950,989 to 803,312 bytes.

The resource pack is 803,312 bytes. Physical PT2 installation and opening the First Reading are verified;
responsiveness and broader device testing are still being evaluated. The pack
exceeds the SDK’s App Store allowance.

## Project layout

```text
src/c/             Watch app, resource reader and bundled decoder
resources/         Chi-Rho icon and generated reading database
tools/             EPUB extraction, packing, auditing and icon generation
tests/             Portable C reader validation
docs/              Development guide, format, audits and release checklist
data/source/       Local source EPUB (ignored by Git)
artifacts/         Generated data, screenshots and measurement records (ignored)
build/             Pebble build output (ignored)
```

The Pebble project lives at the repository root. Its package name is
`daily-roman-missal`; the watch app name is **Daily Roman Missal**. The existing
UUID is preserved so updates replace the same installed app.

## Build and verify

Place the source EPUB in `data/source/`, then run from the repository root:

```sh
uv run --with zopfli==0.4.3 python tools/extract.py data/source/*.epub --compression zopfli
python3 tests/test_reader.py
cp artifacts/readings.pmr resources/readings.pmr
pebble build
pebble install --emulator emery build/daily-roman-missal.pbw
```

Requires Python 3.9+, `uv`, a C compiler with ASan/UBSan, and the Pebble SDK.
See [the development guide](docs/DEVELOPMENT.md) for validation and memory
measurements, [release work](docs/RELEASE.md) for remaining milestones, and
[HANDOFF.md](HANDOFF.md) for continuation notes.

The source EPUB, extracted texts, screenshots and build outputs stay local and
are excluded from Git. A fresh clone requires the EPUB to reproduce the build.

Near-term goal: a Pebble App Store release while preserving offline readings.
Store-compatible packaging and automatic calendar selection remain release work.
The user confirmed reading-text distribution approval on October 8, 2026.

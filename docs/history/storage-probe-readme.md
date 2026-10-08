> Archived before the October 8 project reorganization. Paths and naming below
> are historical; use the root README for current build instructions.

# Daily Roman Missal reader prototype

An Emery-only offline reader for the pinned October 8, 2026 weekday preview.
Choose a reading with Select. Tap Up/Down for a page, hold Up/Down for top/bottom,
and return with Back. While reading, Select opens Large (24 px bold), Extra Large
(28 px bold), and jump commands. Font size is saved. Paragraph gaps are 8 pixels.
Deep red title strips and smaller red references/summaries distinguish the
heading from the black scripture text. The main menu uses the same red palette;
all four sample readings fit on screen. A 25×25 monochrome Chi-Rho marks the
app in the launcher. Source icon geometry and PNG are in `resources/images/`.
The complete extracted library is bundled; date-based selection is not yet
implemented.

Follow the [prototype instructions](../README.md) to generate the local PMR2
resource and run the host tests, then build and install from this directory:

```sh
pebble build
pebble install --emulator emery --logs
```

`src/c/pmr.c` is independent of Pebble and tested with the same C decoder on the
host. `src/c/storage-probe.c` adapts resource reads and implements the sample UI.
The technical project name is retained; the watch display name is Daily Roman Missal.

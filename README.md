# Daily Mass

An offline daily Catholic Mass readings app for Pebble Time 2.

The project starts with English readings extracted from a locally supplied Pocket
Missal EPUB. The intended app selects readings by date using the celebrations and
reading cycles represented in that source.

## Status

The extraction and storage prototype builds for Pebble Time 2 (Emery). Its bundled
resource pack is 936,593 bytes. Watch-side decompression, the reading interface,
and liturgical calendar selection are not implemented yet.

See [the prototype documentation](offline-prototype/README.md) for measurements,
verification, limitations, and reproduction instructions.

The source EPUB, generated reading database, and build outputs remain local and
are excluded from Git. To reproduce the prototype, put your EPUB in this project
folder and follow the prototype instructions.

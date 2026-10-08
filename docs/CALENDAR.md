# Offline calendar selection

The watch uses its local civil date on launch. It refreshes the menu after a
midnight/date change; an open reading keeps its original text until Back returns
to the menu. Hold Select for Today, Previous/Next day, Next Sunday and Next Holy
Day. The latter searches for the next feast or solemnity in this General Roman
Calendar; it is not a national list of holy days of obligation. Next Sunday is
strictly the following Sunday, even when browsing a Sunday. Hold Up/Down on the
main menu to step backward/forward through days; continued holds repeat. Short
presses still select readings. Browsed dates are absolute and stay fixed across
midnight, until Today is selected or the app closes.

The menu shows the date, season/week, and a celebration's rank and name. Long
names may be abbreviated on the two-line heading; Day details contains the full
name, optional memorials, reading-cycle information and selection notes.

## Scope and choices

The bundled General Roman Calendar covers January 1, 2020 through December 31,
2037. An out-of-range date displays Calendar unavailable and offers no stale
readings. The table applies the pinned generator's current rules across that
range; it is not a historical record of when individual decrees took effect.
Future calendar changes require regenerating the table and updating the app.
The range ends before the current emulator firmware's 2038 time boundary:
2037-12-31 worked, while attempted dates in late 2038/2040/2041 became 1970-01-01.
The app shows an unavailable notice for that invalid clock date.

Epiphany is January 6; Ascension and Corpus Christi remain Thursdays. National,
diocesan, religious-order and parish calendars are not included. Dates follow
the watch's local calendar, without a phone or runtime network connection.

The build joins calendar identities to explicit EPUB pages and indexes seasonal
weekdays by season, week, day and cycle. Sunday cycles A/B/C and weekday cycles
I/II are supplied by romcal, including the Advent year boundary. Unmapped dates,
missing required reading categories or mismatched Scripture resources fail the
build. No fuzzy saint-name matching or network fallback is used.

Mandatory memorials normally retain weekday readings. Proper sets are selected
for Timothy/Titus, Barnabas, the Guardian Angels, Martha/Mary/Lazarus, John the Baptist's Passion, Our Lady of
Sorrows, the Immaculate Heart, and Mary Mother of the Church. Optional memorials
are listed in Day details; the weekday remains the automatic selection.
Where only some readings are obligatory, this edition uses the whole supplied
proper set. See the [Guardian Angels indications](https://www.usccb.org/resources/world-meeting-families-2015-congress-liturgies.pdf)
and [Martha/Mary/Lazarus indications](https://www.usccb.org/prayer-worship/liturgical-year/saints-martha-mary-and-lazarus).
The rule follows [GIRM 357–358](https://www.vatican.va/roman_curia/congregations/ccdds/documents/rc_con_ccdds_doc_20030317_ordinamento-messale_en.html).

One permitted reading set is selected per date, normally the first long form in
the source. Alternatives are not concatenated. All Souls uses one set from the
Common of the Dead. All Souls is labeled Commemoration, while romcal uses its solemnity-level rank
for precedence. Lord's feasts include a second reading when celebrated on
Sunday. Palm Sunday uses the Mass readings, excluding the procession Gospel.
Christmas selects Mass during the Day. Other vigil/night/dawn texts and optional
memorial choices are not included in the selection UI. Holy Saturday explicitly says there is
no daytime Mass and that the Easter Vigil is not included in this edition.
These limitations must be included in release claims.

The EPUB lacks John 8:12–20 for Monday of Lent Week 5 in Year C. The owner
supplied the passage directly from the USCCB March 14, 2016 reading. Its five
paragraphs are appended during extraction, with source attribution, and selected
on all six affected dates. The original John 8:1–11 remains the A/B default.
See [RELEASE-AUDIT.md](RELEASE-AUDIT.md) for citation corrections, cycle-dependent
alternatives and the memorial collision fix.

## Font preference and evening preparation

Each launch opens today's menu. Reading positions, selected readings and browsed
dates are not saved. Extra Large is the default when no font choice exists;
a previously chosen Large/Extra Large preference is respected.

From 16:00 local time on Saturday, or on the evening before a solemnity, the main
menu and date menu offer Evening Mass → Tomorrow's readings. A confirmation
identifies the date and explicitly says these are the **day Mass readings**;
separate vigil texts are not included. This is an optional date shortcut, never
an automatic switch or an assertion that every evening liturgy uses that set.
Christmas offers Christmas Day readings; Holy Saturday offers Easter Sunday Day
readings with the same notice, not the Easter Vigil. The main menu updates when
16:00 is crossed, while open readings remain stable. App Glance continues to
show the actual civil date's celebration.

## Liturgical colors and launcher

The selected celebration's first `romcal.colors` entry supplies its color;
memorials and feasts therefore override the season where appropriate. Optional
memorials do not recolor the automatically selected weekday. Romcal prefers rose
on Gaudete and Laetare Sundays, and purple for All Souls (black is an alternative).
Holy Saturday has no assigned daytime color or Mass: it uses neutral gray.
Unexpected missing or unknown colors fail the build.

Menus, highlights, date headings and reading strips share this palette. Green, violet, red and rose use dark fills with white lettering.
White is represented by gold fills with black lettering and darker gold ink for
season/celebration labels on white pages. References and introductory rubrics
retain the traditional missal red; references in selected menu rows use the
highlight's contrasting lettering. Scripture stays black on white. Browsing dates changes the theme;
an open reading retains its text and theme across midnight until returning.

On exit, App Glance queues up to seven daily subtitles plus a permanent
“Open for today's readings” fallback, within the capacity supplied by firmware
(currently eight). Expirations are UTC instants at successive local midnights,
including DST. The callback uses the real watch date, independently of browsing.
Saved slices advance without starting the app, a worker, phone or network. The
queue is replenished each time the app exits. Reopen after timezone changes;
stored UTC expiry instants cannot adjust themselves to a newly chosen timezone.
Launcher labels use OT, Wk, Mem., Opt. Mem., Sol. and Commem.; Christmas/Easter
omit “Time”. Names use St./Ss.; Advent, Lent and Feast remain spelled out.
Examples: “OT · Wk 27”, “Mem. St. Teresa of Jesus”, “Sol. All Saints”. Full
calendar labels remain in the app. Long subtitles may be ellipsized by the launcher.
The app controls the subtitle, not the firmware's launcher highlight color.

## Source and reproducibility

Build tools pin `romcal` and `@romcal/calendar.general-roman` to
`3.0.0-dev.140` with npm integrity hashes. The watch runs no JavaScript.
[romcal](https://github.com/romcal/romcal) is a calendar implementation, not an
ecclesiastical approval of this app. Its MIT license is retained and bundled in
`resources/licenses/romcal.txt`. The generator and EPUB mapping still need a
broader liturgical review before public release.

After extracting the EPUB and copying `readings.pmr` to resources:

```sh
npm ci --prefix tools/calendar --ignore-scripts
node tools/calendar/generate.cjs
python3 tools/build_calendar.py
cp artifacts/calendar.bin resources/calendar.bin
python3 tests/test_calendar.py
python3 tests/test_glance.py
python3 tests/test_planner.py
pebble build
```

The builder checks that the installed reading resource exactly matches the
extracted catalog before encoding segment IDs. It emits the compressed calendar,
the raw index and an audit manifest with each date's page, section, citation and
paragraph references under `artifacts/`. Generated text and binaries stay out of
Git. Use the default 2020–2037 range; the UI and regression harness advertise it.

Two EPUB headings contain the placeholder “Scripture”, and the Good Friday
acclamation has an unrelated citation. Only their displayed
citation metadata is corrected; the underlying body remains unchanged:

- `part1220`, second reading: 2 Corinthians 1:18–22, verified against the
  [USCCB Seventh Sunday, cycle B](https://bible.usccb.org/es/node/18683).
- `part1241`, first reading: 1 Peter 1:10–16, verified against the
  [USCCB Tuesday, week 8, cycle II](https://bible.usccb.org/bible/readings/052416.cfm).

- `part1051`, acclamation: Philippians 2:8–9 replaces an unrelated Isaiah/Luke
  reference; verified against [USCCB Good Friday](https://bible.usccb.org/bible/readings/040326.cfm).

The bare numeric Psalm citation on `part0958` receives the label “Psalm”. No
Scripture wording is imported from these external reference checks.

## Format and memory

CAL1 is wrapped as a single synthetic PMR2 segment, using the same checked zlib
decoder as Scripture. CAL1 starts 24 bytes into the uncompressed PMR2 payload.
All integers are little endian and decoded explicitly, without packed C structs.

| Record | Layout |
| --- | --- |
| Header, 48 bytes | `CAL1`, then 11 uint32 values: first epoch day, day/mass/reading/part counts, offsets to days/masses/readings/parts/refs/strings |
| Day, 15 bytes | mass uint16; name/details string offsets uint32; season/week/cycles/rank/color uint8 |
| Mass, 15 bytes | count uint8; seven reading IDs uint16 |
| Reading, 8 bytes | citation string offset uint32; first part uint16; part count/kind uint8 |
| Part, 6 bytes | first reference ordinal uint32; reference count uint16 |
| Reference, 2 bytes | source segment ID in low 15 bits, rubric flag in high bit |
| String pool | NUL-terminated UTF-8; offsets relative to pool |

Epoch days start at 1970-01-01. Cycle bits 0–1 represent A/B/C; bit 2 represents
weekday II. Enum order is explicit in `tools/build_calendar.py` and the watch UI.
Color codes are 0 green, 1 purple, 2 white, 3 red, 4 rose, 5 none. The 1.1.0
layout adds one color byte per date (6,575 uncompressed bytes); its decoder
rejects the earlier 14-byte layout. App and calendar resources ship together.
The C reader validates counts, offsets, record bounds, capacities, colors and dates.

Readings split only between source paragraphs, at 6,500 bytes or 56 references.
The 8 KiB buffer and 64 paragraph boxes also accommodate the citation. Down at
the end of a part continues to the next; Up at its start returns to the previous
part. Holding Up/Down jumps to the beginning/end of the whole reading. No source
paragraph is truncated. A footer identifies the part on long readings.

A separate calendar decoder is heap allocated: the firmware metadata limits
static virtual size to 65,535 bytes despite Emery's larger total RAM budget.
The Scripture decoder stays static. See DEVELOPMENT.md for the current build
and runtime measurements.


1.1.0 validation adds all-date color-byte and App Glance summary checks,
DST/UTC midnight boundaries, and emulator screenshots for all five colors plus
neutral Holy Saturday. An open green reading remained pixel-identical across
October 14–15 midnight; Back showed the new gold memorial menu. App Glance
advanced to Teresa's memorial and eventually to its neutral fallback while the
app was closed. Evidence: `artifacts/liturgical-colors/`.

Developer clock testing: Pebble's App Glance database rejects updates whose
creation timestamp is older than the stored update. After testing future dates,
restore the real clock and reinstall the app in the emulator before checking
Today's glance. This is a firmware rule, not a calendar-selection error; do not
wipe emulator storage. See [the firmware insertion check](https://github.com/coredevices/PebbleOS/blob/main/fw/services/blob_db/app_glance_db.c).

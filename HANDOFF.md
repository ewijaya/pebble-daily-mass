# Daily Roman Missal — current handoff

Updated October 8, 2026 (Asia/Tokyo). Read docs/RELEASE.md and docs/RELEASE-AUDIT.md next.

Project-local release skills were adapted from Popeye G&W and Orationes:
`.agents/skills/missal-release` and `.agents/skills/missal-appstore`.
Their shared procedure is docs/RELEASING.md; identity is docs/release-config.json.
The original skills-maintenance task made no external changes; release preparation
subsequently committed and pushed the app and made the repository public.

## Version 1.3.0 publication in progress

Owner explicitly authorized the new banner, version bump and release of the latest
implementation on both services. This supersedes the earlier local-only constraint.
Version 1.3.0 adds continuous Mass reading, shared Settings, scalable menus/prompts,
Church mode and the crimson ribbon. The reading/calendar resources are unchanged.
No saved-place restoration is added. Shared body layout/draw width loses two pixels.

The banner now includes “FOR PEBBLE TIME 2” beneath the title. Native screenshots
are being refreshed. Evidence is in artifacts/release-1.3.0/, with earlier extensive
UI/pixel checks in artifacts/continuous-reader/ and artifacts/ribbon/.
All four host suites and a clean SDK build are being checked for the final version.
Normal closure frees all allocations; forced exit during native window opening
retains the same 148 bytes with and without the ribbon. No invalid ribbon handles.

Freeze one candidate, install those exact bytes, commit/tag the source and publish
GitHub first, then the existing store listing. Preserve older release versions.
Update this section with verified destination state when complete.

## Store description update

Owner approved replacing the final promotional sentence with St. Josemaría
Escrivá’s quotation from Christ Is Passing By, no. 154, on loving the Mass
and making it the centre of our day. Live dashboard and both public catalogs
updated and verified. Current copies are in `release/store/`; the frozen release
inputs remain historical. Verification is in `artifacts/release-1.2.0/escriva-description/`.

## Release 1.2.0 publication (current)

Owner explicitly authorized publication on both services. Both are live:
- https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.2.0
- https://apps.repebble.com/9e466762014d4b1ab248219f

The annotated tag targets frozen source `6d49fcaab2ea9a71835c6ac5197f9a2ac519b422`.
Anonymous downloads from both destinations match the frozen candidate digest.
General and Emery catalogs, rendered public page, notes and six screenshots
verified. Earlier store drafts remain unpublished. No rebuild or duplicate upload.
See the manifest and `docs/RELEASE.md` for final status. Battery soak and mobile
My Apps cache remain unobserved.

Banner sharpness fix completed: new built-in image_gen revision with larger watch
face and cleaner solid gold title. Current master/export at `release/store/`;
exact prompt in `banner-generation-prompt.md`. Store-served 720×320 image was
visually checked; both catalogs reference the replacement. Other listing fields,
icons, screenshot order and app releases unchanged. Evidence and original assets
retained under `artifacts/banner-sharpness/`; frozen candidate inputs untouched.

## Release 1.2.0 preparation (historical)

The owner selected the engraved Chi-Rho master icon and polished upright silver
watch banner with plain gold corners, and requested release preparation for
GitHub and Pebble plus a warm nostalgic description. They explicitly requested
making the existing GitHub repository public. Build/listing/artifact evidence
is in `artifacts/release-1.2.0/`; frozen package/status is recorded in
`.release/1.2.0/candidate-1/manifest.json`. Read that status before uploading.

Preparation is complete. Source commit `6d49fcaab2ea9a71835c6ac5197f9a2ac519b422`.
PBW SHA-256 `66b60438b7b0f6d3ee9a46c20d93c75fb2df322a0dfdac7e8c20de3c83ce390f`,
958,150 bytes. Exact frozen file installed successfully on physical PT2 4.38.4.
Repository is public. GitHub draft ID `406529715` targets that source commit;
its downloaded PBW matches. Store draft ID `e381ff18df3c4a9b8ad10319` is hidden
and unpublished, with updated description, banner, icons and six native captures.
Store screenshot pixels match exactly; artwork is palette-quantized by the store
and visually verified. Older drafts unchanged. Anonymous catalogs and draft PBW
return 404 as recorded; public PBW hash verification remains for publication.
Do not rebuild or upload duplicates. Promote these drafts after publication review.
See `release/store/preview.html`, `docs/RELEASE.md` and the frozen notes/manifest.

The old emulator rejects PutBytes storage initialization. Its original flash
was retained and a separate emulator-state directory created for release QA.
The same PBW installs there. Do not wipe the original emulator. The package
has a clean SDK build and all four host suites pass. Source EPUB, supplemental
text, generated databases, local references and exploratory art remain ignored.

The owner's gaming/development usage report does not establish a dedicated
physical offline/midnight/battery soak. No fabricated battery result.

## Saved-place feature removed (current 1.2.0 development build)

The owner reversed the request to remember the reader's place. Removed bookmark
storage/validation, debounced save timer, restore logic and Resume/Today prompt.
Launch always opens today's menu; opening a reading starts at part 1/top.
Retired persistence key 2 is deleted on launch; font preference key 1 remains.
Extra Large default, date shortcuts, held date stepping, Evening Mass prompt,
App Glance and seasonal colors/red rubrics are preserved. Do not restore resume.
`planner.c/.h` and its tests now cover date planning only. Build/verification
and current artifact identity are in `artifacts/remove-resume/`.
Static footprint is 63,138 bytes; available heap before allocations 67,934.
Physical PT2 install succeeded (firmware 4.38.4). Both initial launch and a
stop/start relaunch show today's menu without Resume; see `physical.png` and
`physical-reopened.png`. Host planner checks passed under ASan/UBSan.
Emulator update attempts were rejected even after restarting the emulator;
`verify.py` screenshots are failed-install states, not passing UI evidence.
PBW: 958,150 bytes; SHA256 23b856e03bc1d96ec4df6db29cb938c3034bde7c8f08e62c606a9fc01e90f577.
Earlier reader-preparation evidence below describes the removed feature.
No public release, store upload, commit or push was requested by this change.

## Reader preparation update (superseded 1.2.0 build)

Implemented the user's latest four requests:
- Extra Large is the default when no font preference is saved. Explicit Large
  choices persist.
- Six-hour Resume/Today offer for a saved absolute date, reading, part, pixel
  offset and font; browsed dates/menu selections also resume. The 28-byte key-2
  record validates calendar/text identities and bounds. Saves debounce 400 ms
  and flush on normal exit; key 1 remains the font preference.
- Date menu adds Next Sunday and Next Holy Day (next feast/solemnity, not a
  national obligation list). Main-menu holds Up/Down repeat previous/next days;
  short presses select readings. Browsed dates stay fixed across midnight.
- After 16:00 Saturday or the eve of a solemnity, Evening Mass offers tomorrow's
  readings with a confirmation saying these are day readings and separate vigil
  texts are absent. It never switches automatically. App Glance remains civil-date based.

`src/c/planner.c/.h` owns portable date/search/bookmark rules. `calendar_rank`
reads only the rank byte for fast searches. `tests/test_planner.py` validates
all civil-date/next-Sunday conversions, feast precedence, 16:00 and six-hour
boundaries, DST dates, invalid/changed records and 2037 limits. Calendar and
App Glance regressions also pass. All requested UI states were inspected in
Emery. Resume screenshots match exactly for a Gospel page, browsed Sunday,
Palm Sunday Gospel part 3, Large font, and yesterday's reading after midnight.
Resume prompt ready 71 ms; normal first menu 47 ms; no retained heap on exits.
Evidence/build identity: `artifacts/reader-planning/`. The emulator was returned
to the real clock, today's menu, and Extra Large after tests.

PBW version 1.2.0, 959,882 bytes; SHA256:
ccdca8199660a816e4fc732c3ea69e681691f9013ce361eb46032a6e404ed764.
Resources unchanged at 937,965 bytes. Static footprint 64,936; heap before
allocations 66,136; observed reader heap about 31,000 bytes. The static 65,535-byte metadata
ceiling now has only 599 bytes spare: recheck before further code growth.
Physical installation succeeded: `physical-diagnostic.log` reports success,
and `physical-diagnostic.png` shows the live PT2 Resume First Reading prompt.
The owner reopened the phone app/Dev Connection, but the initial retry still
reported failure. Read-only runtime inspection showed the latest 66,136-byte
heap build was already running despite that report. Closing the running Missal
(UUID-checked AppRunStateStop), then retrying through the normal phone installer
succeeded. Avoid assuming phone failure means the watch is unchanged; inspect
runtime/screenshot before repeating installs. No direct-protocol installer,
uninstall, reset or wipe was used. Final physical resume prompt: 207 ms.
No commit, push,
GitHub release or store upload. Existing hidden store draft remains 1.0.1.
Earlier task/install details below are historical and superseded by this section.

## Compact App Glance labels (earlier 1.1.0 build)

The user requested shorter launcher text. `glance.c` now uses OT, Wk, Mem.,
Opt. Mem., Sol. and Commem.; Christmas/Easter omit “Time”. St./Ss. were already
used. Advent, Lent and Feast stay readable. Examples: “OT · Wk 27”,
“Mem. St. Teresa of Jesus”, “Sol. All Saints”. Full in-app labels are unaffected.
All 6,575 glance summaries and existing date/expiration tests pass ASan/UBSan.
Evidence and the latest PBW identity are in `artifacts/compact-glance/`.
Build footprint 60,890 bytes; heap before allocations 70,182. Resources unchanged.
Emulator install/screenshot pass. Physical update returned “App install failed”;
an async question asks the owner to reopen the Pebble phone app and reply Ready.
Retry this exact PBW after that reply. Earlier hybrid build remains installed.

## App Glance and liturgical-color update (1.1.0)

The latest user requests are implemented: offline launcher App Glance and a
liturgical palette, followed by the user's explicit **traditional hybrid** choice:
seasonal date/title strips and highlights, red references/introductory rubrics,
black Scripture on white. Selected menu references use contrasting highlight
text. White celebrations use gold/black; green, violet, red and rose use dark
fills/white. Season and celebration labels keep the day's color. Holy Saturday
has no assigned daytime color and uses neutral gray.

Romcal's first color is encoded in one extra CAL1 byte per day. Rose is preferred
on Gaudete/Laetare, purple on All Souls; feast/memorial precedence determines the
color even when weekday readings remain. The decoder rejects old 14-byte day
records; new records are 15 bytes. App/resource must ship together. All 6,575
dates/colors and 2,523 reading parts pass host ASan/UBSan checks.

`src/c/glance.c` formats real-clock summaries and finds the next local midnight
using firmware timezone rules. On exit `main.c` saves seven daily slices plus a
permanent “Open for today's readings” fallback, respecting SDK capacity. No
worker, phone, network or launch-time extra work. Browsing dates does not change
which days populate the glance. Open/exit the app to replenish the week; reopen
after timezone changes. Native launcher text may ellipsize long feast names.
`tests/test_glance.py` passes all 6,575 summaries plus DST, leap, year-end and
out-of-range tests. Add it to normal release validation.

Emulator verification: six color states, gold options menu, hybrid red rubrics,
midnight green-to-gold menu update, unchanged open reading across midnight,
Oct 14 weekday glance -> Oct 15 Teresa memorial -> expired-queue fallback.
Startup remains ~50 ms from main to first menu draw; no retained heap on exit.
Final emulator is restored to today's date, with the correct Week 27 glance.
Evidence and final build/install records are in `artifacts/liturgical-colors/`.

**Emulator date-test trap:** firmware AppGlanceDB rejects a new entry if its
creation time is <= the old entry's. After future-date tests, restore real time
and reinstall the app before checking today's glance; otherwise a saved future
glance persists even though the app computes today's summary correctly. Confirmed
in coredevices/PebbleOS `fw/services/blob_db/app_glance_db.c`. Do not wipe storage.

The new resource pack is 937,965 bytes (110,611 headroom); readings 799,578,
calendar 131,845. The calendar grows only 1,608 compressed bytes. Build RAM
footprint 60,868, available heap before allocations 70,204. Resource and content
counts otherwise remain unchanged. See `final-build.json` for PBW size/hash.
The existing **hidden store draft remains 1.0.1** with earlier screenshots;
1.1.0 has not been uploaded/published/committed/pushed. Release preparation must
freeze the latest build and refresh listing/screenshots before promoting it.
The final hybrid build installed successfully on the physical PT2 via CloudPebble
(`hybrid-physical-install.log`) and the emulator. The final physical screenshot
shows the user-opened Psalm: green title strip, red citation and black text.
The earlier measurements below
describe previous builds.

## Release preparation update

The user approved release preparation. A hidden, unpublished store draft was
created successfully: app 9e466762014d4b1ab248219f, current release 1.0.1
(a5a92545663b49fdb6831dba). Previous 1.0.0 is also unpublished. Visibility and
anonymous Emery 404 were verified. All five screenshots, icons and listing text
are uploaded. The store accepted the complete bundle; the legacy 256 KiB SDK
warning is not a current upload blocker. Dashboard:
https://developer.repebble.com/dashboard/apps/9e466762014d4b1ab248219f/edit

The audit corrected calendar collisions, weekday Holy Family, Year A Gospel
alternatives and wrong EPUB headings. The source gap in John 8:12–20 is now
resolved: the user pasted the complete USCCB March 14, 2016 passage directly.
It is saved in ignored data/source/john-8-12-20.json. tools/supplements.py loads
it; extract.py appends five segments, preserving all original EPUB segment IDs.
The calendar selects it for all six Year C Mondays. No missing-Gospel notice
remains. The scripture resource now has 13,168 segments, still 1,006 EPUB pages.
A fresh checkout requires BOTH local source files, not only the EPUB.

Final ASan/UBSan checks pass all 6,575 dates, 2,523 parts and all 13,168 text
segments/1,006 pages, including corruption/capacity/short-read checks. Twenty
emulator restarts passed before the supplement, with zero retained allocations
and 36,760 bytes reader heap free. The final supplement was visually inspected
at its start and end; top/bottom jumps work. Emulator time restored to today.

Current metrics: readings 799,578 bytes; calendar 130,237 bytes; 755 mass sets;
2,516 readings; resource pack 936,357 bytes; PBW 953,102 bytes. SHA256:
82874dae98d60231e387f9591d4ba3e3032773f34b861cc27d4d1ac16d1624a0.
Evidence: artifacts/release-prep/. Store assets: release/store/. Author Edward
matches the signed-in developer account. No commit/push or public publication.
A physical PT2 offline/battery soak remains before public release.

The installed CLI publish command ignores its unpublished flag and hardcodes
public visibility; do not use it for drafts. Ignored upload_draft.py created
with visibility=hidden and isPublished=false; update_draft.py uploaded 1.0.1
and updated the listing while preserving hidden status. Both scripts guard
against duplicate creation. Auth tokens/cookies are held only in memory.

An emulator transport timeout during an earlier reinstall was recovered with
pebble kill (no wipe), then a throttled reinstall. Final Emery install succeeded.
Physical 1.0.1 install succeeded after the owner reopened the Pebble phone app.
Earlier phone-side failures are recorded, but the final retry succeeded on
PT2 hardware 18 / firmware 4.38.4. Log: physical-install-reconnected.log.
physical-final.png was visually inspected: October 8, Ordinary Time Week 27,
correct reading menu. User selection is on Psalm; palette differs from Emery.
The preceding calendar handoff below records the earlier installed build; its
metrics and source-gap descriptions are superseded by this release update.

## Current task and state

The user requested automatic local-date and liturgical-calendar selection and
chose the **General Roman Calendar**. This is implemented for **2020–2037**.
The watch selects today's readings offline, shows season/week and celebration
name/rank, and lists full names and optional memorials in Day details. Hold
Select on the main menu for Today, Previous day or Next day. Browsing resets
on relaunch. Midnight updates a visible menu; an open reading stays unchanged
until returning to the menu.

The app remains a development build, not a public release. See RELEASE.md:
store resource allowance, independent calendar/content review, public listing
assets and broader physical-device tests remain. National calendars, vigil
selection and optional memorial choices are outside this edition. Holy Saturday
shows an explicit daytime/no-Mass notice and says the Vigil is not included.

Project brand: **Daily Roman Missal**. Installed name: **Roman Missal** for BOTH
shortName and longName, with the original Chi-Rho launcher icon. The user
confirmed that this compact label fixes the physical launcher's truncation.
UUID remains `bfbd18b3-4a25-44b1-99f9-2de4fbe0f076`.

## Files and implementation

The repository root is the Pebble project. `pebble build` produces
`build/daily-roman-missal.pbw`; wscript explicitly sets that bundle name.

- `src/c/main.c`: menu, date browsing, automatic refresh, reader and font options.
- `src/c/calendar.{c,h}`: bounded CAL1 date lookup and reading-part decoder.
- `src/c/pmr.{c,h}` and `vendor/tinf/`: existing validated compressed resource reader.
- `tools/calendar/`: pinned romcal 3.0.0-dev.140 and matching General Roman pack;
  build-time only, with package-lock. MIT license retained and bundled.
- `tools/build_calendar.py`: source-aware EPUB section parsing, explicit calendar
  identity mappings, reading alternatives, metadata corrections, part generation.
- `tests/test_calendar.py`, `tests/calendar_test.c`: calendar regressions and
  exact watch C reader under ASan/UBSan.
- `docs/CALENDAR.md`: scope, rules, source references, exceptions, binary format.
- `data/source/*.epub`, `resources/readings.pmr`, `resources/calendar.bin`,
  `artifacts/`, `build/`, node_modules: local/generated and ignored by Git.

The source library remains 1,006 pages / 13,163 unique segments. Its 799,048-byte
PMR2 resource is unchanged. The calendar builder verifies it exactly matches the
extracted catalog before assigning segment IDs. Final CAL1 resource: 129,722
bytes; 6,575 days, 747 mass sets, 2,514 readings and 2,521 parts.

Calendar conventions: Epiphany January 6, Ascension/Corpus Christi Thursday.
Memorials retain weekday readings unless proper readings apply. Proper exceptions
include Timothy/Titus, Barnabas, Guardian Angels, Martha/Mary/Lazarus, John the
Baptist's Passion, Our Lady of Sorrows, Immaculate Heart and Mary Mother of the
Church. When only some readings are obligatory, the whole supplied proper set
is selected. Optional memorials are listed without replacing the weekday default.
All Souls uses a permitted Common of the Dead set; its DISPLAY rank is
Commemoration (romcal uses a solemnity-level rank for precedence).

Three erroneous/missing source citations are corrected only in calendar display
metadata: part1220 second reading (2 Cor 1:18–22), part1241 first reading
(1 Pet 1:10–16), part1051 Good Friday acclamation (Phil 2:8–9). Sources and exact
exceptions are documented in CALENDAR.md. No Scripture wording was imported
from the web. The existing October 8 EPUB/USCCB wording difference remains.

Long readings split at source paragraph boundaries (6,500 bytes / 56 refs).
The reader retains its 8 KiB buffer, 64 paragraph boxes, bold Large/Extra Large
fonts, 8-pixel paragraph gaps and red rubrics. Down crosses to the next part;
Up at the start crosses back; holding Up/Down jumps to the whole reading's
beginning/end. The footer shows part numbers. Font persists under key 1.

A separate Calendar (~33 KiB including PMR buffers) is heap allocated to stay
below the firmware's 65,535-byte static virtual-size metadata limit. Scripture
still opens lazily. Final build static RAM footprint 59,546 bytes; available
heap before runtime allocations 71,526 bytes. Emulator reading free heap was
36,760–36,908 bytes. Resource pack 934,106 bytes, below firmware's 1,048,576
limit; SDK still warns about the 262,144-byte App Store allowance.

## Validation and evidence

Host checks pass for all 6,575 dates and 2,521 parts: exact resource references,
source text loading, buffer limits, leap/invalid/out-of-range dates and bounds.
Regressions cover A/B/C and I/II, Advent rollover, Annunciation transfer,
Sunday precedence, proper memorials, fixed General Roman feasts, Lord's feasts
on weekdays vs Sundays, and alternative boundaries.

Emulator evidence is in `artifacts/calendar-validation/`:
- March 29, 2026 Palm Sunday: full Matthew Passion across 3 parts, automatic
  part transition and top/bottom jumps. Preparation 193–315 ms depending on part.
- October 7 Rosary memorial: title and weekday Galatians reading.
- November 1 All Saints: Solemnity title, Sunday reading set and Day details.
- Midnight October 8→9: menu updates automatically; open reading screenshots
  are byte-identical across midnight, Back refreshes to October 9.
- Out-of-range 2019: explicit unavailable state, no stale readings.
- Main-to-first-menu draw measured 51 ms with calendar (excludes firmware load).
- Future-date testing: 2037-12-31 works, late 2038/2040/2041 become 1970-01-01
  on current emulator firmware. Therefore the originally explored 2040 horizon
  was reduced to 2037. Do not promise functioning watch dates after 2037.

Some evidence predates the final small data corrections/range reduction; the
final host run and build cover the final resource. Calendar audit manifests,
raw binary, local test scripts and screenshots are under ignored artifacts/.

The final bundle installed successfully on both Emery and the physical PT2 via
CloudPebble. The physical screenshot shows October 8, 2026 / Ordinary Time
Week 27 and matches the emulator pixel-for-pixel. Its UUID, both compact names,
hash and install result are recorded
in `artifacts/calendar-validation/final-build.json`. No public publication or
Git push was performed.

## Toolchain, install and continuation

Use the local `pebble-sdk-inspector` skill before unfamiliar APIs/commands.
Pebble Tool 5.0.40 / SDK 4.33.1, macOS arm64. SDK root:
`~/Library/Application Support/Pebble SDK/SDKs/4.33.1`.
PT2 firmware v4.38.4, hardware obelix18; physical connection via CloudPebble.
Sideloading is already authorized. Do not change the UUID or the compact name.

Reproduction is in README. Install explicitly:

```sh
pebble install --emulator emery build/daily-roman-missal.pbw
pebble install --cloudpebble build/daily-roman-missal.pbw
```

The SDK can miss an appinfo-only bundle change when the binary is unchanged;
archive/remove the old PBW or clean-build, and verify metadata INSIDE the ZIP.
Phone `PbwApp.toLockerEntry` reads longName and sends the locker title as the
watch name, which is why changing only shortName did not fix truncation.
The successful previous label install is recorded in
`artifacts/launcher-investigation/physical-install.json`.

Each CLI `--emulator` connection resets the clock to host time. Date simulation
and screenshots therefore use one `pebble repl` session. Return it to real time
after testing. Do not change physical watch time. Do not print broad process
command lines: the emulator bridge can contain credentials. Do not use pebble
wipe. Preserve `artifacts/startup-investigation/qemu_spi_flash-before-reset.bin`.

Last pushed commit before this work: e83368c on main, private origin repository
`ewijaya/pebble-daily-mass`. The current changes include the previous physical
launcher-label fix and this calendar feature. No commit/push/publication is
requested in this turn. Earlier development history is in docs/history/.

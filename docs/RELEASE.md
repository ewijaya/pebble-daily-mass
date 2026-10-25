# Public release — 1.5.3

Daily Roman Missal 1.5.3 is published on GitHub and the Pebble App Store as of
October 25, 2026. The owner requested the store update to make both destinations
consistent. Whole-line repetitions of the opening Psalm response now receive
`R.`. Reading wording and bundled resources are unchanged.

- [GitHub Release](https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.5.3)
  — public, Latest, release ID `423201172`.
- [Pebble App Store](https://apps.repebble.com/9e466762014d4b1ab248219f)
  — listed and published, release ID `d45cee0c957b4b37929e7d75`. Both general
  and Emery catalogs verify 1.5.3; the canonical website remains cached on 1.5.2.
- Source commit: `32f16aa537d3fe4772b0d5235ee37cc36c7c504f` on
  `fix/psalm-response-markers`, pushed to that branch and `main`. Annotated tag
  `v1.5.3` is pushed and verified against the source commit.
- Frozen candidate: `.release/1.5.3/candidate-1/manifest.json`.
- PBW: 972,952 bytes; SHA-256:
  `50c8549c56181e17836661786d19bac7d4b249cfd1f615f65a68fe34811232bf`.

All five ASan/UBSan suites passed earlier in this session for the same runtime
and test sources. Their hashes are recorded in `host-results.json`; only release
metadata and documents changed afterward. A clean SDK build and embedded PBW
identity/version checks passed. Final-build Emery checks cover repeated responses
in both font sizes and themes, canticle responses, Easter's alternative response,
and the Settings version. Screenshots were inspected; emulator preferences and
real time were restored.

Resources are byte-identical to published 1.5.2 at 945,316 bytes. Static footprint
is 61,496 bytes (+416); linker free heap is 69,576 bytes. The existing SDK
resource-size and linker warnings remain. The exact frozen PBW installed and
launched on PT2 4.41.2 after the owner reopened the companion app. Two earlier
phone-side attempts failed. Expected running UUID, fault-free logs and an
inspected native main-menu screenshot verify launch. Explicit owner readability
and navigation confirmation for this final version remains unobserved.
Dedicated physical offline/overnight/battery measurements remain unobserved.

GitHub title, notes, tag and Latest state verify. Both authenticated and anonymous
GitHub downloads, the store draft download and the public store catalog PBW match
the frozen digest. The store dashboard and general/Emery catalogs verify 1.5.3,
notes, description, platform support and artwork order. Existing description,
artwork, screenshot order, visibility, unrelated metadata and all older releases
were preserved. Only the new 1.5.3 release was added and published.

The canonical store website still shows 1.5.2 and links its old PBW; its separate
changelog is also stale after three bounded read-only checks. Publication is
verified in the catalogs, but those web surfaces remain pending. Phone My Apps
cache refresh has not been observed. Use GitHub for the verified 1.5.3 download
until the website refreshes; do not re-upload or republish identical bytes.
Evidence: `artifacts/release-1.5.3/` and `artifacts/psalm-responses/`.
Unrelated `docs/RELEASING.md` and `release/reddit/` edits are preserved.

---

# Public release — 1.5.2

Daily Roman Missal 1.5.2 is published on GitHub and the Pebble App Store.
On October 14, 2026, the owner confirmed successful PT2 testing and approved
publication to both destinations and the proposed new-branch commit.

- [GitHub Release](https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.5.2) — public, Latest, release ID `412322671`.
- [Pebble App Store](https://apps.repebble.com/9e466762014d4b1ab248219f) — listed and published, release ID `29a2a665ac00417a993583e4`.
- Source commit: `47fd5db0b29124746340c9ce14db10812b568fa6`; annotated tag `v1.5.2`, pushed and verified.
- PBW: 972,536 bytes; SHA-256:
  `68358b5c780658161f4c84667c59d03336a5b723da53d130ca311448ba86620d`.

The date shortcut selector now wraps in both directions. Down from the last
item returns to Today; Up from Today selects the last item. Evening Mass
remains reachable as the last row when offered. Held Up/Down repeats every
300 ms. Calendar search and reading selections are unchanged.

All four ASan/UBSan host suites passed. A clean SDK 4.33.1 build and embedded
identity/version checks passed. Final-version Emery checks verify five/six-row
wraparound, Today and Next Feast selection, the Evening Mass prompt, Back and
held navigation. Screenshots were inspected and real emulator time restored.
The first emulator transfer failed; restarting without clearing state recovered
installation. Resources remain byte-identical at 945,316 bytes. Static footprint
is 61,080 bytes (+216 from 1.5.1, below 65,535); linker free heap is 69,992 bytes.
Existing SDK resource-size and linker warnings remain.

The exact frozen PBW installed and launched on PT2 4.40.0 through CloudPebble.
Running UUID, fault-free logs and a native screenshot verify launch. The owner
then tested wraparound successfully and approved release. Dedicated physical
offline/overnight/battery measurements and phone My Apps refresh remain unobserved.

GitHub tag, notes, Latest and authenticated/anonymous downloads verify. Store
draft and public downloads match the frozen digest. General/Emery catalogs
verify version, notes, description and artwork order. Description, assets,
visibility, unrelated listing fields and all older releases were preserved.
The canonical store page verifies version, description and linked PBW. The
separate web changelog remains stale after three bounded read-only checks.

Frozen candidate: `.release/1.5.2/candidate-1/manifest.json`. Evidence:
`artifacts/release-1.5.2/` and `artifacts/date-menu-wrap/`. Source was committed
on `fix/date-menu-wrap` and pushed to that branch and `main`; tag `v1.5.2`
identifies the frozen app source. Unrelated `docs/RELEASING.md` and
`release/reddit/` edits remain untouched and excluded from commits.
Never rebuild or overwrite the published bytes.

---

# Public release — 1.5.1

Daily Roman Missal 1.5.1 is published on GitHub and the Pebble App Store.
On October 14, 2026, the owner confirmed successful PT2 testing and approved
publication to both destinations and the proposed new-branch commit.

- [GitHub Release](https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.5.1) — public, Latest, release ID `412314473`.
- [Pebble App Store](https://apps.repebble.com/9e466762014d4b1ab248219f) — listed and published, release ID `fc6b92518d424c8e8061eb7c`.
- Source commit: `70baa83450ae1cec0d78ad657347a8ae9cd06ca0`; annotated tag `v1.5.1`, pushed and verified.
- PBW: 972,320 bytes; SHA-256:
  `07fe7628a9a7bd1dd51aaec11355c1747a18757acd609404f19442bc572c42f0`.

The date shortcut is now **Next Feast**, retaining the “Feast or solemnity”
subtitle. Calendar search, reading selections and navigation are unchanged.

All four ASan/UBSan host suites passed. A clean SDK 4.33.1 build and embedded
identity/version checks passed. Final-version Emery checks verify one-line
labels and subtitles in Large/Extra Large and Light/Church, unchanged feast
and solemnity targets, and restoration of preferences and real time.
Resources are byte-identical at 945,316 bytes. Static footprint is 60,864 bytes
(10 fewer than 1.5.0, below 65,535); linker free heap is 70,208 bytes. Existing
SDK resource-size and linker warnings remain.

The exact frozen PBW installed and launched on PT2 4.40.0 after the owner
confirmed the developer connection. Two initial attempts failed; the subsequent
attempt succeeded. Running UUID, fault-free logs and a reading screenshot verify
launch; normal exit retained zero app allocations. The owner then tested the
shortcut successfully and approved release. Dedicated offline/overnight/battery
measurements remain unobserved; phone My Apps cache refresh is not claimed.

GitHub tag, notes, Latest and authenticated/anonymous downloads verify. Store
draft and public downloads match the frozen digest. General/Emery catalogs
verify the version, notes, description and artwork order. Description, assets,
visibility, unrelated listing fields and all older releases were preserved.
The canonical store page and separate changelog verify the release.

Frozen candidate: `.release/1.5.1/candidate-1/manifest.json`. Evidence:
`artifacts/release-1.5.1/` and `artifacts/next-feast-label/`. Source was committed
on `fix/next-feast-label` and pushed to that branch and `main`; tag `v1.5.1`
identifies the frozen app source. Unrelated `docs/RELEASING.md` and
`release/reddit/` edits remain untouched and excluded from commits.
Never rebuild or overwrite the published bytes.

---

# Public release — 1.5.0

Daily Roman Missal 1.5.0 is published on both services. On October 14, 2026,
the owner confirmed successful PT2 feature testing and explicitly approved release.

- [GitHub Release](https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.5.0) — public, Latest, release ID `412276545`.
- [Pebble App Store](https://apps.repebble.com/9e466762014d4b1ab248219f) — listed and published, release ID `36ff8ebd5c8f43cdb37e70fc`.
- Source commit: `37e273219002477463ec22f54a338cd181a8acdd`; annotated tag `v1.5.0`, pushed and verified.
- PBW: 972,333 bytes; SHA-256:
  `8123048c819cee17dd95d8b9afae512d1c4f531ee8b889a9e2240da63354ca7b`.

Choose Mass adds Christmas Day/Vigil/Night/Dawn, Easter Vigil (all seventeen
entries with A/B/C Gospel and baptism-dependent psalm), Pentecost simple/extended
forms, and the vigils of John the Baptist, Peter and Paul, and the Assumption.
Evening shortcuts open the relevant chooser. Existing daily defaults remain
unchanged. Full vigil rites/intervening prayers are outside the feature scope.

All four ASan/UBSan host suites passed. All 6,575 daily selections match the prior
manifest. The clean SDK 4.33.1 build and embedded identity/version checks pass.
Final frozen-byte Emery checks verify version 1.5.0 and representative
Christmas/Easter/Pentecost forms; broader development coverage includes all forms,
seventeen Easter entries, forward/back transitions, font/theme, evening routes
and midnight behavior. Emulator time was restored.

The exact frozen PBW installed and launched on PT2 firmware 4.40.0 through
CloudPebble, verified by running UUID, fault-free startup logs and screenshot.
Normal exit retained zero app allocations. The owner subsequently tested the
feature successfully and approved this release. Dedicated offline/overnight
and battery measurements remain unobserved; phone My Apps refresh is not claimed.

Resources: 945,316 bytes (+7,351 from published 1.4.3). Static footprint: 60,874
bytes; linker free heap: 70,198 bytes. Scripture resource is unchanged. Existing
SDK resource/linker warnings remain; the build fits the recorded Emery resource
limit.

GitHub tag, notes, Latest and authenticated/anonymous downloads verify. Store
draft/public downloads match the frozen digest. General/Emery catalogs verify
1.5.0, notes and the approved description. The canonical web page still serves
its older version, description and linked PBW after three read-only checks;
the separate web changelog is also stale. Those web cache refreshes remain pending.
Use the verified GitHub download while the web page catches up. Main/Emery
descriptions, artwork, screenshot order, visibility, older releases and unrelated
listing fields were checked against the baseline; only approved fields changed.

Frozen candidate/provenance: `.release/1.5.0/candidate-1/manifest.json`.
Evidence: `artifacts/release-1.5.0/`; implementation evidence:
`artifacts/vigil-readings/`. Source/tag are pushed. Unrelated
`docs/RELEASING.md` edits and `release/reddit/` are preserved and excluded from
release commits. Never rebuild or overwrite these released bytes.

---

# Public release — 1.4.3

Daily Roman Missal 1.4.3 is published on both services, following the owner's
request to release the implemented Controls page on October 12, 2026.

- [GitHub Release](https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.4.3) — public, Latest.
- [Pebble App Store](https://apps.repebble.com/9e466762014d4b1ab248219f) — listed, release ID `f9f3900b32344831b9d2b12d`.
- Source commit: `84bb608a5ead218f481e706af3f388fe091c5967`; annotated tag `v1.4.3`.
- PBW: 962,130 bytes; SHA-256:
  `852671cd3537ae6ac914f869651470f62569c78e0f77dce67575e195e7875e34`.

Controls appears after Settings on the main menu. It collects reading, date
and touch instructions in the normal bold reading font and selected theme.
The faint Settings hint and date-shortcut instruction in Day details are removed.
Controls scrolls without advancing into readings, including on dates with no Mass.

All four ASan/UBSan host suites and a clean SDK 4.33.1 build passed. Final-build
emulator captures/checks verify Large/XL, light/dark, bounded scrolling, Settings,
Back, menu wrapping, reading exit and Holy Saturday access. Real time was restored.
Normal exit retains zero app allocations. An emulator installer failure cleared
after restarting without wiping saved data. The no-readings test's date setup
was corrected and its final successful log is `empty-day-verified.log`.

The exact frozen PBW installed and launched on PT2 firmware 4.38.4, verified by
running UUID, startup log and screenshot. Initial phone installation reported
failure; after reconciling watch state, one retry with Missal closed succeeded.
Physical Controls readability/navigation, dedicated offline/overnight and battery
soak remain unobserved. Phone My Apps refresh is not claimed.

Resources are byte-identical to 1.4.2 at 937,965 bytes. Static RAM is 59,020 bytes
(+350); linker free heap is 72,052 bytes. Existing SDK warnings remain.

GitHub tag, notes, Latest and authenticated/anonymous downloads verify. Store
draft/public downloads match the frozen digest. General/Emery catalogs verify
1.4.3 and the updated description. Canonical page version, description and linked PBW verified; separate changelog remains cached at 1.4.2 after three read-only checks.
Artwork, screenshot order, prior releases and unrelated listing fields are preserved.

Frozen candidate/provenance: `.release/1.4.3/candidate-1/manifest.json`.
Evidence: `artifacts/release-1.4.3/`. Source/tag are pushed. Unrelated
`release/reddit/` remains untracked and excluded. Never rebuild these released bytes.

---

# Public release — 1.4.2

Daily Roman Missal 1.4.2 is published and verified on both services, following
the owner's October 12 request to release the new Settings hint.

- [GitHub Release](https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.4.2) — public, Latest.
- [Pebble App Store](https://apps.repebble.com/9e466762014d4b1ab248219f) — listed, release ID `4158bc94e43d443084c27915`.
- Source commit: `b5a6b791d0c61927a55079fa7da3ffc7e089fb6d`; annotated tag `v1.4.2`.
- PBW: 961,782 bytes; SHA-256:
  `bbe17e01f2c0512440706528245e1be4783ffc662717b754f4603b8c1fb851e4`.

Settings now shows “In reader: Hold Select to exit” above the version. The
existing footer layer reserves space for the hint and follows the selected
theme. Font/theme controls remain visible in both Large and Extra Large.
Reader button behavior is unchanged from 1.4.1.

All four ASan/UBSan host suites and a clean SDK 4.33.1 build passed. Final
emulator captures verify the footer in Large, Extra Large and Church mode;
tap/sub-threshold Select, font/theme toggles, Back and hold-to-exit checks pass.
Normal exit retains zero app allocations. An unresponsive emulator was restarted
without wiping its data. A test harness startup-timing assumption was corrected
before the successful run in `emulator-qa-verified.log`.

The exact frozen PBW installed successfully on PT2 firmware 4.38.4. Running UUID,
fault-free startup logs and menu screenshot verify its launch. Physical hint
readability, dedicated offline/overnight and battery soak remain unobserved;
phone My Apps cache refresh is not claimed.

Resources remain byte-identical to 1.4.1: 937,965 bytes. Static RAM is 58,670
bytes (+30); linker free heap is 72,402 bytes. Existing SDK resource/linker
warnings remain. No resource/text regeneration was needed.

GitHub tag, notes, Latest and authenticated/anonymous downloads verify. Store
draft/public downloads match the frozen digest. General/Emery catalogs, canonical
page and linked PBW verify 1.4.2. The separate web changelog still shows cached
1.4.1 after three bounded read-only checks; its refresh remains pending.
Main/Emery descriptions, artwork,
screenshot order, older releases and unrelated listing fields are preserved.

Frozen candidate/provenance: `.release/1.4.2/candidate-1/manifest.json`.
Evidence: `artifacts/release-1.4.2/`. Source/tag are pushed; unrelated
`release/reddit/` remains untracked and excluded. Never rebuild or overwrite
this released version.

---

# Public release — 1.4.1

Daily Roman Missal 1.4.1 is published and verified on both services. The owner
confirmed the physical shortcut worked and approved release on October 12, 2026.

- [GitHub Release](https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.4.1) — public, Latest.
- [Pebble App Store](https://apps.repebble.com/9e466762014d4b1ab248219f) — listed, release ID `c5fc68e6ec7048eba5e53681`.
- Source commit: `98a4e21c43a6f089f88f705767dac925fead71b9`; annotated tag `v1.4.1`.
- PBW: 961,750 bytes; SHA-256:
  `2f25a5891f5862e4930f95c0b940c823c434b8dcaf6f933cad934da6cb631034`.

Hold Select for 700 ms in the reader to exit to the launcher; Back there returns
to the watchface. Tap Select opens font size/theme Settings on release. The
main-menu hold-Select date shortcut remains available. No on-watch shortcut hint
is included; adding one was suggested for a follow-up update.

All four ASan/UBSan host suites and clean SDK build passed. Final emulator
checks cover tap/sub-threshold hold, font/theme controls, Back, date shortcuts
and exit with zero remaining app allocations. Resources are byte-identical to
1.4.0 at 937,965 bytes. Static RAM is 58,640 bytes (+48); linker free heap is
72,432 bytes. Existing legacy SDK resource/linker warnings remain.

The exact frozen candidate installed and launched on PT2 firmware 4.38.4;
running UUID, fault-free startup logs and a menu screenshot were verified.
The first transfer reported failure; reconciliation followed by one retry with
Missal closed succeeded. The owner subsequently tested and approved the feature.
Dedicated offline, overnight and battery soak remain unmeasured; phone My Apps
cache refresh has not been observed.

GitHub's tag, notes, Latest status and authenticated/anonymous PBW downloads
verify. Store draft/public downloads and the canonical page's download match
the frozen digest. General and Emery catalogs, canonical page and changelog
show 1.4.1 and approved shortcut notes/description. Artwork, older releases and
unrelated listing fields were preserved.

Frozen bytes, provenance and publication state: `.release/1.4.1/candidate-1/`.
Evidence: `artifacts/release-1.4.1/`. Source/tag pushed; unrelated `release/reddit/`
remains untracked and excluded. Do not rebuild or overwrite this version.

---

# Public release — 1.4.0

Daily Roman Missal 1.4.0 is published and verified on both services. The owner
requested release on October 9, 2026 after approving the revised PT2 reading flow.

- [GitHub Release](https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.4.0) — Latest, release ID `407383762`.
- [Pebble App Store](https://apps.repebble.com/roman-missal_9e466762014d4b1ab248219f) — listed, release ID `2dd7571bc4614c55b10c5672`.
- Source commit: `d8ae12c85f8953fd866261f77b42fa1b71967fa7`; annotated tag `v1.4.0`.
- PBW: 961,701 bytes; SHA-256:
  `1e6d345db5e97be23026a2ab293b42b55536e119e956501bd8fb2f80e0f5be39`.

Authenticated and anonymous GitHub downloads, the store draft and public store
download all match the frozen digest. General and Emery catalogs show 1.4.0,
correct notes and description, and the updated six-image carousel. The rendered
GitHub page is Latest. The store main page shows 1.4.0 and its correct download;
the separate changelog updated on the second bounded check. No web cache issue
remains pending. Phone My Apps refresh has not been observed.

## Changes

The main menu wraps in both directions, and native touch scrolling is enabled.
Tap Up/Down for the SDK's short scrolling step. Hold for 500 ms to page by 80%
of the viewport, repeating every 325 ms until release. Held paging continues
through parts of a reading and stops at whole-reading boundaries.

A fresh Down tap at the end opens the next reading directly; Up at the start
returns to the preceding reading's last page. Intermediate/completion cards are
removed. The Gospel ends on its final text. Back returns to the menu. The
reading/calendar resource pack, App Glance and saved-preference behavior are
unchanged; no saved reading position is introduced.

The store description now describes swipes and tap/hold controls. The obsolete
transition-card screenshot is replaced by the Psalm; Settings shows v1.4.0.
All six native 200×228 screenshots were captured from the release build; served
pixels/order match. Banner, icons, quotation, content scope, attribution,
unrelated listing fields and every older release were preserved.

## Verification and limitations

A clean SDK 4.33.1 build and all four ASan/UBSan host suites pass: 13,168 reading
segments, 1,006 complete pages, 6,575 calendar dates, 2,523 complete reading parts,
Glance and planner regressions. The final Emery build passes direct next/previous
reading navigation, held-stop boundaries, bounded Gospel end and all three
Passion parts. Native carousel captures pass, and emulator time was restored.
Earlier tap timing, menu wrapping, theme/font and cancellation checks remain in
`artifacts/touch-scroll/` and `artifacts/direct-readings/`.

The exact frozen 1.4.0 PBW installed and launched successfully on physical PT2
firmware 4.38.4, verified by app UUID, fault-free startup log and screenshot.
The owner confirmed real touch scrolling and the direct reading flow before the
version bump. Physical clock changes were not used. Dedicated overnight,
disconnected-use and battery soak remain unmeasured.

Resources: 937,965 bytes (unchanged); static RAM: 58,592 bytes; linker free heap:
72,480 bytes, 530 more than public 1.3.1. Normal emulator exit retains zero app
allocations. The legacy resource warning remains; the store accepted the PBW.

Evidence: `artifacts/release-1.4.0/`; frozen bytes, provenance, hashes and state:
`.release/1.4.0/candidate-1/manifest.json`. Never rebuild or overwrite this version.
The source and tag are pushed; the unrelated `release/reddit/` pack remains
untracked and excluded from the release commits.

---

# Public release — 1.3.1

Daily Roman Missal 1.3.1 is published on both services, as authorized
by the owner on October 8, 2026.

- [GitHub Release](https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.3.1) — Latest, release ID `406651126`.
- [Pebble App Store](https://apps.repebble.com/roman-missal_9e466762014d4b1ab248219f) — listed, release ID `c7b3f04025f445109a33fba0`.
- Source commit: `dcb6745a386460c6f2a09679849305676b924721`; annotated tag `v1.3.1`.
- PBW: 962,242 bytes; SHA-256:
  `ad66f1a57e2e26b7eeba89f3c6fe6ac6849eb94cc702c18e87edd2a238b8c58b`.

Authenticated and anonymous GitHub downloads and the public store download match
the frozen digest. Both public catalogs show 1.3.1 with the correct notes, six
native screenshots and download URL. Served screenshot pixels match exactly.
Banner, icons, description, identity, unrelated metadata and all older releases
were preserved. The rendered GitHub release is verified.

The canonical store web page still shows cached 1.3.0 version information and its
old PBW link after three bounded checks, including a hard reload. Its cache refresh
is pending; no extra upload or republish was attempted. Use the GitHub release for
a verified direct download while that surface catches up.

## Changes

Readings now use the full 228-pixel screen, reclaiming the former 22-pixel black
footer. The crimson ribbon still shows progress, and Select still opens Settings.
A fine crimson rule and small gap separate the readings from Day details and
Settings. A quiet `v1.3.1` label in Settings comes from the SDK build metadata.

The menu divider uses ordinary row drawing. An earlier local native separator
callback caused a startup fault on PT2 firmware 4.38.4; that development fault
was corrected and verified before this release. Reading/calendar resources and
App Glance are unchanged. Saved reading position remains deliberately absent.

## Verification and limitations

Clean SDK 4.33.1 build and all four ASan/UBSan host suites pass. The final build
was installed in an isolated Emery emulator; native carousel captures, forward
and backward reading transitions and real-clock restoration passed. Earlier
full-height, font/theme, divider, empty-date, evening and multi-part checks remain
in `artifacts/settings-version/` and `artifacts/launch-fix/`.

The exact frozen 1.3.1 PBW installed successfully on physical PT2 firmware 4.38.4.
Its launch was verified through the running-app UUID, startup logs without faults
and a screenshot of today's Missal menu. The owner had already confirmed the
same corrected UI before the version bump. No physical watch clock was changed.

Resources: 937,965 bytes; static RAM: 59,122 bytes; heap before allocations: 71,950.
Compared with 1.3.0, resources are unchanged, static RAM rises 68 bytes and the
bundle grows 117 bytes. Emulator normal exit retains zero app allocations.
The legacy SDK resource warning is unchanged; the store accepted these exact bytes.
Dedicated overnight, disconnected-use and battery soak remain unmeasured.
Phone My Apps cache refresh is not claimed.

Evidence: `artifacts/release-1.3.1/`; frozen package, checksums, provenance and
publication state: `.release/1.3.1/candidate-1/manifest.json`. Do not recreate or
overwrite these releases. Further app changes require a new candidate/version.

---

# Public release — 1.3.0

Daily Roman Missal 1.3.0 is published and verified on both services, as authorized
by the owner on October 8, 2026.

- [GitHub Release](https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.3.0) — Latest, release ID `406602907`.
- [Pebble App Store](https://apps.repebble.com/9e466762014d4b1ab248219f) — listed, release ID `be8a71e0aa7248a9ae6607aa`.
- Source commit: `270f8fa69e209b6a6ad4ea578ac09be29f571d6b`; annotated tag `v1.3.0`.
- PBW: 962,125 bytes; SHA-256:
  `60770c4a413dd5a6a5683b3784f99a3ecbaee20f49e2742140449f1de20e1d90`.

Authenticated and anonymous GitHub downloads and the public store download match
this frozen digest. General and Emery catalogs show 1.3.0, matching release notes,
description, banner and screenshot order. The rendered public page shows the new
compatibility imprint, six screenshots, version and correct PBW link. Icons,
identity, unrelated fields and every older release were preserved.

## Features and artwork

Version 1.3.0 adds continuous reading with transition cards, shared on-watch
Settings, larger menus and prompts, Church mode, and the 4-pixel crimson ribbon.
Reading and calendar resources are unchanged. Text size and theme persist;
reading position and browsed date do not. Each launch opens today's menu.

The banner now includes “FOR PEBBLE TIME 2” beneath the main title. It was edited
with the built-in image_gen tool, exported at 720×320 and inspected after store
conversion. Master, exact prompt and export are in `release/store/`. The six
native 200×228 screenshots show All Saints, Gospel/ribbon, Church mode, the
next-reading card, Settings and date shortcuts. Served screenshot pixels match
exactly. The description retains the St. Josemaría quotation and scope/attribution.

## Validation and limitations

A clean SDK 4.33.1 build and all four ASan/UBSan host suites pass. Emery installation,
new carousel captures and reading transitions pass, together with the extensive
Settings, multi-part, font, ribbon geometry and cancellation evidence recorded
in `artifacts/continuous-reader/` and `artifacts/ribbon/`.
Resources: 937,965 bytes; footprint: 59,054 bytes; heap before allocations: 72,018.

Normal exit retains zero app allocations. Forced exit during native window
opening retains 148 bytes in both builds with and without the ribbon; ribbon
cancellation has no invalid animation handles. The legacy SDK resource warning
is unchanged, and this exact bundle was accepted and published by the store.

The attempt to install the frozen 1.3.0 candidate through the phone developer
connection timed out after 100 seconds. Physical installation is unconfirmed;
no successful physical update or phone My Apps refresh is claimed. Dedicated
physical overnight, disconnected-use and battery soak remain unmeasured.

Evidence: `artifacts/release-1.3.0/`. Frozen app, assets, notes, source provenance,
checksums and destination results: `.release/1.3.0/candidate-1/manifest.json`.
Do not rebuild to upload again or overwrite the existing tag/release.

---

# Public release — 1.2.0

Daily Roman Missal 1.2.0 is published on GitHub Releases and the Pebble
App Store, as explicitly authorized by the owner on October 8, 2026.
App identity remains **Roman Missal**, UUID
`bfbd18b3-4a25-44b1-99f9-2de4fbe0f076`, for Pebble Time 2 (Emery).

The Pebble listing is public, with version 1.2.0, the new description, icons,
banner and six native screenshots. Versions 1.0.0 and 1.0.1 remain unchanged
and unpublished. Public downloads are live and their SHA-256 digests match.

- [Public source repository](https://github.com/ewijaya/pebble-daily-mass)
- [GitHub release](https://github.com/ewijaya/pebble-daily-mass/releases/tag/v1.2.0), release ID `406529715`
- [Public Pebble listing](https://apps.repebble.com/9e466762014d4b1ab248219f)
- [Pebble dashboard](https://developer.repebble.com/dashboard/apps/9e466762014d4b1ab248219f/edit), release ID `e381ff18df3c4a9b8ad10319`
- [Listing and carousel preview](../release/store/preview.html)

Frozen source: `6d49fcaab2ea9a71835c6ac5197f9a2ac519b422`; annotated tag `v1.2.0`.
PBW: 958,150 bytes; SHA-256:
`66b60438b7b0f6d3ee9a46c20d93c75fb2df322a0dfdac7e8c20de3c83ce390f`.
Authenticated and anonymous GitHub downloads and the public Pebble download
match this digest. GitHub marks 1.2.0 as Latest. Do not rebuild or upload duplicates.

## Candidate and checks

Version 1.2.0 adds automatic date selection, the General Roman Calendar for
2020–2037, offline App Glance, seasonal colors with red rubrics, Extra Large by
default, date shortcuts and optional evening preparation. The owner removed
saved-place resume; each launch opens today's menu.

All four host suites pass under ASan/UBSan: 13,168 text segments, 1,006 source
pages, 6,575 calendar dates, 2,523 complete reading parts, daily glance summaries,
DST boundaries, calendar precedence and date planning. The clean final SDK build
uses 937,965 resource bytes, a 63,138-byte static footprint and 67,934 bytes of
heap before runtime allocations. The legacy SDK resource warning is unchanged;
this resource size fits the verified PT2 firmware limit.

A separate Emery storage image was used after the old emulator failed to
initialize transfer storage. The old emulator data was retained. The final app
installs in the isolated emulator; first menu draw is about 50 ms and observed
exits retain no app heap allocations. Reader font persistence and reopening to
Today passed. Native screenshot and navigation evidence is in
`artifacts/release-1.2.0/`. Frozen identity and destination status are recorded in
`.release/1.2.0/candidate-1/manifest.json`.

## Listing and artwork

The description in [listing.md](../release/store/listing.md) recalls a familiar
printed missal while accurately describing selected daily Mass readings. The
Order of Mass and all its prayers, national calendars, optional choices and
separate vigil forms are outside this edition. Source attribution is retained.

After publication, the description was updated at the owner’s request to include
St. Josemaría Escrivá’s quotation on loving the Mass and making it the centre
of our day, attributed to [Christ Is Passing By, no. 154](https://escriva.org/en/es-cristo-que-pasa/154/).
The main and Emery catalog descriptions are verified; app bytes and artwork
are unchanged. Evidence: `artifacts/release-1.2.0/escriva-description/`.

The approved artwork uses a solid engraved-style Chi-Rho, crimson leather,
a plain gold border and a polished upright silver PT2. Banner: 720×320;
icons: 80×80 and 144×144. The carousel uses unmodified 200×228 app captures,
with the All Saints main menu first. Order is in `release/store/carousel.json`.

Dashboard read-back verifies the main and Emery descriptions, screenshot order,
published 1.2.0 and listed visibility. Unrelated fields and older releases are
unchanged. All six served screenshots have identical pixels. The store converts
artwork to indexed PNGs (mean RGB difference about 1/255); dimensions and visual
appearance were checked. Anonymous general and Emery catalogs return 200 with
version 1.2.0, matching description, release notes and ordered screenshots. The
public PBW digest matches. The rendered public page shows the banner, six
screenshots, description, version and download link. Phone My Apps refresh has
not been observed. Publication evidence is in `artifacts/release-1.2.0/`.

After publication the owner requested a sharper banner. The replacement keeps
the approved crimson-and-gold design, enlarges the watch face, and uses cleaner
gold lettering. It was generated with the built-in image_gen tool, checked at
the required 720×320 size, uploaded, and visually checked after store conversion.
Both public catalogs serve the new banner. The description, icons, six screenshots
and release bytes remain unchanged. Evidence: `artifacts/banner-sharpness/`.

## Remaining physical evidence

The exact frozen final PBW installed successfully on PT2 firmware 4.38.4.
`artifacts/release-1.2.0/physical-install.log` records success and
`physical-frozen.png` shows the watch's Today menu.

A dedicated disconnected-use, midnight and battery soak has not been established.
The owner reports substantial gaming last night and active watch development
today; that mixed use is not a controlled battery result. Do not claim a battery
benchmark or a physical overnight test from emulator checks.

For publication, use [RELEASING.md](RELEASING.md) and the project-local release
skills. Publish only the frozen candidate bytes and verify each destination.
Do not use top-level `pebble publish`, which rebuilds and hardcodes public flags.

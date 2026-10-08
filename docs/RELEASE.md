# Release preparation — 1.2.0

Daily Roman Missal 1.2.0 is being prepared for GitHub Releases and the Pebble
App Store. The owner requested that the existing GitHub repository become public.
App identity remains **Roman Missal**, UUID
`bfbd18b3-4a25-44b1-99f9-2de4fbe0f076`, for Pebble Time 2 (Emery).

The existing Pebble listing is hidden; versions 1.0.0 and 1.0.1 are unpublished
older candidates. Preparation will add 1.2.0 as a separate unpublished draft and
refresh its description, icons, banner and native screenshot carousel.
No older candidate should be promoted.

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
`.release/1.2.0/candidate-1/manifest.json` once preparation is complete.

## Listing and artwork

The description in [listing.md](../release/store/listing.md) recalls a familiar
printed missal while accurately describing selected daily Mass readings. The
Order of Mass and all its prayers, national calendars, optional choices and
separate vigil forms are outside this edition. Source attribution is retained.

The approved artwork uses a solid engraved-style Chi-Rho, crimson leather,
a plain gold border and a polished upright silver PT2. Banner: 720×320;
icons: 80×80 and 144×144. The carousel uses unmodified 200×228 app captures,
with the All Saints main menu first. Order is in `release/store/carousel.json`.

## Remaining physical evidence

The preceding 1.2.0 removal build was installed and its Today/relaunch behavior
verified on PT2 firmware 4.38.4. The frozen final build must be installed and
verified separately if its digest differs. Record that outcome in the manifest.

A dedicated disconnected-use, midnight and battery soak has not been established.
The owner reports substantial gaming last night and active watch development
today; that mixed use is not a controlled battery result. Do not claim a battery
benchmark or a physical overnight test from emulator checks.

For publication, use [RELEASING.md](RELEASING.md) and the project-local release
skills. Publish only the frozen candidate bytes and verify each destination.
Do not use top-level `pebble publish`, which rebuilds and hardcodes public flags.

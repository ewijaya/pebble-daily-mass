# Release preparation — 1.2.0

Daily Roman Missal 1.2.0 is prepared as a draft on GitHub Releases and the Pebble
App Store. The existing GitHub repository is now public, as the owner requested.
App identity remains **Roman Missal**, UUID
`bfbd18b3-4a25-44b1-99f9-2de4fbe0f076`, for Pebble Time 2 (Emery).

The Pebble listing remains hidden. Version 1.2.0 is a separate unpublished draft
with the new description, icons, banner and six native screenshots. Versions 1.0.0
and 1.0.1 remain unchanged and unpublished. Public app downloads are not live yet.

- [Public source repository](https://github.com/ewijaya/pebble-daily-mass)
- [GitHub draft](https://github.com/ewijaya/pebble-daily-mass/releases/tag/untagged-0d7a5662a1b24f992d11), release ID `406529715`
- [Pebble dashboard](https://developer.repebble.com/dashboard/apps/9e466762014d4b1ab248219f/edit), release ID `e381ff18df3c4a9b8ad10319`
- [Listing and carousel preview](../release/store/preview.html)

Frozen source: `6d49fcaab2ea9a71835c6ac5197f9a2ac519b422`; intended tag `v1.2.0`.
PBW: 958,150 bytes; SHA-256:
`66b60438b7b0f6d3ee9a46c20d93c75fb2df322a0dfdac7e8c20de3c83ce390f`.
The GitHub draft download matches this digest. Do not rebuild or upload duplicates.

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

The approved artwork uses a solid engraved-style Chi-Rho, crimson leather,
a plain gold border and a polished upright silver PT2. Banner: 720×320;
icons: 80×80 and 144×144. The carousel uses unmodified 200×228 app captures,
with the All Saints main menu first. Order is in `release/store/carousel.json`.

Dashboard read-back verifies the main and Emery descriptions, screenshot order,
hidden state and unpublished releases. Unrelated fields and older releases are
unchanged. All six served screenshots have identical pixels. The store converts
artwork to indexed PNGs (mean RGB difference about 1/255); dimensions and visual
appearance were checked. Anonymous general and Emery catalogs return 404, as
expected for a hidden draft. The draft PBW URL also returns 404; its public
download digest must be checked after publication. Upload identity and size match.

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

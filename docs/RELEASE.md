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

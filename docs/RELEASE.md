# Public release preparation

The current build is a development preview. The reader, visual design, fonts,
navigation, offline storage and PT2 installation work. The user approved the
reading-text distribution and is satisfied with launch speed as of October 8,
2026. Folder cleanup does not change release readiness.

Before publishing:

- Implement automatic local-date selection and liturgical calendar resolution,
  including cycles, seasons, celebrations, precedence, transfers and linked
  propers/commons. Select and document the supported calendar scope.
- Generalize reading boundaries and alternatives beyond the four manually
  audited October 8 sections. Preserve citations, rubrics and full text.
- Handle long readings beyond the current 8 KiB buffer and 64 paragraph limit.
- Resolve the installed SDK's 262,144-byte App Store resource allowance warning.
  The current 803,312-byte pack installs on PT2; that does not establish store
  acceptance. Confirm the submission requirements when preparing the release.
- Validate representative dates and celebrations, long readings, offline use,
  repeated launches, date changes, font persistence and battery behavior on PT2.
- Prepare the store description, screenshots, version, supported device list,
  source attribution and relevant licenses. Keep claims aligned with tested
  behavior. The UUID must remain stable for updates.

No public release or repository visibility change has been made.

# Release audit — October 8, 2026

Status: private candidate. The missing source passage has been supplied and
incorporated; physical-device soak testing remains before public release. Text distribution was already
approved by the owner; this audit does not reopen that decision.

## Calendar and content

Compared first/second reading and Gospel citations across 2026 against the
[USCCB 2026 calendar](https://www.usccb.org/resources/2026cal.pdf), separating
General Roman conventions from US national observances. After corrections,
327 dates matched the comparison script's normalized first-choice citations.
The remaining 28 flags concern national differences, permitted alternatives,
weekday feast reading counts, procession/vigil scope, or citation notation.
Ten dates needed manual inspection because the PDF labels morning/day/vigil
forms or uses a citation the exploratory parser did not recognize.

This is a citation/selection audit, not a word-by-word certification of all
Scripture, psalms or acclamations. The local comparison and downloaded reference
are retained under ignored `artifacts/release-prep/`; the PDF is not redistributed.

Corrected the Immaculate Heart collision with another obligatory memorial:
both become optional, and weekday readings are the default. Added weekday
Holy Family handling. Corrected the Year A Gospel choices for Easter Week 4
Monday and Ordinary Time Week 18 Monday/Tuesday. The latter Tuesday uses the
[permitted alternative](https://bible.usccb.org/bible/readings/080426.cfm).

Audited EPUB corrections applied to calendar metadata and reference selection:

| EPUB part | Correction |
| --- | --- |
| 0365 | John 20:24–29, missing digit in heading |
| 0535 | Luke 6:12–16 for Simon/Jude; stop before verses 17–19 |
| 1002 | Restore Esther's chapter C |
| 1035 | Body is John 8:1–11, not the printed 8:12–20 |
| 1181 | 1 Kings 2:1–4,10–12; omit unrelated summary |
| 1249 | Jude 17,20b–25; single-chapter reference syntax |
| 1375 | Jeremiah 3:14–17; omit unrelated summary |
| 1555 | Matthew 22:1–14 (short form 1–10), not Mark 4 |

Earlier fixes for parts 1220, 1241, 1051 and 0958 remain in place. Scripture
wording is unchanged. A source search found no complete John 8:12–20 passage.
The [Year C reference](https://bible.usccb.org/bible/readings/031416.cfm)
confirms the required Gospel and the condition for its alternative. The owner
pasted the complete passage directly on October 8. It is retained in local
`data/source/john-8-12-20.json`, appended as five resource segments and selected
on all six affected Mondays. Words and punctuation are preserved; line wraps
are normalized into paragraphs. The supplement's CCD credit is bundled.

## Technical checks

- ASan/UBSan source checks: all 13,168 segments and 1,006 pages pass, including
  capacity, corruption and short-read handling.
- ASan/UBSan calendar checks: all 6,575 dates and 2,523 complete reading parts pass;
  invalid dates, buffer limits and resource bounds are covered.
- Twenty emulator stop/start cycles from an open reader: all successful;
  free reader heap 36,760 bytes, zero retained allocations reported at exit.
- XL preference survived restarts. Large/XL, feast and memorial menus, corrected
  Sunday citation were visually inspected. The 20-restart run predates the
  small supplemental-text addition; the app binary behavior is unchanged.
- Final 1.0.1 John 8:12–20 start/end screenshots and top/bottom jumps pass.
- Final 1.0.1 installed on PT2 after reopening the companion app. Its screenshot
  confirms October 8, 2026, Ordinary Time Week 27 and today's reading menu.
- Earlier checks cover midnight refresh, keeping an open reading stable across
  midnight, date browsing, long Passion parts and out-of-range dates.
- No companion JavaScript, network requests, analytics or account is used by
  the app. All date selection and text decoding occur on the watch.
- A multi-day physical PT2 battery/offline soak remains outstanding. Short
  emulator tests do not establish battery life or every physical-button path.

## Store preparation

The current developer API reports a 4.4 MiB PBW upload ceiling and 200×228 Emery
screenshots. The SDK's 256 KiB resource warning is a separate legacy check;
the full bundle was accepted as a hidden draft, resolving that upload concern.

The installed pebble-tool 5.0.40 publish command hardcodes `isPublished=true`
and `visible=true`, despite its help advertising an unpublished default.
It was not used. The current dashboard's submission API accepts explicit
`visibility=hidden`; preparation uses that plus `isPublished=false`.
Authenticated detail confirms hidden/unpublished status; anonymous Emery app
lookup returned 404, matching the [API visibility contract](https://appstore-api.repebble.com/).

Listing, privacy text, release notes, original Chi-Rho exports and five native
screenshots are in `release/store/`. The listing uses Roman Missal so the phone
locker preserves the tested compact launcher title. The repository stays private.

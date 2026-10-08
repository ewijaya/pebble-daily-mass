# Extraction audit — 2026-10-08

Source SHA-256:
`a88c1e4917d3a6c81cd6f2eda709aad7ef4486c80d45e027c26f2aa6f6ae78c5`

## Five secondary-scan candidates

Inspected the English-side source content of each page. None contains an omitted
scripture reading body. Keep their links in the calendar inventory for future
resolution; exclusion from the reading-body database does not make the pages
irrelevant to the calendar.

| Source | Finding |
| --- | --- |
| `text/part0106.html` | Good Friday rubrics/intercessions; proper-readings link |
| `text/part0112.html` | Easter Vigil rubrics/prayers; proper-readings link |
| `text/part0865.html` | Funeral readings index with citations/links |
| `text/part0922.html` | Explanation of lectionary cycles |
| `text/part1973.html` | Order of Mass for a single minister, rubrics/dialogue |

The extractor now regenerates `review-candidates.json`, including content hashes,
links and findings. New or changed candidates are marked `needs_review`.
This scan is still heuristic; it cannot certify complete coverage.

## Rosary reading sample

Audited `text/part0519.html` against its XHTML structure. All 15 nonempty
English-column paragraphs match extracted segments exactly. Latin columns,
blank paragraphs and trailing navigation are excluded from the reading views.
The full page, including navigation, remains in the database.

| Reading | Local segment range (inclusive) | Content |
| --- | --- | --- |
| First Reading | 1–4 | Heading/citation, summary, two body paragraphs |
| Psalm | 5–11 | Heading/citation, response and five stanzas |
| Acclamation | 12–13 | Heading/citation, complete acclamation |
| Gospel | 14–21 | Heading/citation, summary, six body paragraphs |

Segment 0 is the celebration heading. Segment 22 is onward navigation. No
alternative readings or intervening rubrics occur on this page. The extractor
checks these boundaries against heading/citation text, paragraph count and all
English-column paragraphs. Its local `sample-audit.json` retains roles and text.

This establishes a sample for testing the reader. It does not assert that its
readings replace weekday readings on every October 7. Semantic classification,
reading alternatives, rubrics, and cross-references on the remaining pages
still require review.


## October 8, 2026 preview

User requested today's readings in the emulator for an accuracy check. Checked
Asia/Tokyo date: October 8, 2026. The [USCCB date page](https://bible.usccb.org/bible/readings/100826.cfm)
lists Thursday of the Twenty-seventh Week in Ordinary Time, lectionary 464.
Selected `text/part1549.html`: Thursday, Ordinary Week 27, Year II. Its citations
match the reference: Galatians 3:1–5; Luke 1:69–70, 71–72, 73–75; Acts 16:14b;
Luke 11:5–13. No national-calendar customization was added.

| Section | Local segment range (inclusive) |
| --- | --- |
| First Reading | 3–5 |
| Responsorial Psalm/canticle | 6–10 |
| Acclamation | 11–12 |
| Gospel | 13–17 |

All nine English-column paragraphs match the extracted segments. Opening
season/day/year headings (0–2) and onward navigation (18–19) are excluded from
individual reading views. Source summaries (4, 14) remain visible. The extractor
now regenerates `october8-audit.json` and checks these boundaries.

The scripture body wording agrees with the USCCB reference apart from
punctuation/formatting. Two differences matter for review:

- The EPUB acclamation says “Open your hearts”; USCCB says “Open our hearts”.
  This apparent EPUB typo is intentionally preserved in the preview.
- The EPUB joins the two middle canticle stanzas with one refrain. USCCB places
  a refrain after each. The canticle verse text remains present.

This is a manually pinned preview, not a calendar-engine test. On later dates,
it will continue to display October 8 until the selection is changed.

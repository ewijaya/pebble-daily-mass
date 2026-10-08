"""Narrow, source-specific audit; not a general reading-boundary classifier."""
import hashlib

REVIEWED = {
    'text/part0106.html': 'Good Friday rubrics and intercessions; links to proper readings.',
    'text/part0112.html': 'Easter Vigil rubrics and prayers; links to proper readings.',
    'text/part0865.html': 'Funeral reading index containing citations and links, not reading bodies.',
    'text/part0922.html': 'Explanation of the lectionary and its cycles.',
    'text/part1973.html': 'Order of Mass rubrics and dialogue, not a scripture reading.',
}


REVIEWED_HASHES = {
    "text/part0106.html": "88fc407b07228626f0789d486ce71c734693d659ba3d74d2dc8e803a7d662fc2",
    "text/part0112.html": "58cc33000ce83c1ed8ce8ce785fd0eaea38014acdbdbb2d650c2d8107c9e53d8",
    "text/part0865.html": "dfdbce329f5fe5a6ed15a97f5235dd3f4ca9b40880a6e412cb564fbfde19ce10",
    "text/part0922.html": "224f8b1761572bf5d9aa4ae72a24ffa87f9f87adff300a01345102caf0c5e395",
    "text/part1973.html": "0355a11bafea1221bfd23088fddbcec2c48b285bd44ab2489dac00bd1280bcda"
}


def review_candidate(source, parts, links):
    digest = hashlib.sha256('\n\n'.join(parts).encode()).hexdigest()
    reviewed = REVIEWED_HASHES.get(source) == digest
    return {'source': source, 'opening': parts[:5], 'links': links,
            'status': 'reviewed' if reviewed else 'needs_review',
            'reason': REVIEWED[source] if reviewed else 'New or changed candidate; inspect source.',
            'normalized_sha256': digest}


def audit_sample(body, parts):
    # These ranges are explicitly reviewed against the supplied EPUB. Fail closed
    # if an updated source changes the shape of the sample used by the C app.
    from extract import segments
    assert len(parts) == 23
    assert parts[0] == 'October 7\nReadings for Our Lady of the Rosary'
    headings = {1: 'First reading\nActs 1: 12–14',
                5: 'Responsorial Psalm\nLuke 1: 46–47, 48–49, 50–51, 52–53, 54–55 (:49)',
                12: 'Gospel Acclamation\nSee Luke 1: 28',
                14: 'Gospel\nLuke 1: 26–38'}
    for index, heading in headings.items():
        assert parts[index] == heading
    right = [p for el in body.iter() if 'right' in el.get('class', '').split()
             for p in segments(el)]
    body_indices = [3, 4, *range(6, 12), 13, *range(16, 22)]
    assert right == [parts[i] for i in body_indices]
    assert parts[22].startswith('Prayer of the Faithful')
    roles = ['english_body' if i in body_indices else 'summary' for i in range(23)]
    roles[0], roles[22] = 'celebration', 'navigation'
    for i in headings:
        roles[i] = 'heading_and_citation'
    return {'source': 'text/part0519.html', 'alternatives': [],
            'readings': [{'label': label, 'first': first, 'count': count}
                         for label, first, count in [('First Reading', 1, 4),
                                                    ('Psalm', 5, 7),
                                                    ('Acclamation', 12, 2),
                                                    ('Gospel', 14, 8)]],
            'segments': [{'index': i, 'role': roles[i], 'text': part}
                         for i, part in enumerate(parts)],
            'excluded': ['Latin div.left columns', 'blank paragraphs',
                         'celebration heading and trailing navigation from reading views'],
            'scope': 'Only this page has audited semantic boundaries; no date-selection claim.'}


def audit_october8(body, parts):
    """Manually selected 2026-10-08 weekday preview, checked against USCCB."""
    from extract import segments
    assert len(parts) == 20
    assert parts[:3] == ['week 27 in ordinary time', 'Readings for Thursday', 'Year II']
    headings = {3: 'First reading\nGalatians 3: 1–5',
                6: 'Responsorial Psalm\nLuke 1: 69–70, 71–72, 73–75 (:see 68)',
                11: 'Gospel Acclamation\nSee Acts 16: 14b',
                13: 'Gospel\nLuke 11: 5–13'}
    for index, heading in headings.items():
        assert parts[index] == heading
    body_indices = [5, 7, 8, 9, 10, 12, 15, 16, 17]
    right = [p for el in body.iter() if 'right' in el.get('class', '').split()
             for p in segments(el)]
    assert right == [parts[i] for i in body_indices]
    assert parts[18].startswith('Prayer of the Faithful')
    assert parts[19].startswith('General Intercessions')
    assert 'Open your hearts' in parts[12]  # Documented source discrepancy; preserved.
    return {'source': 'text/part1549.html', 'preview_date': '2026-10-08',
            'reference': 'https://bible.usccb.org/bible/readings/100826.cfm',
            'readings': [{'label': label, 'first': first, 'count': count}
                         for label, first, count in [('First Reading', 3, 3),
                                                    ('Psalm', 6, 5),
                                                    ('Acclamation', 11, 2),
                                                    ('Gospel', 13, 5)]],
            'segments': [{'index': i, 'text': part} for i, part in enumerate(parts)],
            'differences': ['EPUB acclamation says "your hearts"; USCCB says "our hearts".',
                            'EPUB groups two middle canticle stanzas with one response; USCCB separates them.',
                            'EPUB includes reading summaries and differs in punctuation/line breaks.'],
            'scope': 'Manually verified weekday preview; no automatic calendar or local-calendar claim.'}

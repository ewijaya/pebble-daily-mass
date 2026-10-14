"""Explicit, source-checked selections for the bundled special Mass forms.

Indices refer to English-side EPUB nodes, not Scripture verse numbers. Fail
closed if a source revision changes a boundary. Normal daily parsing is separate.
"""
import re

SPECIAL_HEADING = re.compile(
    r'^(?:(?:First|Second|Third|Fourth|Fifth|Sixth|Seventh) reading|'
    r'Responsorial Psalm|Epistle|Gospel Acclamation|Gospel)(?:\n|$)', re.I)
KINDS = {'First reading': 0, 'Responsorial Psalm': 1, 'Second reading': 2,
         'Gospel Acclamation': 4, 'Gospel': 5, 'Third reading': 6,
         'Fourth reading': 7, 'Fifth reading': 8, 'Sixth reading': 9,
         'Seventh reading': 10, 'Epistle': 11}


def section(cat, page, start, heading, citation=None):
    nodes = cat.nodes[page]
    assert nodes[start]['text'].splitlines()[0] == heading, (page, start, heading)
    gospel = heading.startswith('Year ')
    kind = 5 if gospel else KINDS[heading]
    refs = []
    found_citation = ''
    for i, n in enumerate(nodes[start:], start):
        t = n['text']
        if i > start and not n['right']:
            if (SPECIAL_HEADING.match(t) or t == 'Or:' or
                    t.startswith(('The Creed', 'Prayer after the reading',
                                  'Responsorial Psalm [', 'Year '))):
                # A cross-link to another Gospel cycle precedes this cycle's text.
                if gospel and t.startswith(('Year B [', 'Year C [')):
                    continue
                break
        if 'centertextred' in n['class']:
            for line in t.splitlines():
                if re.match(r'^(?:[1-3] )?[A-Z][A-Za-z ]*\s\d', line):
                    found_citation = line
                    break
            continue
        if i == start or 'rubrics' in n['class']:
            continue
        if n['right'] or 'plaincenter' in n['class'] or 'calibre4' in n['class']:
            assert '➔' not in t, (page, start, t)
            refs.append(n['sid'] | (0x8000 if not n['right'] and 'plaincenter' in n['class'] else 0))
    assert refs and (found_citation or kind == 4), (page, start)
    if citation is not None:
        assert found_citation == citation, (page, start, found_citation, citation)
    return dict(page=page, start=start, kind=kind, citation=found_citation,
                refs=refs, year=heading[-1] if gospel else None)


def variants(cat, day):
    """Additional forms attached to the observed celebration, not fixed dates."""
    identity = day['id']
    cycle = day['sundayCycle'][-1]
    forms = []

    def add(label, note, readings, color):
        forms.append(dict(label=label, note=note, readings=readings, color=color))

    if identity == 'nativity_of_the_lord':
        for label, page, note in [
            ('Vigil Mass', 963, 'Christmas Vigil: evening of December 24.'),
            ('Night Mass', 964, 'Christmas Mass during the Night.'),
            ('Dawn Mass', 965, 'Christmas Mass at Dawn.'),
        ]:
            add(label, note+' Long reading forms.', cat.mass(page, cycle), 'WHITE')
    short = {'nativity_of_john_the_baptist': (336, 'WHITE'),
             'peter_and_paul_apostles': (341, 'RED'),
             'assumption_of_the_blessed_virgin_mary': (432, 'WHITE')}
    if identity in short:
        page, color = short[identity]
        add('Vigil Mass', 'Vigil readings for the preceding evening.', cat.mass(page, cycle), color)
    if identity in ('holy_saturday', 'easter_sunday'):
        for baptism in (True, False):
            starts = [2, 22, 35, 63, 68, 77, 83, 89, 94, 101, 106, 112,
                      118, 125 if baptism else 132, 143, 148,
                      {'A': 153, 'B': 162, 'C': 170}[cycle]]
            headings = ['First reading', 'Responsorial Psalm', 'Second reading',
                        'Responsorial Psalm', 'Third reading', 'Responsorial Psalm',
                        'Fourth reading', 'Responsorial Psalm', 'Fifth reading',
                        'Responsorial Psalm', 'Sixth reading', 'Responsorial Psalm',
                        'Seventh reading', 'Responsorial Psalm', 'Epistle',
                        'Responsorial Psalm', 'Year '+cycle]
            readings = [section(cat, 1054, i, h) for i, h in zip(starts, headings)]
            add('Easter Vigil' if baptism else 'Vigil: no baptism',
                'After nightfall on Holy Saturday. All seven Old Testament readings; '
                'long forms and first psalm options. Psalm after reading 7: '+
                ('with baptism.' if baptism else 'without baptism.'), readings, 'WHITE')
    if identity == 'pentecost_sunday':
        tail = [section(cat, 1120, i, h) for i, h in
                [(42, 'Responsorial Psalm'), (48, 'Second reading'),
                 (52, 'Gospel Acclamation'), (54, 'Gospel')]]
        for start, name in [(6, 'Genesis'), (14, 'Exodus'), (24, 'Ezekiel'), (36, 'Joel')]:
            add('Vigil: '+name, 'Pentecost Vigil, simple form. First reading: '+name+'.',
                [section(cat, 1120, start, 'First reading')]+tail, 'RED')
        extended = [(4, 'First reading'), (10, 'Responsorial Psalm'),
                    (16, 'Second reading'), (24, 'Responsorial Psalm'),
                    (37, 'Third reading'), (47, 'Responsorial Psalm'),
                    (53, 'Fourth reading'), (59, 'Responsorial Psalm'),
                    (65, 'Fifth reading'), (69, 'Gospel Acclamation'), (71, 'Gospel')]
        add('Vigil: extended', 'Pentecost Vigil, extended form. All four Old Testament '
            'readings; first psalm options. Prayers between readings are not included.',
            [section(cat, 1119, i, h) for i, h in extended], 'RED')
    return forms

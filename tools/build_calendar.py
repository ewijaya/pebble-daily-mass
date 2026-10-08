#!/usr/bin/env python3
"""Join the pinned General Roman calendar to the local EPUB's reading catalog.

Build-time selection only; CAL1 on the watch is a bounded, offline date index.
The first allowed (normally long) reading form is used, with audited exceptions.
"""
import datetime as dt
import json
import pathlib
import re
import struct
import sys
import xml.etree.ElementTree as ET
import zipfile
import zlib
from pmr import HEADER, CHUNK_SIZE, unpack
from extract import NS, text
from supplements import john_8

ROOT = pathlib.Path(__file__).resolve().parents[1]
KINDS = ['First Reading', 'Psalm', 'Second Reading', 'Sequence', 'Acclamation', 'Gospel']
SEASONS = ['ORDINARY_TIME', 'ADVENT', 'CHRISTMAS_TIME', 'LENT', 'PASCHAL_TRIDUUM', 'EASTER_TIME']
RANKS = ['WEEKDAY', 'SUNDAY', 'MEMORIAL', 'OPTIONAL_MEMORIAL', 'FEAST', 'SOLEMNITY', 'COMMEMORATION']
# Stored enum order matches CalColor. Use romcal's first permitted color:
# rose on Gaudete/Laetare; purple on All Souls (black is an alternative).
COLORS = ['GREEN', 'PURPLE', 'WHITE', 'RED', 'ROSE', 'NONE']
ORDINALS = {name: i for i, name in enumerate(['', 'first', 'second', 'third', 'fourth', 'fifth', 'sixth', 'seventh'])}
DOW = ['sunday', 'monday', 'tuesday', 'wednesday', 'thursday', 'friday', 'saturday']
HEADING = re.compile(r'^(?:Alternative )?(First reading|Second reading|Responsorial Psalm|Gospel Acclamation|Gospel|Sequence)(?:\n|$)', re.I)
KIND = {'first reading': 0, 'responsorial psalm': 1, 'second reading': 2, 'sequence': 3, 'gospel acclamation': 4, 'gospel': 5}
CITATION = re.compile(r'^(?:See |Cf\. )?(?:[1-3] )?[A-Z][A-Za-z]*(?: [A-Za-z]+){0,3}:?\s*\d')
# Explicit EPUB targets, chosen by identity, never fuzzy saint-name matching.
FIXED = {
 'mary_mother_of_god': 973, 'epiphany_of_the_lord': 975,
 'second_sunday_after_christmas': 974, 'nativity_of_the_lord': 966,
 'ash_wednesday': 992, 'thursday_after_ash_wednesday': 993,
 'friday_after_ash_wednesday': 994, 'saturday_after_ash_wednesday': 995,
 'holy_monday': 1045, 'holy_tuesday': 1046, 'holy_wednesday': 1047,
 'holy_thursday': 1050, 'friday_of_the_passion_of_the_lord': 1051,
 'easter_sunday': 1055, 'joseph_spouse_of_mary': 1715,
 'annunciation_of_the_lord': 1717, 'conversion_of_saint_paul_the_apostle': 1688,
 'presentation_of_the_lord': 1693, 'chair_of_saint_peter_the_apostle': 1706,
 'mark_evangelist': 1728, 'philip_and_james_apostles': 1735, 'matthias_apostle': 1741,
 'visitation_of_mary': 1754, 'nativity_of_john_the_baptist': 337,
 'peter_and_paul_apostles': 342, 'thomas_apostle': 365, 'mary_magdalene': 377,
 'james_apostle': 380, 'transfiguration_of_the_lord': 422, 'lawrence_of_rome_deacon': 427,
 'assumption_of_the_blessed_virgin_mary': 433, 'bartholomew_apostle': 442,
 'nativity_of_the_blessed_virgin_mary': 472, 'exaltation_of_the_holy_cross': 476,
 'matthew_apostle': 483, 'michael_gabriel_and_raphael_archangels': 489,
 'luke_evangelist': 529, 'simon_and_jude_apostles': 535, 'all_saints': 562,
 'dedication_of_the_lateran_basilica': 565, 'andrew_apostle': 580,
 'immaculate_conception_of_the_blessed_virgin_mary': 604,
 'stephen_the_first_martyr': 613, 'john_apostle': 614, 'holy_innocents_martyrs': 615,
}
CYCLIC = {'holy_family_of_jesus_mary_and_joseph': 967, 'baptism_of_the_lord': 988,
 'divine_mercy_sunday': 1062, 'ascension_of_the_lord': 1104, 'pentecost_sunday': 1121,
 'most_holy_trinity': 1664, 'most_holy_body_and_blood_of_christ': 1667,
 'most_sacred_heart_of_jesus': 1670, 'our_lord_jesus_christ_king_of_the_universe': 1673}
# These memorials have strictly proper texts. Use the supplied proper set;
# other mandatory memorials retain the weekday readings (GIRM 357–358).
PROPER_MEMORIALS = {
 'timothy_of_ephesus_and_titus_of_crete_bishops': 1689,
 'barnabas_apostle': 330, 'holy_guardian_angels': 515,
 'martha_of_bethany_mary_of_bethany_and_lazarus_of_bethany': 382,
 'passion_of_saint_john_the_baptist': 447, 'our_lady_of_sorrows': 477,
 'immaculate_heart_of_mary': 1759, 'mary_mother_of_the_church': 1755,
}


def tokens(node, right=False):
    cls = node.get('class', '').split()
    if 'left' in cls:
        return
    right = right or 'right' in cls
    tag = node.tag.split('}')[-1]
    if tag in ('p', 'h1', 'h2', 'h3', 'li', 'span'):
        value = text(node)
        if value:
            yield {'text': value, 'class': cls, 'right': right}
        return
    for child in node:
        yield from tokens(child, right)


class Catalog:
    def __init__(self, epub, extracted):
        self.segments = extracted['segments']
        self.pages = {int(re.search(r'part(\d+)', p['source'])[1]): p for p in extracted['pages']}
        self.nodes = {}
        self.temporal = {}
        self.cache = {}
        with zipfile.ZipFile(epub) as z:
            for number, page in self.pages.items():
                body = ET.fromstring(z.read(page['source'])).find('h:body', NS)
                nodes = list(tokens(body))
                assert [n['text'] for n in nodes] == [self.segments[i] for i in page['segments']], number
                for n, sid in zip(nodes, page['segments']):
                    n['sid'] = sid
                self.nodes[number] = nodes
                opening = [n['text'].lower() for n in nodes[:3]]
                m = re.fullmatch(r'week (\d+) in ordinary time', opening[0])
                seasonal = re.fullmatch(r'(first|second|third|fourth|fifth|sixth|seventh) week of (advent|lent|easter)', opening[0])
                if m or seasonal:
                    season = 'ordinary_time' if m else seasonal[2]
                    week = int(m[1]) if m else ORDINALS[seasonal[1]]
                    day = next((d for d in DOW if re.search(r'\b'+d+r'\b', opening[1])), None)
                    if day:
                        cycle = opening[2].replace('year ', '') if opening[2].startswith('year ') else ''
                        key = (season, week, day, cycle)
                        assert key not in self.temporal, key
                        self.temporal[key] = number

    def sections(self, page):
        if page in self.cache:
            return self.cache[page]
        nodes = self.nodes[page]
        starts = [(i, HEADING.match(n['text'])) for i, n in enumerate(nodes)
                  if 'centertextred' in n['class'] and HEADING.match(n['text'])]
        found = []
        for k, (start, match) in enumerate(starts):
            end = starts[k+1][0] if k+1 < len(starts) else len(nodes)
            kind = KIND[match[1].lower()]
            block = nodes[start:end]
            citation = ''
            refs = []
            year = None
            for j, n in enumerate(block):
                t = n['text']
                if not n['right']:
                    if j and (t.startswith(('Prayer of the Faithful', 'The Creed', 'Index of Readings', 'General Intercessions', 'When this Feast')) or t == 'Or:' or t.startswith('Short form')):
                        break
                    if 'rubrics' in n['class']:
                        continue
                    if t.startswith('Verse before the Gospel'):
                        break
                    if re.match(r'^Year [ABC](?:\n|$)', t):
                        year = t[5]
                        continue
                    if t.startswith(('Year B [', 'Year C [', 'Other options')):
                        continue
                    for line in t.splitlines():
                        if CITATION.match(line):
                            if not citation:
                                citation = line
                            break
                    if j == 0 or 'centertextred' in n['class']:
                        continue
                if j and (n['right'] or 'calibre4' in n['class'] or 'plaincenter' in n['class']):
                    # Rubric summaries are structurally outside the English body column.
                    rubric = not n['right'] and 'plaincenter' in n['class']
                    if t == 'Here all kneel and pause for a short time.':
                        rubric = True
                    refs.append(n['sid'] | (0x8000 if rubric else 0))
            # Two EPUB headings contain only "Scripture". References verified
            # against USCCB lectionary entries; the body text is unchanged.
            if (page, kind) in {(1220, 2), (1241, 0)}:
                assert block[0]['text'].endswith('\nScripture')
                citation = {(1220, 2): '2 Corinthians 1: 18–22',
                            (1241, 0): '1 Peter 1: 10–16'}[(page, kind)]
            if page == 1051 and kind == 4:
                assert citation == 'Isaiah 61: 1 (Luke 4: 18)'
                citation = 'Philippians 2: 8–9'
            if page == 958 and kind == 1:
                citation = 'Psalm ' + block[0]['text'].splitlines()[1]
            # Audited EPUB copy/paste errors. Keep Scripture wording intact;
            # discard summaries belonging to a different passage.
            corrections = {
                (1181, 0): ('2 Samuel 24: 2, 9–17', '1 Kings 2: 1–4, 10–12'),
                (1375, 0): ('Jeremiah 2: 1–3, 7–8, 12–13', 'Jeremiah 3: 14–17'),
                (1249, 0): ('Jude 17: 20–25', 'Jude 17, 20b–25'),
                (365, 5): ('John 20: 24–9', 'John 20: 24–29'),
                (1035, 5): ('John 8: 12–20', 'John 8: 1–11'),
                (1002, 0): ('Esther: 12, 14–16, 23–25', 'Esther C: 12, 14–16, 23–25'),
            }
            if (page, kind) in corrections:
                old, citation = corrections[(page, kind)]
                assert old in block[0]['text'], (page, kind)
                if page in (1181, 1375):
                    assert refs[0] & 0x8000
                    refs = refs[1:]
            if page == 1555 and kind == 5:
                assert citation == ('Mark 4: 1–10, 13–20' if start == 19 else 'Mark 4: 1–9')
                citation = 'Matthew 22: 1–14' if start == 19 else 'Matthew 22: 1–10'
            if page == 535 and kind == 5:
                assert citation == 'Luke 6: 12–19' and len(refs) == 5
                citation = 'Luke 6: 12–16'
                refs = refs[:3]  # Stop after the Twelve, before verses 17–19.
            if refs:
                found.append({'page': page, 'start': start, 'kind': kind,
                              'citation': citation, 'refs': refs, 'year': year})
        self.cache[page] = found
        return found

    def mass(self, page, cycle='A', first_alternative=False):
        selected = {}
        for sec in self.sections(page):
            if sec['year'] and sec['year'] != cycle:
                continue
            if sec['kind'] not in selected:
                selected[sec['kind']] = sec
        if first_alternative:
            selected[0] = [s for s in self.sections(page) if s['kind'] == 0][1]
        return [selected[k] for k in sorted(selected)]

    def resolve(self, date, day):
        cycle = day['sundayCycle'][-1]
        id = day['id']
        note = ''
        if id == 'holy_saturday':
            return [], 'No daytime Mass on Holy Saturday. The Easter Vigil is celebrated after nightfall; vigil selection is not included in this edition.'
        if day['rank'] in ('MEMORIAL', 'COMMEMORATION', 'OPTIONAL_MEMORIAL'):
            if id in PROPER_MEMORIALS:
                return self.mass(PROPER_MEMORIALS[id], cycle), 'Proper readings for this memorial.'
            assert day['weekday'], (date, id)
            id = day['weekday']['id']
            note = 'Weekday readings for this memorial (GIRM 357–358).'
        if id == 'commemoration_of_all_the_faithful_departed':
            # One allowed set from the supplied Common of the Dead.
            result = []
            for page in (868, 877, 887, 902, 914):
                result.extend(self.mass(page, cycle))
            return sorted(result, key=lambda s:s['kind']), 'One selection from the Common of the Dead. Other approved choices may be used.'
        if id == 'palm_sunday_of_the_passion_of_the_lord':
            result = self.mass(1042, cycle)
            if cycle != 'A':
                result = [r for r in result if r['kind'] != 5] + self.mass(1043 if cycle == 'B' else 1044, cycle)
            return result, 'Mass readings; the procession Gospel is not included. Long Passion form.'
        if id in FIXED:
            result = self.mass(FIXED[id], cycle)
            if day['rank'] == 'FEAST' and date.weekday() != 6:
                result = [r for r in result if r['kind'] != 2]
            if id == 'nativity_of_the_lord': note = 'Christmas Mass during the Day. Vigil, Night and Dawn forms are not selected automatically.'
            if id == 'holy_thursday': note = "Evening Mass of the Lord's Supper."
            if id == 'friday_of_the_passion_of_the_lord': note = "Good Friday: celebration of the Lord's Passion (not Mass)."
            return result, note
        if id in CYCLIC:
            result = self.mass(CYCLIC[id] + ord(cycle)-ord('A'), cycle)
            if day['rank'] == 'FEAST' and date.weekday() != 6:
                result = [r for r in result if r['kind'] != 2]
            return result, note
        if id == 'sunday_of_the_word_of_god': id = 'ordinary_time_3_sunday'
        if id.startswith('christmas_octave_day_'):
            return self.mass(965+int(id.rsplit('_',1)[1]), cycle), note
        if id.startswith('christmas_time_january_'):
            return self.mass(974+int(id.rsplit('_',1)[1]), cycle), note
        if id.endswith('_after_epiphany'):
            # General Roman Calendar keeps Epiphany on January 6: use DATE,
            # not the optional weekday labels intended for transferred Epiphany.
            return self.mass(975+date.day, cycle), note
        if re.fullmatch(r'advent_december_\d+', id):
            return self.mass(937+date.day, cycle), note
        if id.startswith('easter_octave_') or id in {f'easter_{d}' for d in DOW[1:]}:
            dow = (date.weekday()+1)%7
            return self.mass(1055+dow, cycle), note
        m = re.fullmatch(r'(ordinary_time|advent|lent|easter)_(\d+)_(\w+)', id.replace('easter_time_', 'easter_'))
        if not m:
            raise ValueError(f'Unmapped celebration {date}: {id}')
        season, week, dow = m[1], int(m[2]), m[3]
        c = cycle.lower() if dow == 'sunday' else ('i' if day['weekdayCycle']=='YEAR_1' else 'ii') if season=='ordinary_time' else ''
        page = self.temporal[(season, week, dow, c)]
        result = self.mass(page, cycle, page==927 and cycle=='A')
        gospel = None
        if page == 1083 and cycle == 'A':
            gospel = next(s for s in self.sections(1081) if s['kind'] == 5)
        if page in (1398,1399,1400,1401) and cycle == 'A':
            gospel = [s for s in self.sections(page) if s['kind'] == 5][1]
        if gospel:
            result = [gospel if s['kind'] == 5 else s for s in result]
        if page == 1035 and cycle == 'C':
            # The EPUB mislabels John 8:1–11 as 8:12–20. Use the missing
            # passage supplied directly by the owner, appended at extraction.
            supplement = john_8()
            gospel = {'page': 1035, 'start': -1, 'kind': 5,
                      'citation': supplement['citation'], 'year': 'C',
                      'refs': [self.segments.index(p) for p in supplement['paragraphs']]}
            result = [gospel if s['kind'] == 5 else s for s in result]
            note = 'Year C Gospel: John 8:12–20. John 8:1–11 applies if Year A readings were used on the preceding Sunday.'
        return result, note


def build():
    calendar = json.loads((ROOT/'artifacts/calendar-days.json').read_text())
    extracted = json.loads((ROOT/'artifacts/readings.json').read_text())
    assert unpack((ROOT/'resources/readings.pmr').read_bytes()) == (extracted['pages'], extracted['segments']), 'Reading resource and source catalog differ'
    cat = Catalog(next((ROOT/'data/source').glob('*.epub')), extracted)
    assert len(cat.segments) < 32768
    dates = sorted(calendar['days'])
    assert dates == [(dt.date(calendar['first'],1,1)+dt.timedelta(days=i)).isoformat()
                     for i in range((dt.date(calendar['last']+1,1,1)-dt.date(calendar['first'],1,1)).days)]
    # Compile a compact dictionary of readings, mass sets and strings.
    strings = bytearray(b'\0'); string_ids = {'': 0}
    def string(value):
        value = value.replace('–','-').replace('’', "'")
        if value not in string_ids:
            string_ids[value] = len(strings); strings.extend(value.encode()+b'\0')
        return string_ids[value]
    sections, sec_ids, parts, refs, masses, mass_ids, days = [], {}, [], [], [], {}, []
    report = []
    for iso, celebrations in sorted(calendar['days'].items()):
        date = dt.date.fromisoformat(iso); day = celebrations[0]
        selection, note = cat.resolve(date, day)
        assert not selection or {0,1,5}.issubset({r['kind'] for r in selection}), (iso,day,selection)
        chosen = []
        for r in selection:
            assert r['citation'] or r['kind'] in (3,4), (iso,r)
            key = (r['kind'], r['citation'], tuple(r['refs']))
            if key not in sec_ids:
                sec_ids[key] = len(sections)
                first_part = len(parts); group = []; size = 0
                for ref in r['refs']:
                    length = len(cat.segments[ref & 0x7fff].encode())+2
                    if group and (size+length>6500 or len(group)==56):
                        parts.append((len(refs),len(group))); refs.extend(group);group=[];size=0
                    group.append(ref);size+=length
                if group: parts.append((len(refs),len(group)));refs.extend(group)
                sections.append((string(r['citation']),first_part,len(parts)-first_part,r['kind']))
            chosen.append(sec_ids[key])
        key = tuple(chosen)
        if key not in mass_ids:
            mass_ids[key] = len(masses); masses.append(key)
        display_rank = 'COMMEMORATION' if day['id']=='commemoration_of_all_the_faithful_departed' else day['rank']
        optional = '\n\n'.join(f"{x['rank'].replace('_',' ').title()}: {x['name']}" for x in celebrations[1:])
        details = f"{display_rank.replace('_',' ').title()}: {day['name']}"
        if optional: details += '\n\n'+optional
        if note: details += '\n\n'+note
        cyc = (ord(day['sundayCycle'][-1])-65) | ((day['weekdayCycle']=='YEAR_2')<<2)
        if day['colors']:
            color = day['colors'][0]
        else:
            # No daytime Mass/assigned color on Holy Saturday. Keep it neutral.
            assert day['id']=='holy_saturday', (iso,day)
            color = 'NONE'
        days.append((mass_ids[key],string(day['name']),string(details),SEASONS.index(day['season']),day['week'],cyc,RANKS.index(display_rank),COLORS.index(color)))
        report.append({'date':iso,'id':day['id'],'name':day['name'],'rank':display_rank,'season':day['season'],'week':day['week'],'color':color,'readings':[{'kind':KINDS[r['kind']],'page':r['page'],'start':r['start'],'citation':r['citation'],'refs':r['refs']} for r in selection]})
    # Fixed-width records, read directly from flash. Offsets are absolute except
    # string offsets (relative to the string pool). Little endian, no C packing.
    header_size = 48
    day_bytes = b''.join(struct.pack('<HII5B',*d) for d in days)
    mass_bytes = b''.join(struct.pack('<B7H',len(m),*(list(m)+[0]*(7-len(m)))) for m in masses)
    sec_bytes = b''.join(struct.pack('<IHBB',*s) for s in sections)
    part_bytes = b''.join(struct.pack('<IH',*p) for p in parts)
    ref_bytes = struct.pack(f'<{len(refs)}H',*refs)
    blocks=[day_bytes,mass_bytes,sec_bytes,part_bytes,ref_bytes,strings]
    offsets=[];at=header_size
    for block in blocks:offsets.append(at);at+=len(block)
    start = (dt.date(calendar['first'],1,1)-dt.date(1970,1,1)).days
    header=struct.pack('<4s11I',b'CAL1',start,len(days),len(masses),len(sections),len(parts),*offsets)
    blob=header+b''.join(blocks)
    # A single synthetic PMR2 segment wraps the binary index. This reuses the
    # checked bounded-memory decoder without adding another compression format.
    raw = struct.pack('<IIIIII', 0, 0, 1, 0, 24, len(blob)) + blob
    chunks = [zlib.compress(raw[i:i+CHUNK_SIZE],9) for i in range(0,len(raw),CHUNK_SIZE)]
    packed = bytearray(HEADER.pack(b'PMR2',CHUNK_SIZE,len(chunks),len(raw),1,1,12,16,24,0))
    offset = HEADER.size + 8*len(chunks)
    for chunk in chunks:
        packed.extend(struct.pack('<II',offset,len(chunk)));offset+=len(chunk)
    packed.extend(b''.join(chunks))
    (ROOT/'artifacts/calendar.bin').write_bytes(packed)
    (ROOT/'artifacts/calendar-raw.bin').write_bytes(blob)
    (ROOT/'artifacts/calendar-selection.json').write_text(json.dumps(report,ensure_ascii=False))
    print(json.dumps({'days':len(days),'masses':len(masses),'readings':len(sections),'parts':len(parts),'bytes':len(packed),'raw_bytes':len(blob),'strings':len(strings)},indent=2))
    assert len(packed)+len((ROOT/'resources/readings.pmr').read_bytes())+8192<=1048576,'Calendar exceeds resource budget'

if __name__=='__main__':build()

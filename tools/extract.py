#!/usr/bin/env python3
"""EPUB inspection and lossless, chunked English-side storage experiment.
No calendar precedence or date-to-reading resolver is implemented here.
"""
import argparse
import hashlib
import json
import pathlib
import re
import xml.etree.ElementTree as ET
import zipfile
import pmr
from audit import audit_sample, audit_october8, review_candidate

NS = {'h': 'http://www.w3.org/1999/xhtml'}
READING = re.compile(r'^(?:First reading|Second reading|Gospel(?:\s|$)|Responsorial Psalm)', re.I)
MONTH = r'January|February|March|April|May|June|July|August|September|October|November|December'

def text(el):
    # Keep inline words intact (e.g. L<span>ord</span>) and explicit line breaks.
    def walk(node):
        value = node.text or ''
        for child in node:
            value += '\n' if child.tag.endswith('}br') else walk(child)
            value += child.tail or ''
        return value
    return '\n'.join(re.sub(r'\s+', ' ', line).strip() for line in walk(el).split('\n') if line.strip())

def segments(node):
    if 'left' in node.get('class', '').split():
        return
    if node.tag.split('}')[-1] in ('p', 'h1', 'h2', 'h3', 'li'):
        value = text(node)
        if value:
            yield value
        return
    # Some instructions occur as standalone spans, outside paragraphs.
    if node.tag.endswith('}span'):
        value = text(node)
        if value:
            yield value
        return
    for child in node:
        yield from segments(child)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('epub', type=pathlib.Path)
    parser.add_argument('--out', type=pathlib.Path, default=pathlib.Path(__file__).resolve().parents[1] / 'artifacts')
    parser.add_argument('--compression', choices=['zlib', 'zopfli'], default='zlib',
                        help='zopfli builds a smaller compatible resource; requires the zopfli package')
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    pages, inventory, unique, ids, expected = [], [], [], {}, []
    reviews = []
    sample_audit = None
    october8_audit = None
    with zipfile.ZipFile(args.epub) as archive:
        for name in sorted(archive.namelist()):
            if not re.fullmatch(r'text/part\d+\.html', name):
                continue
            body = ET.fromstring(archive.read(name)).find('h:body', NS)
            parts = list(segments(body))
            headings = [text(e) for e in body.iter() if e.tag.endswith('}p') and 'centertextred' in e.get('class', '')]
            links = [{'label': text(e), 'href': e.get('href')} for e in body.findall('.//h:a', NS)]
            introduction = '\n'.join(parts[:8])
            entry = {'source': name, 'opening': parts[:5],
                     'dates': sorted(set(re.findall(r'\b(?:' + MONTH + r')\s+\d{1,2}\b', introduction))),
                     'cycles': sorted(set(re.findall(r'\bYear\s+(?:III|II|I|A|B|C)\b', introduction))),
                     'ranks': sorted(set(re.findall(r'\b(?:Optional Memorial|Memorial|Solemnity|Feast)\b', introduction))),
                     'links': links}
            inventory.append(entry)
            if not any(READING.match(h) for h in headings):
                if re.search(r"First reading|Second reading|Responsorial Psalm", "\n".join(parts), re.I):
                    reviews.append(review_candidate(name, parts, links))
                continue
            if name == "text/part0519.html":
                sample_audit = audit_sample(body, parts)
            if name == "text/part1549.html":
                october8_audit = audit_october8(body, parts)
            refs = []
            for part in parts:
                if part not in ids:
                    ids[part] = len(unique)
                    unique.append(part)
                refs.append(ids[part])
            pages.append({'source': name, 'segments': refs})
            expected.append(parts)
    pack, raw_size, chunk_count = pmr.pack(pages, unique, args.compression)
    decoded_pages, decoded = pmr.unpack(pack)
    assert len(decoded_pages) == len(expected)
    for page, original in zip(decoded_pages, expected):
        assert [decoded[i] for i in page['segments']] == original
    assert sample_audit is not None
    assert october8_audit is not None
    report = {'source': args.epub.name, 'source_sha256': hashlib.sha256(args.epub.read_bytes()).hexdigest(),
              'html_pages_inspected': len(inventory), 'detected_reading_pages': len(pages),
              'unique_segments': len(unique), 'uncompressed_payload_bytes': raw_size,
              'packed_bytes': len(pack), 'format': 'PMR2', 'compression': args.compression,
              'chunk_bytes': pmr.CHUNK_SIZE, 'chunks': chunk_count,
              'sdk_emery_resource_limit_bytes': 1048576, 'remaining_bytes_before_resource_wrapper': 1048576-len(pack),
              'roundtrip_verified_pages': len(pages),
              'limitations': ['Reading page detection is heuristic, not a completeness certification.',
                             'English-side extraction also retains surrounding headings, navigation and instructions.',
                             'Inventory records source dates/ranks/links; it is not a resolved liturgical calendar.',
                             'Calendar selection and general reading boundaries remain unvalidated.',
                             'SDK App Store resource allowance is 262144 bytes.']}
    (args.out/'sample-audit.json').write_text(json.dumps(sample_audit, ensure_ascii=False, indent=2))
    (args.out/'october8-audit.json').write_text(json.dumps(october8_audit, ensure_ascii=False, indent=2))
    (args.out/'review-candidates.json').write_text(json.dumps(reviews, ensure_ascii=False, indent=2))
    (args.out/'readings.pmr').write_bytes(pack)
    (args.out/'readings.json').write_text(json.dumps({'pages': pages, 'segments': unique}, ensure_ascii=False, indent=2))
    (args.out/'calendar-inventory.json').write_text(json.dumps(inventory, ensure_ascii=False, indent=2))
    (args.out/'report.json').write_text(json.dumps(report, indent=2))
    sample = next(p for p in pages if p['source'] == 'text/part0519.html')
    (args.out/'sample-october-7.txt').write_text('\n\n'.join(unique[i] for i in sample['segments']))
    print(json.dumps(report, indent=2))

if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""EPUB inspection and lossless, chunked English-side storage experiment.
No calendar precedence or date-to-reading resolver is implemented here.
"""
import argparse
import hashlib
import json
import pathlib
import re
import struct
import xml.etree.ElementTree as ET
import zipfile
import zlib

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

def unpack(blob):
    magic, chunk_size, count, raw_size = struct.unpack_from('<4sIII', blob)
    assert magic == b'PMR1'
    out = bytearray()
    for i in range(count):
        offset, length = struct.unpack_from('<II', blob, 16 + i * 8)
        raw = zlib.decompress(blob[offset:offset + length])
        assert len(raw) <= chunk_size
        out.extend(raw)
    assert len(out) == raw_size
    return bytes(out)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('epub', type=pathlib.Path)
    parser.add_argument('--out', type=pathlib.Path, default=pathlib.Path('output'))
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    pages, inventory, unique, ids, expected = [], [], [], {}, []
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
                continue
            refs = []
            for part in parts:
                if part not in ids:
                    ids[part] = len(unique)
                    unique.append(part)
                refs.append(ids[part])
            pages.append({'source': name, 'segments': refs})
            expected.append(parts)
    # Every extracted paragraph, heading, and instruction is represented; no
    # lossy typography replacements or paragraph truncation are applied.
    meta = json.dumps({'pages': pages}, ensure_ascii=False, separators=(',', ':')).encode()
    data = bytearray(struct.pack('<I', len(meta)) + meta + struct.pack('<I', len(unique)))
    for part in unique:
        encoded = part.encode()
        data.extend(struct.pack('<I', len(encoded)))
        data.extend(encoded)
    chunk_size = 16384
    chunks = [zlib.compress(data[i:i+chunk_size], 9) for i in range(0, len(data), chunk_size)]
    pack = bytearray(struct.pack('<4sIII', b'PMR1', chunk_size, len(chunks), len(data)))
    offset = 16 + len(chunks) * 8
    for chunk in chunks:
        pack.extend(struct.pack('<II', offset, len(chunk)))
        offset += len(chunk)
    pack.extend(b''.join(chunks))
    recovered = unpack(pack)
    assert recovered == bytes(data)
    meta_len = struct.unpack_from('<I', recovered)[0]
    decoded_meta = json.loads(recovered[4:4+meta_len])
    pos = 4 + meta_len
    count = struct.unpack_from('<I', recovered, pos)[0]
    pos += 4
    decoded = []
    for _ in range(count):
        length = struct.unpack_from('<I', recovered, pos)[0]
        pos += 4
        decoded.append(recovered[pos:pos+length].decode())
        pos += length
    assert pos == len(recovered)
    for page, original in zip(decoded_meta['pages'], expected, strict=True):
        assert [decoded[i] for i in page['segments']] == original
    report = {'source': args.epub.name, 'source_sha256': hashlib.sha256(args.epub.read_bytes()).hexdigest(),
              'html_pages_inspected': len(inventory), 'detected_reading_pages': len(pages),
              'unique_segments': len(unique), 'uncompressed_payload_bytes': len(data),
              'packed_bytes': len(pack), 'chunk_bytes': chunk_size, 'chunks': len(chunks),
              'sdk_emery_resource_limit_bytes': 1048576, 'remaining_bytes_before_resource_wrapper': 1048576-len(pack),
              'roundtrip_verified_pages': len(pages),
              'limitations': ['Reading page detection is heuristic, not a completeness certification.',
                             'English-side extraction also retains surrounding headings, navigation and instructions.',
                             'Inventory records source dates/ranks/links; it is not a resolved liturgical calendar.',
                             'Watch decoder and runtime memory/performance not yet tested.',
                             'SDK App Store resource allowance is 262144 bytes.']}
    (args.out/'readings.pmr').write_bytes(pack)
    (args.out/'readings.json').write_text(json.dumps({'pages': pages, 'segments': unique}, ensure_ascii=False, indent=2))
    (args.out/'calendar-inventory.json').write_text(json.dumps(inventory, ensure_ascii=False, indent=2))
    (args.out/'report.json').write_text(json.dumps(report, indent=2))
    sample = next(p for p in pages if p['source'] == 'text/part0519.html')
    (args.out/'sample-october-7.txt').write_text('\n\n'.join(unique[i] for i in sample['segments']))
    print(json.dumps(report, indent=2))

if __name__ == '__main__':
    main()

"""PMR2: chunk-compressed binary indexes and verbatim UTF-8 segments."""
import re
import struct
import zlib

CHUNK_SIZE = 16384
HEADER = struct.Struct('<4s9I')


def pack(pages, segments, compression='zlib'):
    if compression == 'zopfli':
        import zopfli.zlib
        compress = lambda data: zopfli.zlib.compress(bytes(data), numiterations=15)
    elif compression == 'zlib':
        compress = lambda data: zlib.compress(data, 9)
    else:
        raise ValueError(f'Unknown compression: {compression}')
    refs = [ref for page in pages for ref in page['segments']]
    refs_offset = len(pages) * 12
    segments_offset = refs_offset + len(refs) * 4
    text_offset = segments_offset + len(segments) * 8
    raw = bytearray()
    first = 0
    for page in pages:
        source = int(re.fullmatch(r'text/part(\d+)\.html', page['source'])[1])
        raw.extend(struct.pack('<III', source, first, len(page['segments'])))
        first += len(page['segments'])
    raw.extend(struct.pack(f'<{len(refs)}I', *refs))
    texts = [s.encode('utf-8') for s in segments]
    # Adjacent similar passages compress better. Keep semantic segment IDs and
    # page order unchanged; only rearrange physical bytes behind their offsets.
    order = sorted(range(len(texts)), key=texts.__getitem__)
    offsets = [0] * len(texts)
    offset = text_offset
    for index in order:
        offsets[index] = offset
        offset += len(texts[index])
    for index, text in enumerate(texts):
        raw.extend(struct.pack('<II', offsets[index], len(text)))
    raw.extend(b''.join(texts[index] for index in order))
    chunks = [compress(raw[i:i + CHUNK_SIZE])
              for i in range(0, len(raw), CHUNK_SIZE)]
    result = bytearray(HEADER.pack(b'PMR2', CHUNK_SIZE, len(chunks), len(raw),
                                  len(pages), len(segments), refs_offset,
                                  segments_offset, text_offset, 0))
    offset = HEADER.size + 8 * len(chunks)
    for chunk in chunks:
        assert len(chunk) <= CHUNK_SIZE + 64
        result.extend(struct.pack('<II', offset, len(chunk)))
        offset += len(chunk)
    result.extend(b''.join(chunks))
    assert unpack(result) == (pages, segments)
    return result, len(raw), len(chunks)


def unpack(blob):
    magic, size, count, raw_size, page_count, seg_count, refs, segs, texts, flags = HEADER.unpack_from(blob)
    assert magic == b'PMR2' and size == CHUNK_SIZE and flags == 0
    raw = bytearray()
    for i in range(count):
        offset, length = struct.unpack_from('<II', blob, HEADER.size + i * 8)
        chunk = zlib.decompress(blob[offset:offset + length])
        assert len(chunk) == min(size, raw_size - len(raw))
        raw.extend(chunk)
    assert len(raw) == raw_size
    segments = []
    for i in range(seg_count):
        offset, length = struct.unpack_from('<II', raw, segs + i * 8)
        assert texts <= offset <= len(raw) - length
        segments.append(raw[offset:offset + length].decode('utf-8'))
    pages = []
    for i in range(page_count):
        source, first, length = struct.unpack_from('<III', raw, i * 12)
        ids = list(struct.unpack_from(f'<{length}I', raw, refs + first * 4))
        pages.append({'source': f'text/part{source:04}.html', 'segments': ids})
    return pages, segments

#include "pmr.h"
#include "vendor/tinf/tinf.h"
#include <string.h>

static uint32_t u32(const uint8_t *p) {
  return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static bool resource_read(Pmr *db, uint32_t offset, uint8_t *out, uint32_t size) {
  return offset <= db->size && size <= db->size - offset &&
         db->read(db->context, offset, out, size) == size;
}

bool pmr_open(Pmr *db, PmrRead read, void *context, uint32_t size) {
  memset(db, 0, sizeof(*db));
  db->read = read;
  db->context = context;
  db->size = size;
  db->cached_chunk = UINT32_MAX;
  uint8_t h[40];
  if (!read || !resource_read(db, 0, h, sizeof(h)) || memcmp(h, "PMR2", 4) ||
      u32(h + 4) != PMR_CHUNK || u32(h + 36) != 0) return false;
  db->chunks = u32(h + 8);
  db->raw_size = u32(h + 12);
  db->pages = u32(h + 16);
  db->segments = u32(h + 20);
  db->refs_offset = u32(h + 24);
  db->segments_offset = u32(h + 28);
  db->text_offset = u32(h + 32);
  if (!db->raw_size || db->chunks != (db->raw_size - 1) / PMR_CHUNK + 1 ||
      db->chunks > (size - 40) / 8 || !db->pages || !db->segments ||
      db->pages > db->raw_size / 12 || db->refs_offset != db->pages * 12 ||
      db->segments_offset < db->refs_offset || db->segments_offset > db->raw_size ||
      (db->segments_offset - db->refs_offset) % 4 ||
      db->segments > (db->raw_size - db->segments_offset) / 8 ||
      db->text_offset != db->segments_offset + db->segments * 8) return false;
  db->ready = true;
  return true;
}

static bool chunk_load(Pmr *db, uint32_t chunk) {
  if (db->cached_chunk == chunk) return true;
  db->cached_chunk = UINT32_MAX;
  uint8_t record[8];
  if (chunk >= db->chunks || !resource_read(db, 40 + chunk * 8, record, 8)) return false;
  uint32_t offset = u32(record), length = u32(record + 4);
  if (offset < 40 + db->chunks * 8 || length > PMR_COMPRESSED || length < 6 ||
      !resource_read(db, offset, db->compressed, length)) return false;
  unsigned int decoded = PMR_CHUNK;
  uint32_t expected = db->raw_size - chunk * PMR_CHUNK;
  if (expected > PMR_CHUNK) expected = PMR_CHUNK;
  if (tinf_zlib_uncompress(db->raw, &decoded, db->compressed, length) != TINF_OK ||
      decoded != expected) return false;
  db->cached_chunk = chunk;
  db->inflations++;
  return true;
}

bool pmr_read(Pmr *db, uint32_t offset, void *out, uint32_t size) {
  if (!db->ready || offset > db->raw_size || size > db->raw_size - offset) return false;
  uint8_t *dest = out;
  while (size) {
    uint32_t start = offset % PMR_CHUNK;
    uint32_t take = PMR_CHUNK - start;
    if (take > size) take = size;
    if (!chunk_load(db, offset / PMR_CHUNK)) return false;
    memcpy(dest, db->raw + start, take);
    dest += take;
    offset += take;
    size -= take;
  }
  return true;
}

bool pmr_segment(Pmr *db, uint32_t id, char *out, size_t capacity, uint32_t *length) {
  uint8_t record[8];
  if (capacity) out[0] = 0;
  if (id >= db->segments || !pmr_read(db, db->segments_offset + id * 8, record, 8)) return false;
  uint32_t offset = u32(record), size = u32(record + 4);
  if (offset < db->text_offset || size >= capacity || !pmr_read(db, offset, out, size)) {
    if (capacity) out[0] = 0;
    return false;
  }
  out[size] = 0;
  if (length) *length = size;
  return true;
}

bool pmr_reading(Pmr *db, uint32_t source, uint32_t first, uint32_t count,
                 char *out, size_t capacity) {
  if (!capacity) return false;
  out[0] = 0;
  uint32_t low = 0, high = db->pages;
  uint8_t record[12];
  while (low < high) {
    uint32_t mid = low + (high - low) / 2;
    if (!pmr_read(db, mid * 12, record, 12)) return false;
    if (u32(record) < source) low = mid + 1;
    else high = mid;
  }
  if (low >= db->pages || !pmr_read(db, low * 12, record, 12) || u32(record) != source) return false;
  uint32_t ref = u32(record + 4), total = u32(record + 8);
  uint32_t available = (db->segments_offset - db->refs_offset) / 4;
  if (!count || first > total || count > total - first || ref > available || total > available - ref) return false;
  size_t used = 0;
  for (uint32_t i = 0; i < count; ++i) {
    uint32_t length;
    if (!pmr_read(db, db->refs_offset + (ref + first + i) * 4, record, 4)) goto fail;
    if (i) {
      if (capacity - used <= 2) goto fail;
      out[used++] = '\n';
      out[used++] = '\n';
    }
    if (!pmr_segment(db, u32(record), out + used, capacity - used, &length)) goto fail;
    used += length;
  }
  return true;
fail:
  out[0] = 0;
  return false;
}

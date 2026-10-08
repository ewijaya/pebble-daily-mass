#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PMR_CHUNK 16384
#define PMR_COMPRESSED (PMR_CHUNK + 64)
typedef size_t (*PmrRead)(void *context, uint32_t offset, uint8_t *out, size_t size);
typedef struct {
  PmrRead read;
  void *context;
  uint32_t size, chunks, raw_size, pages, segments, refs_offset, segments_offset, text_offset;
  uint32_t cached_chunk, inflations;
  bool ready;
  uint8_t raw[PMR_CHUNK], compressed[PMR_COMPRESSED];
} Pmr;

bool pmr_open(Pmr *db, PmrRead read, void *context, uint32_t size);
bool pmr_read(Pmr *db, uint32_t offset, void *out, uint32_t size);
bool pmr_segment(Pmr *db, uint32_t id, char *out, size_t capacity, uint32_t *length);
// first/count are local segment positions within the source page; joins with blank lines.
bool pmr_reading(Pmr *db, uint32_t source, uint32_t first, uint32_t count,
                 char *out, size_t capacity);

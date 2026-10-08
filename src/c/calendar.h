#pragma once
#include "pmr.h"
#define CAL_MAX_READINGS 7
#define CAL_MAX_REFS 56
#define CAL_CITATION_SIZE 192
#define CAL_DAY_RECORD_SIZE 15
typedef enum {
  CAL_GREEN, CAL_PURPLE, CAL_WHITE, CAL_RED, CAL_ROSE, CAL_NONE, CAL_COLOR_COUNT
} CalColor;

typedef struct {
  uint32_t citation;
  uint16_t first_part;
  uint8_t parts, kind;
} CalReading;
typedef struct {
  uint32_t name, details;
  uint8_t season, week, cycles, rank, color, count;
  CalReading readings[CAL_MAX_READINGS];
} CalDay;
typedef struct {
  Pmr store;
  uint32_t first_day, day_count, mass_count, reading_count, part_count;
  uint32_t days, masses, readings, parts, refs, strings, size;
  bool ready;
} Calendar;

bool calendar_open(Calendar *cal, PmrRead read, void *context, uint32_t size);
bool calendar_day(Calendar *cal, int year, int month, int day, CalDay *out);
bool calendar_rank(Calendar *cal, int32_t number, uint8_t *rank);
bool calendar_string(Calendar *cal, uint32_t offset, char *out, size_t capacity);
bool calendar_part(Calendar *cal, const CalReading *reading, uint8_t part,
                   uint16_t refs[CAL_MAX_REFS], uint16_t *count);
int32_t calendar_day_number(int year, int month, int day);

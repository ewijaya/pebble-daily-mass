#pragma once
#include "pmr.h"
#define CAL_MAX_READINGS 20
#define CAL_KIND_COUNT 12
#define CAL_MAX_REFS 56
#define CAL_CITATION_SIZE 192
#define CAL_DAY_RECORD_SIZE 18
#define CAL_HEADER_SIZE 56
#define CAL_MASS_RECORD_SIZE (1+2*CAL_MAX_READINGS)
#define CAL_FORM_RECORD_SIZE 11
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
  uint16_t default_mass, first_form;
  uint8_t form_count, default_color;
  CalReading readings[CAL_MAX_READINGS];
} CalDay;
typedef struct {
  Pmr store;
  uint32_t first_day, day_count, mass_count, reading_count, part_count;
  uint32_t days, masses, readings, parts, refs, strings, size;
  uint32_t forms, form_count;
  bool ready;
} Calendar;

bool calendar_open(Calendar *cal, PmrRead read, void *context, uint32_t size);
bool calendar_day(Calendar *cal, int year, int month, int day, CalDay *out);
// Form zero is the daily default. Additional forms are explicitly selected.
bool calendar_form_info(Calendar *cal, const CalDay *day, uint8_t form,
                        uint32_t *label, uint32_t *note);
bool calendar_select_form(Calendar *cal, CalDay *day, uint8_t form);
bool calendar_rank(Calendar *cal, int32_t number, uint8_t *rank);
bool calendar_string(Calendar *cal, uint32_t offset, char *out, size_t capacity);
bool calendar_part(Calendar *cal, const CalReading *reading, uint8_t part,
                   uint16_t refs[CAL_MAX_REFS], uint16_t *count);
int32_t calendar_day_number(int year, int month, int day);

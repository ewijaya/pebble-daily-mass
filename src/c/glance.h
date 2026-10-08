#pragma once
#include "calendar.h"
#if defined(PBL_PLATFORM_EMERY)
#include <pebble.h>
#else
#include <time.h>
#endif

#define GLANCE_TEXT_SIZE 151 // SDK maximum: 150 UTF-8 bytes plus NUL.
#define GLANCE_DAYS 7

bool glance_summary(Calendar *calendar, time_t when, char out[GLANCE_TEXT_SIZE]);
time_t glance_next_midnight(time_t when);

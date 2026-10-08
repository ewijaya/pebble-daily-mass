#pragma once
#include "glance.h"

bool planner_date(int32_t number, struct tm *date);
int32_t planner_sunday(int32_t after);
int32_t planner_holy_day(Calendar *cal, int32_t after);
int32_t planner_evening(Calendar *cal, time_t now);

#include "planner.h"

bool planner_date(int32_t number, struct tm *date) {
  // Interpret a civil day using UTC, avoiding timezone/DST date arithmetic.
  // This edition ends before the watch's signed 32-bit timestamp boundary.
  if (number<0 || number>24836) return false; // 2037-12-31
  time_t noon=(time_t)number*86400+43200;
  const struct tm *t=gmtime(&noon);
  if (!t) return false;
  *date=*t;return true;
}

int32_t planner_sunday(int32_t after) {
  if (after<0 || after>24836) return -1;
  return after+7-(after+4)%7; // Strictly next Sunday, including a seven-day jump on Sunday.
}

int32_t planner_holy_day(Calendar *cal, int32_t after) {
  if (after<0 || after>24836) return -1;
  uint8_t rank;
  for (int i=1;i<=366;i++) {
    if (!calendar_rank(cal,after+i,&rank)) break;
    if (rank==4 || rank==5) return after+i;
  }
  return -1;
}

int32_t planner_evening(Calendar *cal, time_t now) {
  const struct tm *t=localtime(&now);
  if (!t || t->tm_hour<16) return -1;
  bool saturday=t->tm_wday==6;
  int32_t tomorrow=calendar_day_number(t->tm_year+1900,t->tm_mon+1,t->tm_mday)+1;
  uint8_t rank;
  if (!calendar_rank(cal,tomorrow,&rank)) return -1;
  return saturday || rank==5 ? tomorrow : -1;
}


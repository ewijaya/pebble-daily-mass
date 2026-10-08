#include "glance.h"
#include <stdio.h>
#include <string.h>

static int local_day(time_t when) {
  const struct tm *t=localtime(&when);
  return t ? calendar_day_number(t->tm_year+1900,t->tm_mon+1,t->tm_mday) : -1;
}

time_t glance_next_midnight(time_t when) {
  // Find the next civil-date boundary in UTC using the firmware's timezone
  // rules. Unlike adding 86400, this handles 23/25-hour DST days. It also
  // avoids relying on the SDK's mktime interpretation of tm_gmtoff/tm_isdst.
  int day=local_day(when);
  if (day<0) return 0;
  time_t low=when, high=when+36*60*60;
  if (high<=when || local_day(high)<=day) return 0;
  while (high-low>1) {
    time_t middle=low+(high-low)/2;
    if (local_day(middle)<=day) low=middle;
    else high=middle;
  }
  return high;
}

bool glance_summary(Calendar *calendar, time_t when, char out[GLANCE_TEXT_SIZE]) {
  out[0]=0;
  if (!calendar) return false;
  const struct tm *t=localtime(&when);
  CalDay day;
  if (!t || !calendar_day(calendar,t->tm_year+1900,t->tm_mon+1,t->tm_mday,&day)) return false;
  // Compact launcher labels; full calendar names remain in the app.
  static const char *seasons[]={"OT","Advent","Christmas","Lent","Triduum","Easter"};
  static const char *ranks[]={"","","Mem. ","Opt. Mem. ","Feast: ","Sol. ","Commem. "};
  if (day.rank>=2 || !day.week || day.season==4 || (day.season==3 && day.week==6)) {
    char name[128];
    if (!calendar_string(calendar,day.name,name,sizeof(name))) return false;
    // Keep single saints' names ahead of their long titles in the narrow
    // launcher. Plural celebrations retain their names/companions.
    const char *short_name=name, *prefix="";
    if (!strncmp(name,"Saint ",6)) {
      short_name=name+6;prefix="St. ";
      char *title=strchr(name,',');if (title) *title=0;
    } else if (!strncmp(name,"Saints ",7)) {short_name=name+7;prefix="Ss. ";}
    snprintf(out,GLANCE_TEXT_SIZE,"%s%s%s",ranks[day.rank],prefix,short_name);
  } else if (day.week && day.season!=2) {
    snprintf(out,GLANCE_TEXT_SIZE,"%s \xc2\xb7 Wk %u",seasons[day.season],day.week);
  } else snprintf(out,GLANCE_TEXT_SIZE,"%s",seasons[day.season]);
  // A source name is at most 127 bytes; even the longest rank prefix fits
  // within 150, so UTF-8 names are never cut in the middle of a character.
  return true;
}

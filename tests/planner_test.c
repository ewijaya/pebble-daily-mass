#include "planner.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static FILE *resource;
static Calendar calendar;
static size_t read_file(void *context,uint32_t offset,uint8_t *out,size_t size) {
  if(fseek(resource,offset,SEEK_SET))return 0;
  return fread(out,1,size,resource);
}
static int32_t day(int y,int m,int d) {return calendar_day_number(y,m,d);}
static time_t at(int y,int m,int d,int hour,int minute) {
  struct tm t={.tm_year=y-1900,.tm_mon=m-1,.tm_mday=d,.tm_hour=hour,.tm_min=minute,.tm_isdst=-1};
  return mktime(&t);
}
int main(int argc,char **argv) {
  assert(argc==2);
  resource=fopen(argv[1],"rb");assert(resource);fseek(resource,0,SEEK_END);long bytes=ftell(resource);
  assert(calendar_open(&calendar,read_file,NULL,bytes));
  assert(!setenv("TZ","Asia/Tokyo",1));tzset();
  for(int32_t n=calendar.first_day;n<(int32_t)(calendar.first_day+calendar.day_count);n++) {
    struct tm t;assert(planner_date(n,&t));
    assert(day(t.tm_year+1900,t.tm_mon+1,t.tm_mday)==n);
    int next=planner_sunday(n);assert(next>n && next<=n+7 && (next+4)%7==0);
  }
  struct tm t;assert(!planner_date(-1,&t));assert(!planner_date(day(2038,1,1),&t));
  assert(planner_sunday(day(2026,10,7))==day(2026,10,11));
  assert(planner_sunday(day(2026,10,11))==day(2026,10,18));
  assert(planner_holy_day(&calendar,day(2026,10,8))==day(2026,10,28));
  assert(planner_holy_day(&calendar,day(2026,10,28))==day(2026,11,1));
  assert(planner_holy_day(&calendar,day(2037,12,31))==-1);
  assert(planner_holy_day(&calendar,INT32_MAX)==-1);
  assert(planner_evening(&calendar,at(2026,10,10,15,59))==-1);
  assert(planner_evening(&calendar,at(2026,10,10,16,0))==day(2026,10,11));
  assert(planner_evening(&calendar,at(2026,10,9,18,0))==-1);
  assert(planner_evening(&calendar,at(2026,12,24,16,0))==day(2026,12,25));
  assert(planner_evening(&calendar,at(2026,4,4,16,0))==day(2026,4,5)); // Day Mass only, UI discloses missing Vigil.
  assert(planner_evening(&calendar,at(2026,11,1,18,0))==-1); // All Souls is a commemoration.
  assert(planner_evening(&calendar,at(2037,12,31,18,0))==-1);
  assert(!setenv("TZ","America/New_York",1));tzset();
  assert(planner_evening(&calendar,at(2026,3,7,16,0))==day(2026,3,8));
  assert(planner_evening(&calendar,at(2026,10,31,16,0))==day(2026,11,1));
  fclose(resource);
  puts("Verified all civil dates/next Sundays, holy-day precedence, evening boundaries and DST boundaries");
}

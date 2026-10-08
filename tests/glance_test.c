#include "glance.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *resource;
static Calendar calendar;
static size_t read_file(void *context, uint32_t offset, uint8_t *out, size_t size) {
  if (fseek(resource,offset,SEEK_SET)) return 0;
  return fread(out,1,size,resource);
}
static time_t at(int y,int m,int d,int h,int min,int sec) {
  struct tm t={.tm_year=y-1900,.tm_mon=m-1,.tm_mday=d,.tm_hour=h,.tm_min=min,.tm_sec=sec,.tm_isdst=-1};
  return mktime(&t);
}
static void zone(const char *name) {assert(!setenv("TZ",name,1));tzset();}
static void boundary(int y,int m,int d,int hours) {
  time_t start=at(y,m,d,0,0,0), end=glance_next_midnight(start);
  assert(end-start==hours*3600);
  assert(glance_next_midnight(end-1)==end);
  struct tm t=*localtime(&end);
  assert(t.tm_hour==0 && t.tm_min==0 && t.tm_sec==0);
}
static void expect(int y,int m,int d,const char *expected) {
  char text[GLANCE_TEXT_SIZE];
  assert(glance_summary(&calendar,at(y,m,d,12,0,0),text));
  if (strcmp(text,expected)) fprintf(stderr,"Got: %s; expected: %s\n",text,expected);
  assert(!strcmp(text,expected));
}
int main(int argc,char **argv) {
  assert(argc==2);resource=fopen(argv[1],"rb");assert(resource);
  fseek(resource,0,SEEK_END);long bytes=ftell(resource);rewind(resource);
  assert(calendar_open(&calendar,read_file,NULL,bytes));
  zone("Asia/Tokyo");
  expect(2026,10,8,"OT · Wk 27");
  expect(2026,10,15,"Mem. St. Teresa of Jesus");
  expect(2026,11,1,"Sol. All Saints");
  expect(2026,11,29,"Advent · Wk 1");
  expect(2026,4,3,"Friday of the Passion of the Lord");
  expect(2026,6,13,"OT · Wk 10"); // Colliding memorials are optional.
  boundary(2024,2,29,24);boundary(2026,12,31,24);
  time_t t=at(2026,10,8,12,34,56),end=glance_next_midnight(t);
  assert(end==at(2026,10,9,0,0,0));
  for (int i=0;i<GLANCE_DAYS;i++) {assert(end>t);t=end;end=glance_next_midnight(t);}
  char text[GLANCE_TEXT_SIZE];
  assert(!glance_summary(&calendar,at(2038,1,1,12,0,0),text) && !text[0]);
  assert(!glance_summary(NULL,t,text) && !text[0]);
  zone("America/New_York");boundary(2026,3,8,23);boundary(2026,11,1,25);
  zone("Asia/Kathmandu");boundary(2026,10,8,24);
  zone("Pacific/Apia");boundary(2011,12,29,24); // Skipped civil date.
  zone("UTC");
  unsigned count=0;
  for (int y=2020;y<=2037;y++) for (int m=1;m<=12;m++) for(int d=1;d<=31;d++) {
    if (calendar_day_number(y,m,d)<0) continue;
    assert(glance_summary(&calendar,at(y,m,d,12,0,0),text));
    assert(text[0] && strlen(text)<=150 && !strchr(text,'{') && !strchr(text,'}'));
    count++;
  }
  assert(count==6575);fclose(resource);
  printf("Verified %u glance summaries, local midnight, leap/year rollover, DST and out-of-range handling\n",count);
}

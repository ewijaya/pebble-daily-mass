#include "calendar.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { uint8_t *data; size_t size; } File;
static Calendar cal;
static Pmr scripture;
static uint16_t u16(const uint8_t *p) { return p[0] | (uint16_t)p[1]<<8; }
static uint32_t u32(const uint8_t *p) { return u16(p) | (uint32_t)u16(p+2)<<16; }
static size_t read_file(void *context, uint32_t offset, uint8_t *out, size_t size) {
  File *f=context;
  if (offset>f->size || size>f->size-offset) return 0;
  memcpy(out,f->data+offset,size);return size;
}
static File load(const char *path) {
  FILE *f=fopen(path,"rb");assert(f);
  assert(!fseek(f,0,SEEK_END));size_t n=ftell(f);rewind(f);
  File result={malloc(n),n};assert(result.data);
  assert(fread(result.data,1,n,f)==n);fclose(f);return result;
}
int main(int argc, char **argv) {
  assert(argc==4);
  File file=load(argv[1]), raw=load(argv[2]), bible=load(argv[3]);
  assert(calendar_open(&cal,read_file,&file,file.size));
  assert(pmr_open(&scripture,read_file,&bible,bible.size));
  uint32_t days=0, parts=0;
  bool seen[65536]={false};
  char name[128],details[1600],citation[CAL_CITATION_SIZE],text[8192];
  for (int year=2020;year<=2037;year++) for(int month=1;month<=12;month++) for(int d=1;d<=31;d++) {
    CalDay day;
    int n=calendar_day_number(year,month,d);
    if(n<0) {assert(!calendar_day(&cal,year,month,d,&day));continue;}
    assert(calendar_day(&cal,year,month,d,&day));
    const uint8_t *rec=raw.data+CAL_HEADER_SIZE+days*CAL_DAY_RECORD_SIZE;
    assert(n==(int)(cal.first_day+days));days++;
    assert(day.season==rec[10] && day.week==rec[11] && day.cycles==rec[12] && day.rank==rec[13]);
    assert(day.color==rec[14] && day.color<CAL_COLOR_COUNT);
    uint8_t rank;assert(calendar_rank(&cal,n,&rank) && rank==day.rank);
    assert(calendar_string(&cal,day.name,name,sizeof(name)));
    assert(calendar_string(&cal,day.details,details,sizeof(details)));
    assert(!strcmp(name,(char*)raw.data+cal.strings+u32(rec+2)));
    assert(!strcmp(details,(char*)raw.data+cal.strings+u32(rec+6)));
    assert(day.first_form==u16(rec+15) && day.form_count==rec[17]);
    assert(!calendar_select_form(&cal,&day,day.form_count+1));
    for (unsigned form=0;form<=day.form_count;form++) {
      uint16_t mass_id=u16(rec);
      if (form) {
        const uint8_t *f=raw.data+cal.forms+(day.first_form+form-1)*CAL_FORM_RECORD_SIZE;
        uint32_t label,note;
        assert(calendar_form_info(&cal,&day,form,&label,&note));
        assert(label==u32(f+2) && note==u32(f+6));
        assert(calendar_string(&cal,label,name,sizeof(name)) && name[0]);
        assert(calendar_string(&cal,note,details,sizeof(details)) && details[0]);
        mass_id=u16(f);
        assert(calendar_select_form(&cal,&day,form) && day.color==f[10]);
      }
      const uint8_t *mass=raw.data+cal.masses+mass_id*CAL_MASS_RECORD_SIZE;
      assert(day.count==mass[0]);
      for(unsigned i=0;i<day.count;i++) {
        CalReading *r=day.readings+i;
        assert(calendar_string(&cal,r->citation,citation,sizeof(citation)));
        uint16_t id=u16(mass+1+2*i);
        const uint8_t *expected=raw.data+cal.readings+id*8;
        assert(r->citation==u32(expected) && r->first_part==u16(expected+4) && r->parts==expected[6] && r->kind==expected[7]);
        if(seen[id])continue;
        seen[id]=true;
        for(unsigned p=0;p<r->parts;p++) {
          uint16_t refs[CAL_MAX_REFS],count;
          assert(calendar_part(&cal,r,p,refs,&count));parts++;
          const uint8_t *part=raw.data+cal.parts+(r->first_part+p)*6;
          assert(count==u16(part+4));
          size_t bytes=strlen(citation)+2;
          for(unsigned j=0;j<count;j++) {
            assert(refs[j]==u16(raw.data+cal.refs+(u32(part)+j)*2));
            uint32_t length;
            assert(pmr_segment(&scripture,refs[j]&0x7fff,text,sizeof(text),&length));
            assert(length>0 && !strstr(text,"\n\n"));bytes+=length+2;
          }
          assert(bytes<sizeof(text) && count+1<=64);
        }
        uint16_t refs[CAL_MAX_REFS], count=99;
        assert(!calendar_part(&cal,r,r->parts,refs,&count) && count==0);
      }
    }
    assert(calendar_select_form(&cal,&day,0));
    assert(day.color==rec[14] && day.default_mass==u16(rec));
  }
  CalDay day;
  assert(days==cal.day_count && parts==cal.part_count);
  assert(calendar_day_number(1970,1,1)==0);
  assert(calendar_day_number(2024,3,1)-calendar_day_number(2024,2,28)==2);
  assert(calendar_day_number(2100,2,29)==-1);
  assert(!calendar_day(&cal,2019,12,31,&day));assert(!calendar_day(&cal,2038,1,1,&day));
  assert(!calendar_day(&cal,2026,2,29,&day));assert(!calendar_day(&cal,2026,13,1,&day));
  assert(!calendar_string(&cal,UINT32_MAX,text,sizeof(text)) && !text[0]);
  assert(!calendar_string(&cal,1,text,1) && !text[0]);
  assert(!calendar_string(&cal,0,text,0));
  file.size=40;assert(!calendar_open(&cal,read_file,&file,file.size));
  printf("Verified %u dates and %u complete reading parts; source references, buffer limits, invalid dates and bounds passed\n",days,parts);
  free(file.data);free(raw.data);free(bible.data);
}

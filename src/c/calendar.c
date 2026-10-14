#include "calendar.h"
#include <string.h>

static uint16_t u16(const uint8_t *p) { return (uint16_t)p[0] | (uint16_t)p[1]<<8; }
static uint32_t u32(const uint8_t *p) { return (uint32_t)u16(p) | (uint32_t)u16(p+2)<<16; }
static bool read_at(Calendar *c, uint32_t offset, void *out, uint32_t size) {
  return offset <= c->size && size <= c->size-offset && pmr_read(&c->store,24+offset,out,size);
}

int32_t calendar_day_number(int year, int month, int day) {
  // Gregorian civil day number relative to 1970-01-01; no timezone/DST math.
  static const uint8_t lengths[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if (year<1970 || year>2200 || month<1 || month>12) return -1;
  bool leap=year%4==0 && (year%100!=0 || year%400==0);
  if (day<1 || day>lengths[month-1]+(month==2 && leap)) return -1;
  int32_t total=0;
  for (int y=1970;y<year;y++) total+=365+(y%4==0 && (y%100!=0 || y%400==0));
  for (int m=1;m<month;m++) total+=lengths[m-1]+(m==2 && leap);
  return total+day-1;
}

bool calendar_open(Calendar *c, PmrRead read, void *context, uint32_t size) {
  memset(c,0,sizeof(*c));
  if (!pmr_open(&c->store,read,context,size) || c->store.raw_size<24+CAL_HEADER_SIZE ||
      c->store.pages!=1 || c->store.segments!=1 || c->store.text_offset!=24) return false;
  c->size=c->store.raw_size-24;
  uint8_t h[CAL_HEADER_SIZE];
  if (!read_at(c,0,h,sizeof(h)) || memcmp(h,"CAL2",4)) return false;
  c->first_day=u32(h+4); c->day_count=u32(h+8); c->mass_count=u32(h+12);
  c->reading_count=u32(h+16); c->part_count=u32(h+20);
  c->days=u32(h+24); c->masses=u32(h+28); c->readings=u32(h+32);
  c->parts=u32(h+36); c->refs=u32(h+40); c->strings=u32(h+44);
  c->forms=u32(h+48); c->form_count=u32(h+52);
  if (!c->day_count || !c->mass_count || c->days!=CAL_HEADER_SIZE || c->strings>=c->size ||
      c->days>c->masses || c->masses>c->readings || c->readings>c->parts ||
      c->parts>c->refs || c->refs>c->forms || c->forms>c->strings ||
      c->day_count!=(c->masses-c->days)/CAL_DAY_RECORD_SIZE || (c->masses-c->days)%CAL_DAY_RECORD_SIZE ||
      c->mass_count!=(c->readings-c->masses)/CAL_MASS_RECORD_SIZE || (c->readings-c->masses)%CAL_MASS_RECORD_SIZE ||
      c->reading_count!=(c->parts-c->readings)/8 || (c->parts-c->readings)%8 ||
      c->part_count!=(c->refs-c->parts)/6 || (c->refs-c->parts)%6 ||
      (c->forms-c->refs)%2 ||
      c->form_count!=(c->strings-c->forms)/CAL_FORM_RECORD_SIZE || (c->strings-c->forms)%CAL_FORM_RECORD_SIZE) return false;
  c->ready=true;
  return true;
}

bool calendar_string(Calendar *c, uint32_t offset, char *out, size_t capacity) {
  if (!capacity) return false;
  out[0]=0;
  if (!c->ready || offset>=c->size-c->strings) return false;
  for (size_t i=0;i<capacity;i++) {
    if (!read_at(c,c->strings+offset+i,out+i,1)) break;
    if (!out[i]) return true;
  }
  out[0]=0;
  return false;
}

bool calendar_rank(Calendar *c, int32_t number, uint8_t *rank) {
  // Planning scans only the date table, avoiding decompression of every Mass.
  return c && c->ready && number>=0 && (uint32_t)number>=c->first_day &&
    (uint32_t)number-c->first_day<c->day_count &&
    read_at(c,c->days+((uint32_t)number-c->first_day)*CAL_DAY_RECORD_SIZE+13,rank,1) && *rank<=6;
}

bool calendar_day(Calendar *c, int year, int month, int day, CalDay *out) {
  memset(out,0,sizeof(*out));
  int32_t n=calendar_day_number(year,month,day);
  if (!c->ready || n<0 || (uint32_t)n<c->first_day || (uint32_t)n-c->first_day>=c->day_count) return false;
  uint8_t data[CAL_DAY_RECORD_SIZE];
  if (!read_at(c,c->days+((uint32_t)n-c->first_day)*CAL_DAY_RECORD_SIZE,data,CAL_DAY_RECORD_SIZE)) return false;
  out->default_mass=u16(data);
  out->first_form=u16(data+15); out->form_count=data[17];
  out->name=u32(data+2); out->details=u32(data+6);
  out->season=data[10]; out->week=data[11]; out->cycles=data[12]; out->rank=data[13];
  out->color=out->default_color=data[14];
  if (out->default_mass>=c->mass_count || out->season>=6 || out->week>34 || out->cycles>6 || out->rank>6 || out->color>=CAL_COLOR_COUNT ||
      out->name>=c->size-c->strings || out->details>=c->size-c->strings ||
      out->form_count>5 || out->first_form>c->form_count || out->form_count>c->form_count-out->first_form ||
      !calendar_select_form(c,out,0)) {
    memset(out,0,sizeof(*out));return false;
  }
  return true;
}

static bool form_record(Calendar *c, const CalDay *day, uint8_t form, uint8_t *data) {
  return c && c->ready && form>0 && form<=day->form_count &&
    day->first_form<c->form_count && form<=c->form_count-day->first_form &&
    read_at(c,c->forms+(day->first_form+form-1)*CAL_FORM_RECORD_SIZE,data,CAL_FORM_RECORD_SIZE) &&
    u16(data)<c->mass_count && u32(data+2)<c->size-c->strings &&
    u32(data+6)<c->size-c->strings && data[10]<CAL_COLOR_COUNT;
}

bool calendar_form_info(Calendar *c, const CalDay *day, uint8_t form,
                        uint32_t *label, uint32_t *note) {
  *label=*note=0;
  uint8_t data[CAL_FORM_RECORD_SIZE];
  if (!form_record(c,day,form,data)) return false;
  *label=u32(data+2);*note=u32(data+6);return true;
}

bool calendar_select_form(Calendar *c, CalDay *out, uint8_t form) {
  if (!c || !c->ready) return false;
  uint16_t mass=out->default_mass;
  uint8_t color=out->default_color, data[CAL_MASS_RECORD_SIZE];
  if (form) {
    if (!form_record(c,out,form,data)) return false;
    mass=u16(data);color=data[10];
  }
  if (mass>=c->mass_count || color>=CAL_COLOR_COUNT ||
      !read_at(c,c->masses+mass*CAL_MASS_RECORD_SIZE,data,sizeof(data)) ||
      data[0]>CAL_MAX_READINGS) goto fail;
  out->count=data[0];
  for (uint8_t i=0;i<out->count;i++) {
    uint16_t id=u16(data+1+2*i);
    uint8_t rec[8];
    if (id>=c->reading_count || !read_at(c,c->readings+id*8,rec,8)) goto fail;
    CalReading *r=out->readings+i;
    r->citation=u32(rec);r->first_part=u16(rec+4);r->parts=rec[6];r->kind=rec[7];
    if (!r->parts || r->kind>=CAL_KIND_COUNT || r->citation>=c->size-c->strings ||
        r->first_part>=c->part_count || r->parts>c->part_count-r->first_part) goto fail;
  }
  out->color=color;
  return true;
fail:
  out->count=0;return false;
}

bool calendar_part(Calendar *c, const CalReading *r, uint8_t part,
                   uint16_t refs[CAL_MAX_REFS], uint16_t *count) {
  *count=0;
  if (!c->ready || part>=r->parts || (uint32_t)r->first_part+part>=c->part_count) return false;
  uint8_t rec[6], data[CAL_MAX_REFS*2];
  if (!read_at(c,c->parts+((uint32_t)r->first_part+part)*6,rec,6)) return false;
  uint32_t first=u32(rec);uint16_t n=u16(rec+4);
  uint32_t available=(c->forms-c->refs)/2;
  if (!n || n>CAL_MAX_REFS || first>available || n>available-first ||
      !read_at(c,c->refs+first*2,data,n*2)) return false;
  for (uint16_t i=0;i<n;i++) refs[i]=u16(data+2*i);
  *count=n;return true;
}

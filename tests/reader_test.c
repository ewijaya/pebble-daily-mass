#include "pmr.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *blob;
static size_t blob_size;
static Pmr db;
static uint32_t get32(const uint8_t *p) {
  return (uint32_t)p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;
}
static size_t read_blob(void *context, uint32_t offset, uint8_t *out, size_t size) {
  if (offset > blob_size || size > blob_size-offset) return 0;
  memcpy(out, blob+offset, size);
  return size;
}
static uint8_t *load(const char *path, size_t *size) {
  FILE *f = fopen(path,"rb"); assert(f);
  assert(!fseek(f,0,SEEK_END)); *size=ftell(f); rewind(f);
  uint8_t *bytes=malloc(*size); assert(bytes);
  assert(fread(bytes,1,*size,f)==*size); fclose(f); return bytes;
}
int main(int argc, char **argv) {
  assert(argc==3);
  blob=load(argv[1],&blob_size);
  size_t raw_size;
  uint8_t *raw=load(argv[2],&raw_size);
  assert(pmr_open(&db,read_blob,NULL,blob_size));
  assert(db.raw_size==raw_size);
  char *out=malloc(raw_size+1); assert(out);
  // Irregular reads exercise every chunk boundary and the short final chunk.
  for (uint32_t i=0;i<raw_size;i+=997) {
    uint32_t n=raw_size-i; if(n>17003)n=17003;
    assert(pmr_read(&db,i,out,n)); assert(!memcmp(out,raw+i,n));
  }
  uint32_t longest=0, crossing=0;
  for (uint32_t i=0;i<db.segments;i++) {
    uint8_t *r=raw+db.segments_offset+i*8;
    uint32_t off=get32(r),n=get32(r+4),length=0;
    assert(pmr_segment(&db,i,out,raw_size+1,&length));
    assert(length==n && !memcmp(out,raw+off,n) && out[n]==0);
    assert(!pmr_segment(&db,i,out,n,NULL)); assert(out[0]==0);
    if(n>longest)longest=n;
    if(n && off/PMR_CHUNK!=(off+n-1)/PMR_CHUNK)crossing++;
  }
  for (uint32_t i=0;i<db.pages;i++) {
    uint8_t *p=raw+i*12;
    uint32_t source=get32(p),first=get32(p+4),count=get32(p+8);
    assert(pmr_reading(&db,source,0,count,out,raw_size+1));
    size_t at=0;
    for(uint32_t j=0;j<count;j++) {
      uint32_t id=get32(raw+db.refs_offset+(first+j)*4);
      uint8_t *r=raw+db.segments_offset+id*8;
      uint32_t off=get32(r),n=get32(r+4);
      if(j){assert(out[at++]=='\n');assert(out[at++]=='\n');}
      assert(!memcmp(out+at,raw+off,n));at+=n;
    }
    assert(out[at]==0);
    assert(!pmr_reading(&db,source,count,1,out,raw_size+1));
  }
  assert(!pmr_reading(&db,UINT32_MAX,0,1,out,raw_size+1));
  assert(!pmr_reading(&db,519,0,UINT32_MAX,out,raw_size+1));
  assert(!pmr_read(&db,UINT32_MAX,out,1));
  assert(!pmr_segment(&db,db.segments,out,raw_size+1,NULL));
  printf("Verified %u segments, %u complete pages; longest segment %u bytes; %u cross-chunk segments\n",db.segments,db.pages,longest,crossing);
  // Truncation, corrupt magic/header/index, checksum failure and short IO.
  assert(!pmr_open(&db,read_blob,NULL,12));
  blob[0]^=1;assert(!pmr_open(&db,read_blob,NULL,blob_size));blob[0]^=1;
  blob[36]=1;assert(!pmr_open(&db,read_blob,NULL,blob_size));blob[36]=0;
  uint8_t saved[8];memcpy(saved,blob+40,8);memset(blob+40,255,8);
  assert(pmr_open(&db,read_blob,NULL,blob_size));assert(!pmr_read(&db,0,out,1));memcpy(blob+40,saved,8);
  uint32_t offset=get32(blob+40),length=get32(blob+44);
  blob[offset+length-1]^=1;
  assert(pmr_open(&db,read_blob,NULL,blob_size));assert(!pmr_read(&db,0,out,1));
  blob[offset+length-1]^=1;
  assert(pmr_open(&db,read_blob,NULL,blob_size));
  size_t original=blob_size;blob_size=offset+length-1;assert(!pmr_read(&db,0,out,1));blob_size=original;
  free(out);free(raw);free(blob);
  puts("Bounds, capacity, corruption and short-read checks passed");
}

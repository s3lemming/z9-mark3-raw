#include "native_virtual_nrmap.h"
#include "native_auxiliary_gate.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
static _Alignas(64) uint8_t Y[0x300],data[0x22200+64],copied[0x22200+64];
static uint8_t beforeY[0x300],beforeData[sizeof data],descriptor[32];
static unsigned checks;
static uint16_t r16(const uint8_t*p){return p[0]|(uint16_t)p[1]<<8;}
static uint32_t r32(const uint8_t*p){return r16(p)|(uint32_t)r16(p+2)<<16;}
static uintptr_t ptr(const uint8_t*p){uintptr_t v=0;for(unsigned i=0;i<8;i++)v|=(uintptr_t)p[i]<<(8*i);return v;}
static void reset(void){memset(Y,0xa5,sizeof Y);Y[0x21]=0;memset(data,0xcc,sizeof data);memcpy(beforeY,Y,sizeof Y);memcpy(beforeData,data,sizeof data);}
static void unchanged(void){assert(!memcmp(Y,beforeY,sizeof Y)&&!memcmp(data,beforeData,sizeof data));checks++;}
int main(void){
 for(unsigned area=0;area<2;area++)for(unsigned v=0;v<2;v++){
  unsigned value=v?128:0;size_t expected=area?139200:98304;reset();
  assert(m3_virtual_nrmap_fill(Y,sizeof Y,area,data,0x22200,value)==expected);checks++;
  assert(Y[0x21]==1&&r32(Y+0x158)==11&&r32(Y+0x15c)==0);
  assert(r16(Y+0x160)==(area?480:384)&&r16(Y+0x162)==(area?290:256));
  assert(r16(Y+0x164)==(area?480:384)&&r16(Y+0x166)==384&&r16(Y+0x168)==256);
  assert(r16(Y+0x16a)==(area?96:0)&&r16(Y+0x16c)==(area?34:0)&&r16(Y+0x16e)==0);
  assert(ptr(Y+0x170)==(uintptr_t)data);checks++;
  for(size_t i=0;i<sizeof Y;i++)if(i!=0x21&&(i<0x158||i>=0x178))assert(Y[i]==beforeY[i]);
  for(size_t i=0;i<sizeof data;i++){assert(data[i]==(i<expected?value:0xcc));}
  checks++;
  memset(copied,0x33,sizeof copied);memset(descriptor,0x55,sizeof descriptor);
  assert(m3_auxiliary8_copy(Y,sizeof Y,area,descriptor,copied,0x22200)==expected);
  assert(!memcmp(data,copied,expected)&&ptr(descriptor+24)==(uintptr_t)copied);checks++;
  /* The last visible DX output sample sits at the end of its native canvas. */
  size_t last=((size_t)(area?34:0)+255)*(area?480:384)+(area?96:0)+383;
  assert(last<expected&&data[last]==value);checks++;
  memcpy(beforeY,Y,sizeof Y);memcpy(beforeData,data,sizeof data);
  assert(!m3_virtual_nrmap_fill(Y,sizeof Y,area,data,0x22200,value));unchanged();
 }
 reset();assert(!m3_virtual_nrmap_fill(0,sizeof Y,0,data,0x22200,0));unchanged();
 reset();assert(!m3_virtual_nrmap_fill(Y,sizeof Y,0,0,0x22200,0));unchanged();
 reset();assert(!m3_virtual_nrmap_fill(Y,0x2ff,0,data,0x22200,0));unchanged();
 reset();assert(!m3_virtual_nrmap_fill(Y,sizeof Y,2,data,0x22200,0));unchanged();
 reset();assert(!m3_virtual_nrmap_fill(Y,sizeof Y,0,data,98303,0));unchanged();
 reset();assert(!m3_virtual_nrmap_fill(Y,sizeof Y,1,data,139199,0));unchanged();
 reset();assert(!m3_virtual_nrmap_fill(Y,sizeof Y,0,data+1,0x22200,0));unchanged();
 reset();assert(!m3_virtual_nrmap_fill(Y,sizeof Y,0,Y,sizeof Y,0));unchanged();
 reset();assert(!m3_virtual_nrmap_fill(Y,sizeof Y,0,data,SIZE_MAX,0));unchanged();
 reset();assert(!m3_virtual_nrmap_fill((void*)(UINTPTR_MAX-64),sizeof Y,0,data,0x22200,0));unchanged();
 reset();assert(!m3_virtual_nrmap_fill(Y,sizeof Y,0,(void*)(UINTPTR_MAX-63),0x22200,0));unchanged();
 reset();assert(!m3_virtual_nrmap_fill(Y,sizeof Y,0,data,0x22200,1));unchanged();
 reset();assert(!m3_virtual_nrmap_fill(Y,sizeof Y,0,data,0x22200,255));unchanged();
 reset();Y[0x21]=2;memcpy(beforeY,Y,sizeof Y);assert(!m3_virtual_nrmap_fill(Y,sizeof Y,0,data,0x22200,0));unchanged();
 printf("VIRTUAL_NRMAP_CONTRACT %u checks passed\n",checks);return 0;
}

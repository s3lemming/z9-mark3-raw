/* Host-only policy matrix. Native duration conversion is an explicit fixture. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "native_precapture_limit.h"
static uint64_t choice;static uint32_t mode=3,area,photo=1,special,requested=3;
uint64_t m3_precapture_test_selection(void){return choice;}
uint32_t m3_precapture_test_applied_mode(void){return mode;}
static uint32_t get(uint32_t id){return id==0x02006401?requested:0x1234;}
static uint32_t cached(uint32_t id){switch(id){case 0x1d00:return photo;case 0x603:return area;case 0x1c26:return mode;case 0x1c8c:return special;default:abort();}}
static uint32_t frames(uint32_t value){unsigned rate=mode==3?30:15;return value==3?rate:value==2?rate/2:0;}
uintptr_t m3_precapture_test_function(uintptr_t at){switch(at){case 0x410db64c:return(uintptr_t)get;case 0x406a3f54:return(uintptr_t)cached;case 0x40f9d61c:return(uintptr_t)frames;default:abort();}}
int main(void){unsigned options[]={0,2,4,8,10},checks=0;
 for(unsigned m=0;m<2;m++)for(area=0;area<2;area++)for(unsigned codec=3;codec<=4;codec++)for(unsigned k=0;k<5;k++){
  mode=m?6:3;choice=((uint64_t)codec<<32)|options[k];requested=3;
  unsigned cap=mode==3&&!area&&codec==3&&k<3;
  assert(m3_precapture_property_get(0x02006401)==(cap?2:3));assert(requested==3);
  assert(m3_precapture_frames(3)==frames(cap?2:3));assert(m3_precapture_property_get(0x02006402)==0x1234);checks+=4;
 }
 mode=3;area=0;choice=(uint64_t)3<<32;special=1;assert(m3_precapture_property_get(0x02006401)==3);special=0;photo=0;assert(m3_precapture_property_get(0x02006401)==3);photo=1;requested=2;assert(m3_precapture_property_get(0x02006401)==2);
 printf("PRECAPTURE_POLICY %u matrix assertions +3 bypasses passed; conversion mocked\n",checks);return 0;
}

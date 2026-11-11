#include "native_shotinfo_group1723.h"
#ifndef FN
#define FN(a,t) ((t)(uintptr_t)(a))
#endif
/* All addresses below are runtime RAM, never inferred ROM initializers. */
static uint64_t v64(uintptr_t p){return *(const volatile uint64_t*)p;}
static uint32_t v32(uintptr_t p){return *(const volatile uint32_t*)p;}
unsigned m3_shotinfo_group1723_native_ready(const void*vp,unsigned mask){
 const uint8_t*m=vp;
 if(!(mask&1)||*(const uint32_t*)(m+0x24b8))return mask&3;
 /* Do not turn a retained-frame metadata read into a first-use allocation,
  * and do not temporarily redirect global registration pointers. */
 static const struct {uint64_t address;uint32_t offset;} pointers[]={
  {0xa034fff88ull,0xed1},{0xa034e9b30ull,0x3f9},{0xa034e9b38ull,0x43d},
  {0xa034e9b40ull,0x78d},{0xa034e9b48ull,0xc39},{0xa034e9b50ull,0xcd9},
  {0xa034e9b58ull,0xd99},{0xa03508d30ull,0x15e4},{0xa034c8a70ull,0x3f9},
  {0xa034c8a78ull,0xc19},{0xa03512f00ull,0x67},{0xa034c8eb8ull,0x1446},
  {0xa03508d58ull,0x1446},{0xa03518488ull,0x48}};
 for(unsigned i=0;i<sizeof pointers/sizeof pointers[0];i++)
  if(v64(pointers[i].address)!=0xa0349b050ull+pointers[i].offset)return mask&2;
 if(*(const volatile uint8_t*)(uintptr_t)0xa0349c790ull!=1||!v64(0xa0349c788ull)||
    v32(0xa034cfe34ull)>5||v32(0xa034cfe38ull)>5)return mask&2;
 return mask&3;
}
void m3_shotinfo_group1723_native_build(void*m,unsigned mask){
 /* The backend calls this exactly once while the E owner is SAMPLING.
  * M23's original timestamp/diagnostic ring and flag consumption remain live. */
 if(mask&1)(void)FN(0x40d79284,unsigned(*)(void*))(m);
 if(mask&2)FN(0x4108700c,void(*)(void*))(m);
}

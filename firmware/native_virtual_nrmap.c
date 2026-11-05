#include "native_virtual_nrmap.h"
static int span(uintptr_t p,size_t n){return p&&n&&p<=UINTPTR_MAX-n;}
static int overlap(uintptr_t p,size_t n,uintptr_t q,size_t m){return p<q+m&&q<p+n;}
static void w16(uint8_t*p,uint16_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
static void w32(uint8_t*p,uint32_t v){w16(p,(uint16_t)v);w16(p+2,(uint16_t)(v>>16));}
size_t m3_virtual_nrmap_fill(void *view,size_t n,uint32_t area,void *owned,size_t room,uint32_t value){
 uintptr_t yp=(uintptr_t)view,p=(uintptr_t)owned;
 if(n!=0x300u||area>1||(value!=0&&value!=128)||!span(yp,n)||
    !span(p,room)||(p&63u)||overlap(yp,n,p,room))return 0;
 uint8_t*Y=view;
 if(__atomic_load_n(Y+0x21,__ATOMIC_ACQUIRE))return 0;
 uint16_t width=area?480:384,height=area?290:256;
 size_t bytes=(size_t)width*height;
 if(room<bytes)return 0;
 /* Proved original4106640C kind11/format0 area0/1 canvas contract. The
  * visible384x256 crop is descriptor metadata; the original stream writer
  * serializes the full DX139200-byte canvas plus64-byte file padding. */
 uint8_t*d=Y+0x158,*out=owned;
 for(size_t i=0;i<bytes;i++)out[i]=(uint8_t)value;
 for(unsigned i=0;i<32;i++)d[i]=0;
 w32(d,11);w32(d+4,0);w16(d+8,width);w16(d+10,height);
 w16(d+12,width);w16(d+14,384);w16(d+16,256);
 w16(d+18,area?96:0);w16(d+20,area?34:0);
 for(unsigned i=0;i<8;i++)d[24+i]=(uint8_t)(p>>(8*i));
 __atomic_store_n(Y+0x21,1,__ATOMIC_RELEASE);
 return bytes;
}

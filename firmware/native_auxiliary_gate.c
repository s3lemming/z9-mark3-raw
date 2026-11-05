#include "native_auxiliary_gate.h"
#include "native_live_settings.h"
#include "native_runtime_layout.h"
static uint16_t r16(const uint8_t*p){return(uint16_t)(p[0]|(uint16_t)p[1]<<8);}
static uint32_t r32(const uint8_t*p){return r16(p)|(uint32_t)r16(p+2)<<16;}
static uint64_t r64(const uint8_t*p){return r32(p)|(uint64_t)r32(p+4)<<32;}
int m3_auxiliary_scope(uint32_t kind,uint32_t mode,uint32_t area,uint64_t s){
 if(kind!=3||(mode!=3&&mode!=6)||area>1||((s>>32)!=3&&(s>>32)!=4))return 0;
 uint32_t option=(uint32_t)s;return option==0||option==2||option==4;
}
uint32_t m3_auxiliary_native_scope(const void*ptr){
 if(!ptr)return 0;
 const uint8_t*M=ptr;
 uint64_t choice=__atomic_load_n(&((struct m3_live_settings*)(uintptr_t)
  M3_LIVE_SETTINGS_ADDRESS)->condition_selection,__ATOMIC_ACQUIRE);
 return(uint32_t)m3_auxiliary_scope(r32(M),r32(M+0x24a0),r32(M+0x26c),choice);
}
static int span(uintptr_t p,size_t n){return p&&n&&p<=UINTPTR_MAX-n;}
static int overlap(uintptr_t p,size_t n,uintptr_t q,size_t m){return !span(p,n)||!span(q,m)||(p<q+m&&q<p+n);}
size_t m3_auxiliary8_copy(const void*source,size_t n,uint32_t area,void*descriptor,void*payload,size_t room){
 const uint8_t*Y=source;
 if(!span((uintptr_t)Y,n)||n<0x178||area>1||!Y[0x21])return 0;
 const uint8_t*d=Y+0x158;const uint16_t w=area?480:384,h=area?290:256,x=area?96:0,y=area?34:0;
 /* Original4106640C initializer, original area0/1 tables, and native
  * descriptor byte-count/crop consumers. Kind11 is mono; format0 is8bit. */
 if(r32(d)!=11||r32(d+4)!=0||r16(d+8)!=w||r16(d+10)!=h||r16(d+12)!=w||r16(d+14)!=384||r16(d+16)!=256||r16(d+18)!=x||r16(d+20)!=y||r16(d+22))return 0;
 size_t bytes=(size_t)w*h;uintptr_t p=(uintptr_t)r64(d+24),out=(uintptr_t)payload,desc=(uintptr_t)descriptor;
 if(!span(p,bytes)||!span(out,room)||room<bytes||(out&63u)||!span(desc,32)||overlap(out,room,(uintptr_t)Y,n)||overlap(out,room,p,bytes)||overlap(desc,32,(uintptr_t)Y,n)||overlap(desc,32,p,bytes)||overlap(out,room,desc,32))return 0;
 volatile uint8_t*b=payload,*view=descriptor;
 for(size_t i=0;i<bytes;i++)b[i]=((const volatile uint8_t*)p)[i];
 for(unsigned i=0;i<32;i++)view[i]=d[i];
 for(unsigned i=0;i<8;i++)view[24+i]=(uint8_t)(out>>(i*8));
 return bytes;
}

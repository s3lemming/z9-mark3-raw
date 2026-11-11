#include "native_shotinfo_battery.h"
static uint32_t r32(const void*p){const uint8_t*b=p;return(uint32_t)b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16|(uint32_t)b[3]<<24;}
static uint64_t r64(const void*p){return r32(p)|(uint64_t)r32((const uint8_t*)p+4)<<32;}
static void w32(void*p,uint32_t x){uint8_t*b=p;for(unsigned i=0;i<4;i++)b[i]=(uint8_t)(x>>(8*i));}
static int target(const void*m,size_t n){
 if(!m||n!=M3_BATTERY_METADATA_BYTES||(uintptr_t)m>UINTPTR_MAX-n)return 0;
 const uint8_t*b=m;uint32_t mode=r32(b+0x24a0);
 return r32(b)==3&&(mode==3||mode==6);
}
int m3_shotinfo_battery_capture(struct m3_shotinfo_battery_sample*s,uint64_t generation,
 const void*m,size_t n,m3_shotinfo_battery_producer producer){
 if(!s)return 0;
 uint8_t*b=(void*)s;for(size_t i=0;i<sizeof *s;i++)b[i]=0;
 if(!generation||!producer||!target(m,n))return 0;
 const uint8_t*source=m;
 /* Keep any future native section authoritative; the original M is readonly. */
 if(r32(source+0x2a60)||!r64(source+0x2a68))return 0;
 uint32_t bytes=0;producer(s->payload,&bytes);
 if(bytes!=8)return 0;
 s->generation=generation;s->source_M=(uintptr_t)m;
 s->counter=r32(source+0x250c);s->sequence=r32(source+0x120);
 s->bytes=bytes;s->stage=M3_BATTERY_RETAIN_STAGE;return 1;
}
int m3_shotinfo_battery_publish(const struct m3_shotinfo_battery_sample*s,
 uint64_t generation,const void*m,size_t n,void*copy,size_t cn){
 if(!s||!generation||!target(m,n)||!target(copy,cn)||copy==m||
    s->stage!=M3_BATTERY_RETAIN_STAGE||s->bytes!=8||s->generation!=generation||
    s->source_M!=(uintptr_t)m)return 0;
 const uint8_t*source=m;uint8_t*dest=copy;
 if(s->counter!=r32(source+0x250c)||s->sequence!=r32(source+0x120)||
    r32(dest+0x250c)!=s->counter||r32(dest+0x120)!=s->sequence||
    r32(source+0x2a60)||r32(dest+0x2a60))return 0;
 uint64_t p=r64(dest+0x2a68),next=r64(dest+0x2a78),old=r64(source+0x2a68);
 /* Original allocator4104CDC0 gives slot26 exactly8B, followed by slot27.
  * Accept only that owned layout, never a source alias or an M-prefix span. */
 if(!p||!old||p==old||p>UINT64_MAX-8||next!=p+8||
    (p<(uintptr_t)m+n&&(uintptr_t)m<p+8)||
    (p<(uintptr_t)copy+cn&&(uintptr_t)copy<p+8))return 0;
 uint8_t*out=(void*)(uintptr_t)p;for(unsigned i=0;i<8;i++)out[i]=s->payload[i];
 w32(dest+0x2a60,s->bytes);return 1;
}

#include "native_shotinfo_section15.h"
static uint32_t r32(const void*p){const uint8_t*b=p;return(uint32_t)b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16|(uint32_t)b[3]<<24;}
static uint64_t r64(const void*p){return r32(p)|(uint64_t)r32((const uint8_t*)p+4)<<32;}
static void w32(void*p,uint32_t x){uint8_t*b=p;for(unsigned i=0;i<4;i++)b[i]=(uint8_t)(x>>(8*i));}
static int span(uintptr_t p,size_t n){return p&&p<=UINTPTR_MAX-n;}
static int overlap(uintptr_t p,size_t pn,uintptr_t q,size_t qn){return p<q+qn&&q<p+pn;}
static int target(const void*m,size_t n){
 if(n!=M3_SECTION15_METADATA_BYTES||!span((uintptr_t)m,n))return 0;
 const uint8_t*b=m;uint32_t mode=r32(b+0x24a0);
 return r32(b)==3&&(mode==3||mode==6);
}
int m3_shotinfo_section15_capture(struct m3_shotinfo_section15_sample*s,
 uint64_t generation,const void*m,size_t n,m3_shotinfo_section15_producer producer){
 if(!span((uintptr_t)s,sizeof *s)||!target(m,n)||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)m,n))return 0;
 const uint8_t*b=m;uintptr_t old=(uintptr_t)r64(b+0x29b8);
 if(!span(old,8)||overlap((uintptr_t)s,sizeof *s,old,8))return 0;
 uint8_t*z=(void*)s;for(size_t i=0;i<sizeof *s;i++)z[i]=0;
 if(!generation||!producer||r32(b+0x29b0))return 0;
 uint32_t counter=r32(b+0x250c),sequence=r32(b+0x120),bytes=0;
 uint8_t native[30];
 producer(native);
 for(unsigned i=0;i<8;i++)s->payload[i]=native[22+i];
 bytes=8;
 /* A native call can yield; never stamp a newly recycled M as the old frame. */
 if(bytes!=8||!target(m,n)||r32(b+0x29b0)||r64(b+0x29b8)!=old||
    r32(b+0x250c)!=counter||r32(b+0x120)!=sequence)return 0;
 s->generation=generation;s->source_M=(uintptr_t)m;
 s->counter=counter;s->sequence=sequence;s->bytes=8;
 s->stage=M3_SECTION15_RETAIN_STAGE;return 1;
}
int m3_shotinfo_section15_publish(const struct m3_shotinfo_section15_sample*s,
 uint64_t generation,const void*m,size_t n,void*copy,size_t cn){
 if(!span((uintptr_t)s,sizeof *s)||!generation||!target(m,n)||!target(copy,cn)||
    overlap((uintptr_t)m,n,(uintptr_t)copy,cn)||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)m,n)||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)copy,cn)||
    s->generation!=generation||s->source_M!=(uintptr_t)m||
    s->stage!=M3_SECTION15_RETAIN_STAGE||s->bytes!=8)return 0;
 const uint8_t*b=m;uint8_t*d=copy;
 if(s->counter!=r32(b+0x250c)||s->sequence!=r32(b+0x120)||
    s->counter!=r32(d+0x250c)||s->sequence!=r32(d+0x120)||
    r32(b+0x29b0)||r32(d+0x29b0))return 0;
 uintptr_t p=(uintptr_t)r64(d+0x29b8),old=(uintptr_t)r64(b+0x29b8);
 /* Native4104CDC0 gives slot15 exactly8B before slot16. */
 if(!span(p,8)||!span(old,8)||r64(d+0x29c8)!=(uint64_t)p+8||
    overlap(p,8,old,8)||overlap(p,8,(uintptr_t)m,n)||
    overlap(p,8,(uintptr_t)copy,cn)||overlap(p,8,(uintptr_t)s,sizeof *s)||
    overlap(old,8,(uintptr_t)s,sizeof *s))return 0;
 uint8_t*out=(void*)p;for(unsigned i=0;i<8;i++)out[i]=s->payload[i];
 w32(d+0x29b0,8);return 1;
}

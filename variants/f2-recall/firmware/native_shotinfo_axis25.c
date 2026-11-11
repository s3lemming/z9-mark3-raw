#include "native_shotinfo_axis25.h"
static uint32_t r32(const void*p){const uint8_t*b=p;return(uint32_t)b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16|(uint32_t)b[3]<<24;}
static uint64_t r64(const void*p){return r32(p)|(uint64_t)r32((const uint8_t*)p+4)<<32;}
static void w32(void*p,uint32_t x){uint8_t*b=p;for(unsigned i=0;i<4;i++)b[i]=(uint8_t)(x>>(8*i));}
static int span(uintptr_t p,size_t n){return p&&p<=UINTPTR_MAX-n;}
static int overlap(uintptr_t p,size_t pn,uintptr_t q,size_t qn){return p<q+qn&&q<p+pn;}
static int target(const void*m,size_t n){
 if(n!=M3_AXIS25_METADATA_BYTES||!span((uintptr_t)m,n))return 0;
 const uint8_t*b=m;uint32_t mode=r32(b+0x24a0);
 return r32(b)==3&&(mode==3||mode==6);
}
int m3_shotinfo_axis25_capture(struct m3_shotinfo_axis25_sample*s,
 uint64_t generation,const void*m,size_t n,m3_shotinfo_axis25_producer producer){
 if(!span((uintptr_t)s,sizeof *s)||!target(m,n)||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)m,n))return 0;
 const uint8_t*b=m;uintptr_t old=(uintptr_t)r64(b+0x2a58);
 if(!span(old,108)||overlap((uintptr_t)s,sizeof *s,old,108))return 0;
 uint8_t*z=(void*)s;for(size_t i=0;i<sizeof *s;i++)z[i]=0;
 if(!generation||!producer||r32(b+0x2a50))return 0;
 uint32_t counter=r32(b+0x250c),sequence=r32(b+0x120),bytes=0;
 producer(s->payload,&bytes);
 /* A native call can yield; never stamp a newly recycled M as the old frame. */
 if(bytes!=108||!target(m,n)||r32(b+0x2a50)||r64(b+0x2a58)!=old||
    r32(b+0x250c)!=counter||r32(b+0x120)!=sequence)return 0;
 s->generation=generation;s->source_M=(uintptr_t)m;
 s->counter=counter;s->sequence=sequence;s->bytes=108;
 s->stage=M3_AXIS25_RETAIN_STAGE;return 1;
}
int m3_shotinfo_axis25_publish(const struct m3_shotinfo_axis25_sample*s,
 uint64_t generation,const void*m,size_t n,void*copy,size_t cn){
 if(!span((uintptr_t)s,sizeof *s)||!generation||!target(m,n)||!target(copy,cn)||
    overlap((uintptr_t)m,n,(uintptr_t)copy,cn)||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)m,n)||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)copy,cn)||
    s->generation!=generation||s->source_M!=(uintptr_t)m||
    s->stage!=M3_AXIS25_RETAIN_STAGE||s->bytes!=108)return 0;
 const uint8_t*b=m;uint8_t*d=copy;
 if(s->counter!=r32(b+0x250c)||s->sequence!=r32(b+0x120)||
    s->counter!=r32(d+0x250c)||s->sequence!=r32(d+0x120)||
    r32(b+0x2a50)||r32(d+0x2a50))return 0;
 uintptr_t p=(uintptr_t)r64(d+0x2a58),old=(uintptr_t)r64(b+0x2a58);
 /* Native4104CDC0 gives slot25 exactly108B before slot26. */
 if(!span(p,108)||!span(old,108)||r64(d+0x2a68)!=(uint64_t)p+108||
    overlap(p,108,old,108)||overlap(p,108,(uintptr_t)m,n)||
    overlap(p,108,(uintptr_t)copy,cn)||overlap(p,108,(uintptr_t)s,sizeof *s)||
    overlap(old,108,(uintptr_t)s,sizeof *s))return 0;
 uint8_t*out=(void*)p;for(unsigned i=0;i<108;i++)out[i]=s->payload[i];
 w32(d+0x2a50,108);return 1;
}

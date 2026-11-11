#include "native_shotinfo_bank12.h"
static uint32_t r32(const void*p){const uint8_t*b=p;return(uint32_t)b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16|(uint32_t)b[3]<<24;}
static uint64_t r64(const void*p){return r32(p)|(uint64_t)r32((const uint8_t*)p+4)<<32;}
static void w32(void*p,uint32_t x){uint8_t*b=p;for(unsigned i=0;i<4;i++)b[i]=(uint8_t)(x>>(8*i));}
static int span(uintptr_t p,size_t n){return p&&p<=UINTPTR_MAX-n;}
static int overlap(uintptr_t p,size_t pn,uintptr_t q,size_t qn){return p<q+qn&&q<p+pn;}
static int target(const void*m,size_t n){
 if(n!=M3_BANK12_METADATA_BYTES||!span((uintptr_t)m,n))return 0;
 const uint8_t*b=m;uint32_t mode=r32(b+0x24a0);
 return r32(b)==3&&(mode==3||mode==6);
}
int m3_shotinfo_bank12_capture(struct m3_shotinfo_bank12_sample*s,
 uint64_t generation,const void*m,size_t n,const volatile void*descriptor,
 const struct m3_bank_mutex_ops*mutex){
 if(!span((uintptr_t)s,sizeof *s)||!target(m,n)||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)m,n))return 0;
 const uint8_t*source=m;
 if(!generation||r32(source+0x2980)||!r64(source+0x2988))return 0;
 uintptr_t old=(uintptr_t)r64(source+0x2988);
 if(!span(old,800)||overlap((uintptr_t)s,sizeof *s,old,800))return 0;
 if(m3_bank_snapshot_record(descriptor,32,800,s,sizeof *s,
    offsetof(struct m3_shotinfo_bank12_sample,payload),mutex)!=M3_BANK_COPIED)return 0;
 s->generation=generation;s->source_M=(uintptr_t)m;
 s->counter=r32(source+0x250c);s->sequence=r32(source+0x120);
 s->bytes=800;s->stage=M3_BANK12_RETAIN_STAGE;return 1;
}
int m3_shotinfo_bank12_publish(const struct m3_shotinfo_bank12_sample*s,
 uint64_t generation,const void*m,size_t n,void*copy,size_t cn){
 if(!span((uintptr_t)s,sizeof *s)||!generation||!target(m,n)||!target(copy,cn)||
    overlap((uintptr_t)m,n,(uintptr_t)copy,cn)||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)m,n)||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)copy,cn)||
    s->stage!=M3_BANK12_RETAIN_STAGE||s->bytes!=800||s->generation!=generation||
    s->source_M!=(uintptr_t)m)return 0;
 const uint8_t*source=m;uint8_t*dest=copy;
 if(s->counter!=r32(source+0x250c)||s->sequence!=r32(source+0x120)||
    r32(dest+0x250c)!=s->counter||r32(dest+0x120)!=s->sequence||
    r32(source+0x2980)||r32(dest+0x2980))return 0;
 uintptr_t p=(uintptr_t)r64(dest+0x2988),old=(uintptr_t)r64(source+0x2988);
 /* The existing native M allocator reserves exactly800B before slot13.
  * Only the private allocator's owned nonaliasing range is a destination. */
 if(!span(p,800)||!span(old,800)||overlap(p,800,old,800)||
    overlap((uintptr_t)s,sizeof *s,old,800)||r64(dest+0x2998)!=(uint64_t)p+800||
    overlap(p,800,(uintptr_t)m,n)||overlap(p,800,(uintptr_t)copy,cn)||
    overlap(p,800,(uintptr_t)s,sizeof *s))return 0;
 uint8_t*out=(void*)p;for(unsigned i=0;i<800;i++)out[i]=s->payload[i];
 w32(dest+0x2980,800);return 1;
}

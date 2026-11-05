#include "native_shotinfo_group1820.h"
static uint32_t r32(const void*p){const uint8_t*b=p;return(uint32_t)b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16|(uint32_t)b[3]<<24;}
static uint64_t r64(const void*p){return r32(p)|(uint64_t)r32((const uint8_t*)p+4)<<32;}
static void w32(void*p,uint32_t x){uint8_t*b=p;for(unsigned i=0;i<4;i++)b[i]=(uint8_t)(x>>(8*i));}
static void w64(void*p,uint64_t x){w32(p,(uint32_t)x);w32((uint8_t*)p+4,(uint32_t)(x>>32));}
static int span(uintptr_t p,size_t n){return p&&p<=UINTPTR_MAX-n;}
static int overlap(uintptr_t p,size_t pn,uintptr_t q,size_t qn){return p<q+qn&&q<p+pn;}
static int target(const void*m,size_t n){
 if(n!=M3_GROUP1820_METADATA_BYTES||!span((uintptr_t)m,n))return 0;
 const uint8_t*b=m;uint32_t mode=r32(b+0x24a0);
 return r32(b)==3&&(mode==3||mode==6);
}
static unsigned size_for(unsigned i){return i==0?500:i==1?160:464;}
static uint8_t*payload(struct m3_shotinfo_group1820_sample*s,unsigned i){return i==0?s->payload18:i==1?s->payload19:s->payload20;}
int m3_shotinfo_group1820_capture(struct m3_shotinfo_group1820_sample*s,
 uint64_t generation,const void*m,size_t n,m3_shotinfo_group1820_producer producer){
 if(!span((uintptr_t)s,sizeof *s)||!target(m,n)||!generation||!producer||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)m,n))return 0;
 const uint8_t*b=m;uint32_t mask=0;
 for(unsigned i=0;i<3;i++){
  unsigned off=0x29e0+i*16;uintptr_t p=(uintptr_t)r64(b+off+8);
  if(span(p,size_for(i))&&overlap((uintptr_t)s,sizeof *s,p,size_for(i)))return 0;
  if(r32(b+off)||!span(p,size_for(i)))continue;
  mask|=1u<<i;
 }
 uint8_t*z=(void*)s;for(size_t i=0;i<sizeof *s;i++)z[i]=0;
 if(!mask)return 0;
 uint8_t*shadow=s->shadow_M;for(unsigned i=0;i<n;i++)shadow[i]=b[i];
 uint8_t descriptors[4][16];
 for(unsigned i=0;i<4;i++){
  unsigned off=i==3?0x29b0:0x29e0+i*16;
  for(unsigned j=0;j<16;j++)descriptors[i][j]=shadow[off+j];
  w64(shadow+off+8,i<3&&(mask&(1u<<i))?(uintptr_t)payload(s,i):0);
 }
 uint32_t counter=r32(b+0x250c),sequence=r32(b+0x120);
 producer(shadow);
 int valid=1;
 for(unsigned i=0;i<3;i++)if((mask&(1u<<i))&&r32(shadow+0x29e0+i*16)!=size_for(i))valid=0;
 /* Restore only the local shadow descriptors, never shared source state. */
 for(unsigned i=0;i<4;i++){
  unsigned off=i==3?0x29b0:0x29e0+i*16;
  for(unsigned j=0;j<16;j++)shadow[off+j]=descriptors[i][j];
 }
 if(!valid||!target(m,n))return 0;
 for(unsigned i=0;i<n;i++)if(shadow[i]!=b[i])return 0;
 s->generation=generation;s->source_M=(uintptr_t)m;s->counter=counter;
 s->sequence=sequence;s->mask=mask;s->stage=M3_GROUP1820_RETAIN_STAGE;return 1;
}
int m3_shotinfo_group1820_publish(const struct m3_shotinfo_group1820_sample*s,
 uint64_t generation,const void*m,size_t n,void*copy,size_t cn){
 if(!span((uintptr_t)s,sizeof *s)||!generation||!target(m,n)||!target(copy,cn)||
    overlap((uintptr_t)m,n,(uintptr_t)copy,cn)||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)m,n)||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)copy,cn)||
    s->generation!=generation||s->source_M!=(uintptr_t)m||
    s->stage!=M3_GROUP1820_RETAIN_STAGE||!s->mask||(s->mask&~7u))return 0;
 const uint8_t*b=m;uint8_t*d=copy;uintptr_t destinations[3]={0};
 if(s->counter!=r32(b+0x250c)||s->sequence!=r32(b+0x120)||
    s->counter!=r32(d+0x250c)||s->sequence!=r32(d+0x120))return 0;
 for(unsigned i=0;i<3;i++)if(s->mask&(1u<<i)){
  unsigned off=0x29e0+i*16,bytes=size_for(i);uintptr_t p=(uintptr_t)r64(d+off+8),old=(uintptr_t)r64(b+off+8);
  if(r32(b+off)||r32(d+off)||old!=r64(s->shadow_M+off+8)||!span(p,bytes)||!span(old,bytes)||
     r64(d+off+24)!=(uint64_t)p+bytes||overlap(p,bytes,(uintptr_t)m,n)||
     overlap(p,bytes,(uintptr_t)copy,cn)||overlap(p,bytes,(uintptr_t)s,sizeof *s))return 0;
  for(unsigned j=0;j<3;j++){
   uintptr_t q=(uintptr_t)r64(b+0x29e8+j*16);unsigned qn=size_for(j);
   if(span(q,qn)&&overlap(p,bytes,q,qn))return 0;
   uintptr_t dq=(uintptr_t)r64(d+0x29e8+j*16);
   if(j!=i&&span(dq,qn)&&overlap(p,bytes,dq,qn))return 0;
  }
  destinations[i]=p;
 }
 /* Every selected destination is checked before any payload/length write. */
 for(unsigned i=0;i<3;i++)if(s->mask&(1u<<i)){
  const uint8_t*in=i==0?s->payload18:i==1?s->payload19:s->payload20;
  uint8_t*out=(void*)destinations[i];for(unsigned j=0;j<size_for(i);j++)out[j]=in[j];
  w32(d+0x29e0+i*16,size_for(i));
 }
 return 1;
}

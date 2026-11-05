#include "native_shotinfo_group1723.h"
static uint32_t r32(const void*p){const uint8_t*b=p;return(uint32_t)b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16|(uint32_t)b[3]<<24;}
static uint64_t r64(const void*p){return r32(p)|(uint64_t)r32((const uint8_t*)p+4)<<32;}
static void w32(void*p,uint32_t x){uint8_t*b=p;for(unsigned i=0;i<4;i++)b[i]=(uint8_t)(x>>(8*i));}
static void w64(void*p,uint64_t x){w32(p,(uint32_t)x);w32((uint8_t*)p+4,(uint32_t)(x>>32));}
static int span(uintptr_t p,size_t n){return p&&p<=UINTPTR_MAX-n;}
static int overlap(uintptr_t p,size_t pn,uintptr_t q,size_t qn){return p<q+qn&&q<p+pn;}
static unsigned off(unsigned i){return i?0x2a30:0x29d0;}
static unsigned capacity(unsigned i){return i?48:5632;}
static unsigned bytes(const void*m,unsigned i){return i?48:r32((const uint8_t*)m+0x24b8)?4:5632;}
static uint8_t*payload(struct m3_shotinfo_group1723_sample*s,unsigned i){return i?s->payload23:s->payload17;}
static int target(const void*m,size_t n){
 if(n!=M3_GROUP1723_METADATA_BYTES||!span((uintptr_t)m,n))return 0;
 const uint8_t*b=m;uint32_t mode=r32(b+0x24a0);return r32(b)==3&&(mode==3||mode==6);
}
int m3_shotinfo_group1723_capture(struct m3_shotinfo_group1723_sample*s,
 uint64_t generation,const void*m,size_t n,m3_shotinfo_group1723_ready ready,
 m3_shotinfo_group1723_producer producer){
 if(!span((uintptr_t)s,sizeof *s)||!target(m,n)||!generation||!ready||!producer||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)m,n)||s->stage)return 0;
 const uint8_t*b=m;unsigned mask=0;
 for(unsigned i=0;i<2;i++){
  uintptr_t p=(uintptr_t)r64(b+off(i)+8);
  if(span(p,capacity(i))&&overlap((uintptr_t)s,sizeof *s,p,capacity(i)))return 0;
  if(!r32(b+off(i))&&span(p,capacity(i)))mask|=1u<<i;
 }
 mask&=ready(m,mask)&3u;if(!mask)return 0;
 /* Stage3 is a one-attempt latch, including a rejected or cancelled build.
  * M23 consumes native pending flags; replaying capture must never consume twice. */
 s->stage=3;
 uint8_t*shadow=s->shadow_M;for(unsigned i=0;i<n;i++)shadow[i]=b[i];
 uint8_t descriptors[2][16];
 for(unsigned i=0;i<2;i++){
  for(unsigned j=0;j<16;j++)descriptors[i][j]=shadow[off(i)+j];
  w64(shadow+off(i)+8,mask&(1u<<i)?(uintptr_t)payload(s,i):0);
 }
 uint32_t counter=r32(b+0x250c),sequence=r32(b+0x120);
 producer(shadow,mask);
 int valid=1;
 for(unsigned i=0;i<2;i++){
  if((mask&(1u<<i))&&r32(shadow+off(i))!=bytes(b,i))valid=0;
  for(unsigned j=0;j<16;j++)shadow[off(i)+j]=descriptors[i][j];
 }
 if(!valid||!target(m,n))return 0;
 for(unsigned i=0;i<n;i++)if(shadow[i]!=b[i])return 0;
 s->generation=generation;s->source_M=(uintptr_t)m;s->counter=counter;
 s->sequence=sequence;s->mask=mask;s->stage=M3_GROUP1723_RETAIN_STAGE;return 1;
}
int m3_shotinfo_group1723_publish(const struct m3_shotinfo_group1723_sample*s,
 uint64_t generation,const void*m,size_t n,void*copy,size_t cn){
 if(!span((uintptr_t)s,sizeof *s)||!generation||!target(m,n)||!target(copy,cn)||
    overlap((uintptr_t)m,n,(uintptr_t)copy,cn)||overlap((uintptr_t)s,sizeof *s,(uintptr_t)m,n)||
    overlap((uintptr_t)s,sizeof *s,(uintptr_t)copy,cn)||s->generation!=generation||
    s->source_M!=(uintptr_t)m||s->stage!=M3_GROUP1723_RETAIN_STAGE||!s->mask||(s->mask&~3u))return 0;
 const uint8_t*b=m;uint8_t*d=copy;uintptr_t destinations[2]={0};
 if(s->counter!=r32(b+0x250c)||s->sequence!=r32(b+0x120)||s->counter!=r32(d+0x250c)||
    s->sequence!=r32(d+0x120)||r32(b+0x24b8)!=r32(s->shadow_M+0x24b8)||
    r32(d+0x24b8)!=r32(b+0x24b8))return 0;
 for(unsigned i=0;i<2;i++)if(s->mask&(1u<<i)){
  uintptr_t p=(uintptr_t)r64(d+off(i)+8),old=(uintptr_t)r64(b+off(i)+8);unsigned cap=capacity(i);
  if(r32(b+off(i))||r32(d+off(i))||old!=r64(s->shadow_M+off(i)+8)||!span(p,cap)||!span(old,cap)||
     r64(d+off(i)+24)!=(uint64_t)p+cap||overlap(p,cap,(uintptr_t)m,n)||
     overlap(p,cap,(uintptr_t)copy,cn)||overlap(p,cap,(uintptr_t)s,sizeof *s))return 0;
  for(unsigned j=0;j<2;j++){
   uintptr_t q=(uintptr_t)r64(b+off(j)+8),dq=(uintptr_t)r64(d+off(j)+8);
   if(span(q,capacity(j))&&overlap(p,cap,q,capacity(j)))return 0;
   if(j!=i&&span(dq,capacity(j))&&overlap(p,cap,dq,capacity(j)))return 0;
  }
  destinations[i]=p;
 }
 /* Validate the entire selected group before changing any private output. */
 for(unsigned i=0;i<2;i++)if(s->mask&(1u<<i)){
  const uint8_t*in=i?s->payload23:s->payload17;uint8_t*out=(void*)destinations[i];
  for(unsigned j=0;j<bytes(b,i);j++)out[j]=in[j];
  w32(d+off(i),bytes(b,i));
 }
 return 1;
}

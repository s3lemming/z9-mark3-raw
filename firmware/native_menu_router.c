#include "native_menu_router.h"
#include <stddef.h>
static uint32_t r16(const uint8_t*p){return p[0]|(uint32_t)p[1]<<8;}
static uint32_t r32(const uint8_t*p){return r16(p)|r16(p+2)<<16;}
static uint64_t r64(const uint8_t*p){return r32(p)|(uint64_t)r32(p+4)<<32;}
static void w64(uint8_t*p,uint64_t v){for(unsigned i=0;i<8;i++)p[i]=(uint8_t)(v>>(8*i));}
static int activate(uint32_t e){return e==0x298||e==0x29d||e==0x2b1||e==0x2c0;}
static int span(const void*p,size_t n){return p&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int disjoint(const void*a,size_t an,const void*b,size_t bn){return(uintptr_t)a+an<=(uintptr_t)b||(uintptr_t)b+bn<=(uintptr_t)a;}
static int shape(const struct m3_native_menu_router*r){
 if(!r||r->magic!=0x33525452||!r->pages||r->pages->magic!=0x334d5047||r->pages->version!=1)return 0;
 if(r->c15_state<0x6c4||r->c15_state>65535||r->c60_state<0x6c4||r->c60_state>65535||r->c15_state==r->c60_state)return 0;
 const struct m3_native_pages*p=r->pages;
 const uint32_t ids[3]={0x2c,r->c15_state,r->c60_state},counts[3]={2,7,2};
 for(unsigned i=0;i<3;i++)if(r32(p->states[i])!=(ids[i]<<16|2u)||r64(p->states[i]+0x50)!=(uintptr_t)p->holders[i]||r64(p->holders[i]+0x10)!=(uintptr_t)p->rows[i]||r64(p->holders[i]+0x18)!=(uintptr_t)p->event_pointers[i]||r16(p->holders[i]+0x20)!=counts[i])return 0;
 return 1;
}
static unsigned private_leaf(const struct m3_native_menu_router*r,uint32_t slot,uint32_t id){
 if(!r||slot!=2)return 0;
 if(r->c15_state>=0x6c4&&r->c15_state<=65535&&id==r->c15_state)return 1;
 if(r->c60_state>=0x6c4&&r->c60_state<=65535&&id==r->c60_state)return 2;
 return 0;
}
enum m3_route_result m3_native_router_bind(struct m3_native_menu_router*r,
 struct m3_native_pages*p,const void*parent,uint32_t c15,uint32_t c60,
 void*(*lookup)(uint32_t,uint32_t),void(*enter)(struct m3_native_menu_event*),
 void(*construct)(uint32_t,uint32_t),int(*condition)(struct m3_native_menu_event*),uint64_t action,uint64_t condition_action){
 if(!span(r,sizeof *r)||!span(p,sizeof *p)||(uintptr_t)r%_Alignof(struct m3_native_menu_router)||(uintptr_t)p%_Alignof(struct m3_native_pages)||!disjoint(r,sizeof *r,p,sizeof *p)||!parent||!lookup||!enter||!construct||!condition||!action||!condition_action||c15<0x6c4||c15>65535||c60<0x6c4||c60>65535||c15==c60)return M3_ROUTE_INVALID;
 const uint8_t*old=(const void*)r;
 for(size_t i=0;i<sizeof *r;i++)if(old[i])return M3_ROUTE_BUSY;
 struct m3_native_menu_router next={.pages=p,.original_parent=parent,.c15_state=c15,.c60_state=c60,.magic=0x33525452,.original_lookup=lookup,.original_enter=enter,.construct=construct,.original_condition=condition};
 if(!shape(&next))return M3_ROUTE_INVALID;
 for(unsigned i=0;i<3;i++)if(r64(p->states[i]+0x48))return M3_ROUTE_BUSY;
 for(unsigned table=0;table<2;table++){
  if(p->event_pointers[0][table]!=(uintptr_t)p->events[table])return M3_ROUTE_INVALID;
  unsigned seen=0;
  for(unsigned j=0;j<16;j++){
   const uint8_t*e=p->events[table][j];uint32_t id=r32(e);
   if(r64(e)!=id||(id==0x3ca)!=(j==15))return M3_ROUTE_INVALID;
   if(activate(id)){
    unsigned bit=id==0x298?1u:id==0x29d?2u:id==0x2b1?4u:8u;
    if((seen&bit)||r64(e+8)!=0x403b20c0||r64(e+16)!=(id==0x2b1?0x403b217c:0)||r64(e+24)!=(table?c60:c15))return M3_ROUTE_INVALID;
    seen|=bit;
   }
  }
  if(seen!=15)return M3_ROUTE_INVALID;
 }
 /* No fallible operation after validation. Publication is caller-owned. */
 for(unsigned table=0;table<2;table++)for(unsigned j=0;j<16;j++)if(activate(r32(p->events[table][j]))){w64(p->events[table][j]+8,action);w64(p->events[table][j]+16,condition_action);}
 /* Keep freestanding binding independent of a libc struct-copy lowering,
  * including firmware builds which reserve platform register X18. Caller
  * still owns publication; enabled remains zero throughout construction. */
 const uint8_t*in=(const void*)&next;volatile uint8_t*out=(volatile void*)r;
 for(size_t i=0;i<sizeof next;i++)out[i]=in[i];
 return M3_ROUTE_OK;
}
void *m3_native_router_lookup(const struct m3_native_menu_router*r,
 uint32_t slot,uint32_t index,const void*previous){
 if(!r||!r->original_lookup)return NULL;
 uint32_t id=index&65535;
 unsigned leaf=private_leaf(r,slot,id);
 if(leaf){
  if(r->enabled!=1||!shape(r))return NULL;
  struct m3_native_pages*p=r->pages;
  if(previous==p->states[0]||previous==p->states[1]||previous==p->states[2])return p->states[leaf];
  /* A rejected private ID must never reach the original unbounded table
   * arithmetic. Disabled/invalid context is absence, not native fallback. */
  return NULL;
 }
 if(r->enabled==1&&slot==2&&shape(r)){
  struct m3_native_pages*p=r->pages;
  if(id==0x2c&&(previous==r->original_parent||previous==p->states[0]||previous==p->states[1]||previous==p->states[2]))return p->states[0];
 }
 /* Preserve the original full argument values/masking for every other call. */
 return r->original_lookup(slot,index);
}
void *m3_native_router_global_lookup(const struct m3_native_menu_router*r,uint32_t slot,uint32_t index){
 if(!r||!r->original_lookup)return NULL;
 unsigned leaf=private_leaf(r,slot,index&65535);
 if(leaf)return r->enabled==1&&shape(r)?r->pages->states[leaf]:NULL;
 /* Ordinary2C remains native. Its group replacement is constructor-context
  * specific; history consumers need only resolve the two owned leaf IDs. */
 return r->original_lookup(slot,index);
}
enum m3_route_result m3_native_router_open(const struct m3_native_menu_router*r,
 struct m3_native_menu_event*e){
 if(!r||!r->pages||!e||e->context!=r->pages->states[0])return M3_ROUTE_INVALID;
 uint32_t target=e->target;e->target=0x6b8;
 if(!shape(r)||!r->original_enter||!r->construct||!activate(e->event)||!r64(r->pages->states[0]+0x48))return M3_ROUTE_INVALID;
 if(r->enabled!=1)return M3_ROUTE_DISABLED;
 const uint8_t*ctx=e->context;uint32_t row=r16(ctx+0x32)+r16(ctx+0x58);
 if(row>=2||target!=(row?r->c60_state:r->c15_state))return M3_ROUTE_INVALID;
 /* Original403B20C0 is a RET. Keep the original action-before-navigation
  * order explicit; dispatcher sees6B8 afterwards and performs no second nav. */
 r->original_enter(e);e->target=0x6b8;
 r->construct(2,target);return M3_ROUTE_OK;
}
int m3_native_router_condition(const struct m3_native_menu_router*r,struct m3_native_menu_event*e){
 if(!r||!r->pages||!e||e->context!=r->pages->states[0])return 0;
 const uint8_t*ctx=e->context;uint32_t row=r16(ctx+0x32)+r16(ctx+0x58);
 int valid=shape(r)&&r->enabled==1&&r->original_enter&&r->construct&&r->original_condition&&activate(e->event)&&r64(r->pages->states[0]+0x48)&&row<2&&e->target==(row?r->c60_state:r->c15_state);
 if(valid&&e->event==0x2b1)valid=(r->original_condition(e)&1)!=0;
 if(!valid)e->target=0x6b8;
 return valid;
}

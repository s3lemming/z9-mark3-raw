#include "native_menu_pages.h"
#include "native_menu_labels.h"
#include <stddef.h>
#include "native_menu_headers.inc"
_Static_assert(sizeof(uintptr_t)==8,"Z9 native pointers are64bit");
_Static_assert(sizeof(struct m3_native_pages)<=4096,"Owned menu page exceeds reserved4KiB");
static uint32_t u16(const uint8_t*p){return p[0]|(uint32_t)p[1]<<8;}
static uint32_t u32(const uint8_t*p){return u16(p)|u16(p+2)<<16;}
static uint64_t u64(const uint8_t*p){return u32(p)|(uint64_t)u32(p+4)<<32;}
static void w16(uint8_t*p,uint32_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
static void w32(uint8_t*p,uint32_t v){w16(p,v);w16(p+2,v>>16);}
static void w64(uint8_t*p,uint64_t v){w32(p,(uint32_t)v);w32(p+4,(uint32_t)(v>>32));}
static void copy(uint8_t*d,const uint8_t*s,size_t n){for(size_t i=0;i<n;i++)d[i]=s[i];}
static int span(const void*p,size_t n){return p&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int overlaps(const void*a,size_t an,const void*b,size_t bn){return(uintptr_t)a<(uintptr_t)b+bn&&(uintptr_t)b<(uintptr_t)a+an;}
static int activate(uint32_t e){return e==0x298||e==0x29d||e==0x2b1||e==0x2c0;}
static int templates_valid(const struct m3_native_page_templates*t){
 if(u32(t->group_state)!=0x2a0002||u32(t->leaf_state)!=0x2c0002||
    u64(t->group_state+8)!=0xa028fca10ull||u64(t->leaf_state+8)!=0xa028fca10ull||
    u64(t->group_state+0x50)!=0xa028d44e0ull||u64(t->leaf_state+0x50)!=0xa028d4580ull||
    u64(t->group_holder+8)!=0x431f6078ull||u64(t->leaf_holder+8)!=0x431f6720ull||
    u16(t->group_holder+0x20)!=2||u16(t->leaf_holder+0x20)!=2)return 0;
 const uint32_t group_UI[2]={0x29c8,0x2dc8},leaf_UI[2]={0x2dc9,0x2dca};
 for(unsigned i=0;i<2;i++)if(u32(t->group_rows[i]+24)!=group_UI[i]||u32(t->leaf_rows[i]+24)!=leaf_UI[i]||u32(t->leaf_rows[i])!=0x308+i)return 0;
 for(unsigned table=0;table<3;table++){
  unsigned n=table<2?16:15;if(t->event_counts[table]!=n)return 0;
  unsigned activates=0,aux=0;
  for(unsigned j=0;j<n;j++){
   const uint8_t*e=t->events[table][j];uint32_t event=u32(e);
   if(u64(e)!=event||(event==0x3ca)!=(j==n-1))return 0;
   if(table<2&&activate(event)){
    if(u64(e+8)!=0x403b20c0||u64(e+24)!=(table?0x2c:0x2b))return 0;
    activates++;
   }else if(table==2&&(event==0x298||event==0x29d||event==0x2c0)){
    if(u64(e+8)!=0x405e5ce0||u64(e+24)!=0x6b9)return 0;
    activates++;
   }
   if(table<2&&event==0x315){if(u64(e+8)!=0x405e5ca4)return 0;aux++;}
  }
  if(activates!=(table<2?4u:3u)||(table<2&&aux!=1))return 0;
 }
 return 1;
}
enum m3_page_result m3_native_pages_build(struct m3_native_pages*out,
 const struct m3_native_page_templates*t,const struct m3_native_page_bindings*b){
 if(!span(out,sizeof *out)||!span(t,sizeof *t)||!span(b,sizeof *b)||
    (uintptr_t)out%_Alignof(struct m3_native_pages)||
    (uintptr_t)t%_Alignof(struct m3_native_page_templates)||
    (uintptr_t)b%_Alignof(struct m3_native_page_bindings)||
    overlaps(out,sizeof *out,t,sizeof *t)||overlaps(out,sizeof *out,b,sizeof *b))return M3_PAGE_INVALID;
 if(b->c15_state<0x6c4||b->c15_state>65535||b->c60_state<0x6c4||b->c60_state>65535||b->c15_state==b->c60_state||!b->group_aux_action||!b->c15_submit)return M3_PAGE_INVALID;
 for(unsigned i=0;i<5;i++)if(!b->group_callbacks[i]||!b->c15_callbacks[i])return M3_PAGE_INVALID;
 if(!templates_valid(t))return M3_PAGE_INVALID;
 const uint8_t*existing=(const uint8_t*)out;for(size_t i=0;i<sizeof *out;i++)if(existing[i])return M3_PAGE_BUSY;
 /* All inputs checked. No fallible operation or publication follows. */
 const uint32_t ids[3]={0x2c,b->c15_state,b->c60_state},counts[3]={2,7,2};
 const uint32_t labels[7]={0x304,0x305,0x307,0x308,0x309,0xc9e,0xc9f};
 for(unsigned page=0;page<3;page++){
  copy(out->states[page],page?t->leaf_state:t->group_state,96);
  copy(out->holders[page],page?t->leaf_holder:t->group_holder,80);
  copy(out->callbacks[page],page?t->leaf_callbacks:t->group_callbacks,40);
  w32(out->states[page],(ids[page]<<16)|2u);
  /* +8 is the native state-machine ancestor, not the visual menu parent.
   * Preserve original common root(2,1); back navigation uses native history. */
  w64(out->states[page]+0x28,(uintptr_t)out->callbacks[page]);
  w64(out->states[page]+0x50,(uintptr_t)out->holders[page]);
  w64(out->holders[page]+8,(uintptr_t)m3_native_page_headers[page]);
  w64(out->holders[page]+0x10,(uintptr_t)out->rows[page]);
  w64(out->holders[page]+0x18,(uintptr_t)out->event_pointers[page]);
  w16(out->holders[page]+0x20,counts[page]);
  if(page<2)for(unsigned j=0;j<5;j++)w64(out->callbacks[page]+j*8,page?b->c15_callbacks[j]:b->group_callbacks[j]);
  for(unsigned row=0;row<counts[page];row++){
   unsigned source=page==1?(row==1||row==4):row;
   copy(out->rows[page][row],page?t->leaf_rows[source]:t->group_rows[source],44);
   if(!page){w32(out->rows[page][row],row?M3_LABEL_C60_C120:M3_LABEL_C15_C30);w32(out->rows[page][row]+24,0x7e85+row);}
   if(page==1){w32(out->rows[page][row],labels[row]);w32(out->rows[page][row]+24,row<5?0x7e80+row:0x7e87+row-5);}
   out->event_pointers[page][row]=(uintptr_t)out->events[page? page+1:row];
  }
 }
 for(unsigned table=0;table<4;table++){
  unsigned source=table<2?table:2,n=t->event_counts[source];
  for(unsigned j=0;j<n;j++){
   uint8_t*e=out->events[table][j];copy(e,t->events[source][j],32);uint32_t event=u32(e);
   if(table<2){
    if(activate(event))w64(e+24,table?b->c60_state:b->c15_state);
    if(event==0x295||event==0x2af)w64(e+8,0x403b208c);
    if(event==0x296||event==0x2b0)w64(e+8,0x403b20a4);
    if(event==0x315)w64(e+8,b->group_aux_action);
   }else if(table==2&&(event==0x298||event==0x29d||event==0x2c0))w64(e+8,b->c15_submit);
  }
 }
 out->magic=0x334d5047;out->version=1;return M3_PAGE_OK;
}

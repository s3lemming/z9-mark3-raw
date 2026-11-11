#include "native_he_menu.h"
#include "native_live_settings.h"
#include "recall_native.h"
#ifdef M3_HE_MENU_TEST
extern struct m3_he_menu_runtime m3_he_menu_test_runtime;
#define R (&m3_he_menu_test_runtime)
extern uintptr_t m3_he_menu_test_function(uintptr_t);
#define FN(a,t) ((t)m3_he_menu_test_function(a))
#else
#define R ((struct m3_he_menu_runtime*)(uintptr_t)M3_MENU_STATE_ADDRESS)
#define FN(a,t) ((t)(uintptr_t)(a))
#endif
extern const struct m3_native_page_templates m3_he_menu_templates;
extern const uint8_t m3_he_menu_submenu_link_row[44];
/* Complete original52-byte getter, copied verbatim. Its only branch is
 * internal; absolute native registry address materialization is unchanged.
 * Never call the patched original entry recursively. */
extern void *m3_menu_original_lookup(uint32_t,uint32_t);
enum {INITIALIZING=1,READY=2,FAILED=3,C15=0x6c4,C60=0x6c5};
static uint32_t r16(const uint8_t*p){return p[0]|(uint32_t)p[1]<<8;}
static uint32_t r32(const uint8_t*p){return r16(p)|r16(p+2)<<16;}
static uint64_t r64(const uint8_t*p){return r32(p)|(uint64_t)r32(p+4)<<32;}
static void w32(uint8_t*p,uint32_t v){for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(v>>(8*i));}
static void w64(uint8_t*p,uint64_t v){for(unsigned i=0;i<8;i++)p[i]=(uint8_t)(v>>(8*i));}
static uint32_t state(void){return __atomic_load_n(&R->init_state,__ATOMIC_ACQUIRE);}
static int permission(void){return FN(0x405dad84,uint32_t(*)(void))()==1;}
static uint32_t row(const void*ctx){const uint8_t*p=ctx;return r16(p+0x32)+r16(p+0x58);}
static int owned(void*ctx,unsigned page){
 if(state()!=READY||page>2||ctx!=R->pages.states[page])return 0;
 struct m3_native_pages*p=&R->pages;
 return p->magic==0x334d5047&&p->version==1&&
  r64(p->states[page]+0x50)==(uintptr_t)p->holders[page]&&
  r64(p->holders[page]+0x10)==(uintptr_t)p->rows[page]&&
  r16(p->holders[page]+0x20)==(page==1?7u:2u)&&r64(p->states[page]+0x48);
}
static void summary_value(void*ctx,uint32_t id,uint32_t resource){
 void*value=FN(0x40503de8,void*(*)(void*,uint32_t,uint32_t))(ctx,id,0x4bec);
 if(!value)return;
 FN(0x403f141c,void(*)(void*,uint32_t))(value,resource!=UINT32_MAX);
 if(resource!=UINT32_MAX)FN(0x403f165c,void(*)(void*,uint32_t))(value,resource);
}
void m3_he_menu_group_values(void*ctx){
 if(!owned(ctx,0))return;
 struct m3_hsfc_settings selected;uint32_t label=UINT32_MAX;
 if(m3_live_read(0,&selected)&&selected.generation&&
    (selected.codec==3||selected.codec==4)){
  switch(selected.option){
  /* Original42006B70 right-value resources, bound by42006BB0 UI IDs:
   * RAW29CC(index13), RAW+F29C9(1), RAW+N29CA(3), F29CE(7), N29CF(9).
   * These are not the language/menu-row labels304..309. */
  case 0:label=0xd38;break;
  case 2:label=0xd36;break;
  case 4:label=0xd37;break;
  case 8:label=0xd34;break;
  case 10:label=0xb3c;break;
  default:break;
  }
 }
 summary_value(ctx,0x7e85,label);
 /* Original405DA9B8 reads this property for its HSFC summary. The private
  * C15/C30 choice never overwrites it; C60/C120 use the original two values. */
 uint32_t grade=FN(0x403e74a8,uint32_t(*)(uint32_t))(0x04006f10);
 label=grade==0?0xd34:grade==1?0xb3c:UINT32_MAX;
 summary_value(ctx,0x7e86,label);
}
static void group_auxiliary(struct m3_native_menu_event*e){
 if(e&&e->context==R->pages.states[0])e->target=0x6b8;
}
void m3_he_menu_group_refresh(void*ctx){
 if(!owned(ctx,0))return;
 __atomic_fetch_add(&R->group_refreshes,1,__ATOMIC_RELAXED);
 uint32_t allowed=(uint32_t)permission();
 /* Native403B1F74 takes DISABLED, inverts it, then writes the actual
  * enabled byte+29 through403F1428/40326364. It is not an enable(value) API. */
 struct m3_hsfc_settings committed;
 FN(0x403b1f74,void(*)(void*,uint32_t,uint32_t))(ctx,0x7e85,!(allowed&&m3_live_read(0,&committed)));
 FN(0x403b1f74,void(*)(void*,uint32_t,uint32_t))(ctx,0x7e86,!allowed);
 if(allowed&&(FN(0x403ae600,uint32_t(*)(void))()!=1||row(ctx)>=2))
  FN(0x403b0560,void(*)(void*,uint32_t))(ctx,0x7e85);
}
static void ui(struct m3_native_menu_ui*out){
 out->pages=&R->pages;out->contracts=M3_REQUIRED_CONTRACTS|M3_DX_CONTRACT|M3_HE_STAR_DX_CONTRACT;
 out->user=0;out->read_committed=m3_live_read;out->busy=m3_live_busy;out->commit=m3_live_commit;
 out->entry_style=FN(0x403ae600,uint32_t(*)(void));out->focus=FN(0x403b0560,void(*)(void*,uint32_t));
 out->mark=FN(0x403b05d0,void(*)(void*,uint32_t,uint32_t));
}
void m3_he_menu_leaf_refresh(void*ctx){
 if(!owned(ctx,1))return;
 __atomic_fetch_add(&R->leaf_refreshes,1,__ATOMIC_RELAXED);
 struct m3_hsfc_settings committed;int enabled=permission()&&m3_live_read(0,&committed);
 for(unsigned i=0;i<7;i++)FN(0x403b1f74,void(*)(void*,uint32_t,uint32_t))(ctx,i<5?0x7e80+i:0x7e87+i-5,!enabled);
 struct m3_native_menu_ui a;ui(&a);(void)m3_native_c15_refresh(&a,ctx);
}
static int setting_event(struct m3_native_menu_event*e){
 if(!e||e->context!=R->pages.states[1])return 0;
 uint32_t target=e->target;e->target=0x6b8;
 if(!owned(e->context,1)||target!=0x6b9||row(e->context)>=7||!permission()||
    (e->event!=0x298&&e->event!=0x29d&&e->event!=0x2c0)||m3_live_busy(0))return 0;
 struct m3_hsfc_settings committed;if(!m3_live_read(0,&committed))return 0;
 e->target=0x6b9;return 1;
}
int m3_he_menu_leaf_condition(struct m3_native_menu_event*e){return setting_event(e);}
void m3_he_menu_leaf_submit(struct m3_native_menu_event*e){
 if(!setting_event(e))return;
 struct m3_native_menu_ui a;ui(&a);
 enum m3_menu_ui_result rc=m3_native_c15_submit(&a,e);
 if(rc==M3_UI_APPLIED)__atomic_fetch_add(&R->submits,1,__ATOMIC_RELAXED);
 else __atomic_fetch_add(&R->blocked_events,1,__ATOMIC_RELAXED);
}
int m3_he_menu_group_condition(struct m3_native_menu_event*e){
 if(!e||e->context!=R->pages.states[0])return 0;
 if(!owned(e->context,0)||!permission()){e->target=0x6b8;return 0;}
 struct m3_hsfc_settings committed;if(row(e->context)==0&&!m3_live_read(0,&committed)){e->target=0x6b8;return 0;}
 return m3_native_router_condition(&R->router,e);
}
void m3_he_menu_group_open(struct m3_native_menu_event*e){
 if(!e||e->context!=R->pages.states[0])return;
 struct m3_hsfc_settings committed;
 if(!owned(e->context,0)||!permission()||(row(e->context)==0&&!m3_live_read(0,&committed))){e->target=0x6b8;return;}
 (void)m3_native_router_open(&R->router,e);
}
/* Native predicate slots are int(void). In405022D4 the action's24-byte
 * event is constructed AFTER the predicate call. Derive a private guard
 * message from the actual current context and this immutable table slot's
 * event identity; never dereference the caller's unspecified X0. Actions
 * still receive the real native event and independently recheck it. */
static int native_group_condition(uint32_t event){
 void*ctx=FN(0x403d6d38,void*(*)(void))();
 if(ctx!=R->pages.states[0]||!owned(ctx,0))return 0;
 uint32_t i=row(ctx);if(i>=2)return 0;
 struct m3_native_menu_event e={.context=ctx,.event=event,.target=i?C60:C15,.control=0,.reserved={0}};
 return m3_he_menu_group_condition(&e);
}
static int native_leaf_condition(uint32_t event){
 void*ctx=FN(0x403d6d38,void*(*)(void))();
 if(ctx!=R->pages.states[1]||!owned(ctx,1))return 0;
 struct m3_native_menu_event e={.context=ctx,.event=event,.target=0x6b9,.control=0,.reserved={0}};
 return m3_he_menu_leaf_condition(&e);
}
#define PREDICATE(name,fn,event) static int name(void){return fn(event);}
PREDICATE(group_298,native_group_condition,0x298)
PREDICATE(group_29d,native_group_condition,0x29d)
PREDICATE(group_2b1,native_group_condition,0x2b1)
PREDICATE(group_2c0,native_group_condition,0x2c0)
PREDICATE(leaf_298,native_leaf_condition,0x298)
PREDICATE(leaf_29d,native_leaf_condition,0x29d)
PREDICATE(leaf_2c0,native_leaf_condition,0x2c0)
#undef PREDICATE
static uintptr_t native_predicate(uint32_t event,unsigned leaf){
 if(event==0x298)return leaf?(uintptr_t)leaf_298:(uintptr_t)group_298;
 if(event==0x29d)return leaf?(uintptr_t)leaf_29d:(uintptr_t)group_29d;
 if(event==0x2c0)return leaf?(uintptr_t)leaf_2c0:(uintptr_t)group_2c0;
 if(!leaf&&event==0x2b1)return(uintptr_t)group_2b1;
 return 0;
}
static int original_aux_permission(struct m3_native_menu_event*e){(void)e;return FN(0x403b217c,int(*)(void))();}
static int initialize(void){
 uint32_t expected=0;
 if(!__atomic_compare_exchange_n(&R->init_state,&expected,INITIALIZING,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE))return expected==READY;
 struct m3_native_page_bindings b;
 b.c15_state=C15;b.c60_state=C60;
 b.group_callbacks[0]=(uintptr_t)m3_he_menu_group_refresh;
 b.group_callbacks[1]=0x405da990;b.group_callbacks[2]=0x403b2748;
 b.group_callbacks[3]=0x405dad84;b.group_callbacks[4]=(uintptr_t)m3_he_menu_group_values;
 b.c15_callbacks[0]=(uintptr_t)m3_he_menu_leaf_refresh;
 b.c15_callbacks[1]=0x405dad80;b.c15_callbacks[2]=0x403b2748;
 b.c15_callbacks[3]=0x405dad84;b.c15_callbacks[4]=0x405dadb0;
 b.group_aux_action=(uintptr_t)group_auxiliary;
 b.c15_submit=(uintptr_t)m3_he_menu_leaf_submit;
 uint32_t failed=0;
 if(m3_native_pages_build(&R->pages,&m3_he_menu_templates,&b)!=M3_PAGE_OK)failed=1;
 if(!failed){
  /* Both detail rows have the original HSFC value control. Their own tail
   * callback above supplies each exact new UI ID; the old original callback
   * only knew29C8/2DC8 and left stale values/hourglasses on private rows. */
  for(unsigned i=0;i<2;i++){
   volatile uint8_t*out=R->pages.rows[0][i];
   for(unsigned j=0;j<44;j++)out[j]=m3_he_menu_templates.group_rows[1][j];
   w32(R->pages.rows[0][i],0xc9c+i);w32(R->pages.rows[0][i]+24,0x7e85+i);
  }
  if(m3_native_router_bind(&R->router,&R->pages,(void*)(uintptr_t)0xa028fd970ull,C15,C60,
      m3_menu_original_lookup,FN(0x403b20c0,void(*)(struct m3_native_menu_event*)),
      /* Public wrapper calls the full403ADBE0 constructor and, only on
       * success, emits the native403EB0DC(403EB078,3,7) UI notification. */
      FN(0x403adbac,void(*)(uint32_t,uint32_t)),original_aux_permission,
      (uintptr_t)m3_he_menu_group_open,(uintptr_t)m3_he_menu_group_condition)!=M3_ROUTE_OK)failed=2;
  if(!failed)for(unsigned table=0;table<3;table++)for(unsigned j=0;j<16;j++){
   uintptr_t predicate=native_predicate(r32(R->pages.events[table][j]),table==2);
   if(predicate)w64(R->pages.events[table][j]+16,predicate);
  }
 }
 R->fault=failed;
 if(!failed)R->router.enabled=1;
 __atomic_store_n(&R->init_state,failed?FAILED:READY,__ATOMIC_RELEASE);
 return !failed;
}
void *m3_native_ctor_lookup_entry(uint32_t slot,uint32_t index,const void*previous){
 uint32_t id=index&65535;
 if(slot==2&&id==F2R_LEAF_ID)return f2r_menu_lookup(slot,index,previous,0);
 if(slot==2&&id==0x2c&&previous==(void*)(uintptr_t)0xa028fd970ull)(void)initialize();
 if(slot==2&&(id==C15||id==C60)){
  if(state()!=READY)return 0;
  __atomic_fetch_add(&R->constructor_calls,1,__ATOMIC_RELAXED);
  return m3_native_router_lookup(&R->router,slot,index,previous);
 }
 if(slot==2&&id==0x2c&&
    (previous==(void*)(uintptr_t)0xa028fd970ull||previous==R->pages.states[0]||previous==R->pages.states[1]||previous==R->pages.states[2])&&state()==READY)
  return m3_native_router_lookup(&R->router,slot,index,previous);
 return m3_menu_original_lookup(slot,index);
}
void *m3_he_menu_global_lookup(uint32_t slot,uint32_t index){
 uint32_t id=index&65535;
 if(slot==2&&id==F2R_LEAF_ID)return f2r_menu_lookup(slot,index,0,1);
 /* Ordinary boot/UI callers never touch the extra RAM page at all. */
 if(slot!=2||(id!=C15&&id!=C60))return m3_menu_original_lookup(slot,index);
 if(state()!=READY)return 0;
 __atomic_fetch_add(&R->private_lookup_calls,1,__ATOMIC_RELAXED);
 return m3_native_router_global_lookup(&R->router,slot,index);
}

#include "recall_native.h"
#include "recall_field_codec.h"
#include "recall_value_overlay.h"
#include "native_he_menu.h"
#include "mark2_audio_runtime.h"
#include <stddef.h>
#ifdef F2R_TEST
extern uintptr_t f2r_test_function(uintptr_t);
#define FN(a,t) ((t)f2r_test_function(a))
#else
#define FN(a,t) ((t)(uintptr_t)(a))
#endif
extern uint32_t f2r_original_get(uint32_t,uint32_t*);
extern void f2r_original_enter(const void*);
extern void f2r_original_exit(void);
extern const uint8_t f2r_vr_header[544];
extern const uint8_t f2r_vr_state[96],f2r_vr_holder[80];
extern const struct m3_native_page_templates m3_he_menu_templates;
enum {MAGIC=0x46325231u,INIT_BUSY=1,INIT_READY=2,INIT_FAILED=3,
      VR_UI=0x7ea0,C15_UI=0x7ea1,C30_UI=0x7ea2,OFF_UI=0x7ea3};
struct f2r_leaf {
 uint8_t state[96],holder[80],callbacks[40],rows[3][44];
 uint64_t event_pointers[3];uint8_t events[15][32];
};
struct runtime {
 uint32_t magic,init_state,fault,vr_allowed,guard_column,guard_mask;
 uint32_t enters,exits,gets,menu_commits,last_error,saves,notifies,guard_generation;
 struct f2r_leaf leaf;
 uint32_t save_gate_calls,save_gate_value,save_requests,save_batch_calls;
 uint32_t save_batch_count,save_first_id,save_precheck_fault,save_return;
};
_Static_assert(offsetof(struct runtime,save_gate_calls)==912,"Diagnostic offset ABI");
_Static_assert(sizeof(struct runtime)<=F2R_HEAP_BYTES,"Recall heap tail exhausted");
_Static_assert(M23_AUDIO_BYTES>=F2R_HEAP_OFFSET+F2R_HEAP_BYTES,"Recall allocation not reserved");
static uint32_t u16(const uint8_t*p){return p[0]|(uint32_t)p[1]<<8;}
static uint32_t u32(const uint8_t*p){return u16(p)|u16(p+2)<<16;}
static uint64_t u64(const uint8_t*p){return u32(p)|(uint64_t)u32(p+4)<<32;}
static void w16(uint8_t*p,uint32_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
static void w32(uint8_t*p,uint32_t v){w16(p,v);w16(p+2,v>>16);}
static void w64(uint8_t*p,uint64_t v){w32(p,(uint32_t)v);w32(p+4,(uint32_t)(v>>32));}
static void copy(uint8_t*d,const uint8_t*s,size_t n){for(size_t i=0;i<n;i++)d[i]=s[i];}
static struct runtime*runtime(void){
 if(!m23_audio_runtime_ready())return 0;
 uint8_t*base=m23_audio_runtime_base();if(!base)return 0;
 struct runtime*r=(void*)(base+F2R_HEAP_OFFSET);uint32_t m=__atomic_load_n(&r->magic,__ATOMIC_ACQUIRE);
 if(!m){uint32_t expected=0;(void)__atomic_compare_exchange_n(&r->magic,&expected,MAGIC,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE);m=__atomic_load_n(&r->magic,__ATOMIC_ACQUIRE);}
 return m==MAGIC?r:0;
}
static uint32_t column(void){
 uint32_t id=FN(0x4050d134,uint32_t(*)(uint32_t))(0);
 return id==0x06007929u?1u:id==0x060014d7u?0u:2u;
}
static uint32_t mask_id(unsigned c){return c?0x06007929u:0x060014d7u;}
static uint32_t target_id(unsigned c){return c?0x0600793du:0x06001792u;}
static int raw(uint32_t id,uint32_t*out){return (f2r_original_get(id,out)&1)!=0;}
static int fields(unsigned c,uint32_t*word,struct f2_recall_fields*f){
 if(c>1||!raw(mask_id(c),word))return 0;
 enum f2_recall_field_status s=f2_recall_fields_decode(*word,f);
 return s==F2_FIELD_OK||s==F2_FIELD_LEGACY;
}
static void requalify(void);
static int commit(unsigned c,uint32_t before,const struct f2_recall_fields*f){
 uint32_t next=0,after=0;struct runtime*r=runtime();
 if(c>1||f2_recall_fields_encode(before,f,&next)!=F2_FIELD_OK)return 0;
 if(!FN(0x403e74ac,uint32_t(*)(uint32_t,uint32_t))(mask_id(c),next)||!raw(mask_id(c),&after)||after!=next){if(r)r->last_error=1;return 0;}
 if(r){r->menu_commits++;r->last_error=0;}requalify();
 /* This mask also determines the existing stored recall release target.
  * Save-current has already published its native target (e.g. S0). A later
  * C15/C30 mask edit must invalidate that target's native subscribers too;
  * merely changing the getter view leaves their old target cached. */
 struct f2_recall_fields prior;
 enum f2_recall_field_status prior_status=f2_recall_fields_decode(before,&prior);
 if((prior_status==F2_FIELD_OK||prior_status==F2_FIELD_LEGACY)&&prior.release_override!=f->release_override){
  uint32_t id=target_id(c);
  FN(0x40ec85c4,void(*)(uint32_t,const uint32_t*,uint32_t))(1,&id,0);
  if(r)r->notifies++;
 }
 return 1;
}
static uint32_t row(const void*ctx){const uint8_t*p=ctx;return u16(p+0x32)+u16(p+0x58);}
#ifdef F2R_TEST
extern void *f2r_test_parent_context,*f2r_test_release_context;
static int parent(const void*ctx){return ctx==f2r_test_parent_context;}
static int release_page(const void*ctx){return ctx==f2r_test_release_context;}
#else
static int parent(const void*ctx){return ctx==(void*)(uintptr_t)0xa02909790ull;}
static int release_page(const void*ctx){return ctx==(void*)(uintptr_t)0xa02909d90ull;}
#endif
static void value(void*ctx,uint32_t ui,uint32_t label){
 void*p=FN(0x40503de8,void*(*)(void*,uint32_t,uint32_t))(ctx,ui,0x4bec);
 if(!p)return;
 /* Original live objects prove kind1 is an icon (P/checkbox), kind2 is
  * translated text (row captions). 165C preserves the current kind. */
 uint32_t kind=0,old_label=0;
 FN(0x403f1400,void(*)(void*,uint32_t*,uint32_t*))(p,&kind,&old_label);
 if(kind!=0&&kind!=2)FN(0x403eca24,void(*)(void*,uint32_t))(p,0);
 FN(0x403f13ec,uint32_t(*)(void*,uint32_t,uint32_t))(p,2,label);
}
static void extras(void*ctx){
 if(!parent(ctx))return;
 uint32_t word=0;struct f2_recall_fields f;int ok=fields(column(),&word,&f);
 void*p=FN(0x40503de8,void*(*)(void*,uint32_t,uint32_t))(ctx,VR_UI,0x4bef);
 if(p)FN(0x403f165c,void(*)(void*,uint32_t))(p,ok&&f.vr_enabled?0x126f:0x1270);
 /* The native checkbox helper's third argument is cache-index81..8C,
  * not a UI ID. Our row uses its own encoded flag; no unreserved cache slot. */
 value(ctx,VR_UI,ok?(f.vr_mode==1?0x5c9:f.vr_mode==2?0x5cb:0x18a):0x18a);
 FN(0x403b1f74,void(*)(void*,uint32_t,uint32_t))(ctx,VR_UI,!ok);
 if(ok&&f.release_override)value(ctx,0x342f,f.release_override==F2_RELEASE_C15?0x95e:0x753);
}
void f2r_parent_refresh(void*ctx){FN(0x4053160c,void(*)(void*))(ctx);extras(ctx);}
void f2r_parent_values(void*ctx){FN(0x40531704,void(*)(void*))(ctx);extras(ctx);}
static int leaf_owned(const struct runtime*r,const void*ctx){
 return r&&ctx==r->leaf.state&&__atomic_load_n(&r->init_state,__ATOMIC_ACQUIRE)==INIT_READY&&
  u64(r->leaf.state+0x50)==(uintptr_t)r->leaf.holder&&u64(r->leaf.holder+0x10)==(uintptr_t)r->leaf.rows&&
  u16(r->leaf.holder+0x20)==3&&u64(r->leaf.state+0x48)!=0;
}
static void vr_refresh(void*ctx){
 struct runtime*r=runtime();if(!leaf_owned(r,ctx))return;
 FN(0x405317a8,void(*)(void*))(ctx); /* Native f2/button heading. */
 uint32_t word=0;struct f2_recall_fields f;int ok=fields(column(),&word,&f);
 for(unsigned i=0;i<3;i++){
  FN(0x403b1f74,void(*)(void*,uint32_t,uint32_t))(ctx,OFF_UI+i,!ok);
  FN(0x403b05d0,void(*)(void*,uint32_t,uint32_t))(ctx,OFF_UI+i,ok&&f.vr_mode==i);
 }
 if(ok&&FN(0x403ae600,uint32_t(*)(void))()!=1)FN(0x403b0560,void(*)(void*,uint32_t))(ctx,OFF_UI+f.vr_mode);
}
static void nop(void*ctx){(void)ctx;}
static uint32_t enabled(void){return 1;}
static void vr_submit(struct m3_native_menu_event*e){
 if(!e)return;
 uint32_t prior=e->target;e->target=0x6b8;
 struct runtime*r=runtime();if(!leaf_owned(r,e->context)||prior!=0x6b9||
  (e->event!=0x298&&e->event!=0x29d&&e->event!=0x2c0))return;
 unsigned c=column(),i=row(e->context);uint32_t word=0;struct f2_recall_fields f;
 if(i>=3||!fields(c,&word,&f))return;
 f.vr_mode=i;
 if(commit(c,word,&f))e->target=0x6b9;
}
static int leaf_init(struct runtime*r){
 uint32_t expected=0;
 if(!__atomic_compare_exchange_n(&r->init_state,&expected,INIT_BUSY,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE))return expected==INIT_READY;
 const struct m3_native_page_templates*t=&m3_he_menu_templates;struct f2r_leaf*p=&r->leaf;
 if(u32(t->leaf_state)!=0x2c0002u||t->event_counts[2]!=15){r->fault=1;__atomic_store_n(&r->init_state,INIT_FAILED,__ATOMIC_RELEASE);return 0;}
 copy(p->state,f2r_vr_state,96);copy(p->holder,f2r_vr_holder,80);copy(p->callbacks,t->leaf_callbacks,40);
 w32(p->state,(F2R_LEAF_ID<<16)|2);w64(p->state+0x28,(uintptr_t)p->callbacks);w64(p->state+0x50,(uintptr_t)p->holder);
 w64(p->holder+8,(uintptr_t)f2r_vr_header);w64(p->holder+16,(uintptr_t)p->rows);w64(p->holder+24,(uintptr_t)p->event_pointers);w16(p->holder+32,3);
 const uint32_t labels[3]={0x18a,0x5c9,0x5cb};
 for(unsigned i=0;i<3;i++){copy(p->rows[i],t->leaf_rows[0],44);w32(p->rows[i],labels[i]);w32(p->rows[i]+24,OFF_UI+i);p->event_pointers[i]=(uintptr_t)p->events;}
 for(unsigned j=0;j<15;j++){
  copy(p->events[j],t->events[2][j],32);uint32_t event=u32(p->events[j]);
  if(event==0x298||event==0x29d||event==0x2c0){w64(p->events[j]+8,(uintptr_t)vr_submit);w64(p->events[j]+16,0);}
 }
 w64(p->callbacks,(uintptr_t)vr_refresh);w64(p->callbacks+8,(uintptr_t)nop);w64(p->callbacks+16,0x403b2748);w64(p->callbacks+24,(uintptr_t)enabled);w64(p->callbacks+32,(uintptr_t)nop);
 __atomic_store_n(&r->init_state,INIT_READY,__ATOMIC_RELEASE);return 1;
}
void *f2r_menu_lookup(uint32_t slot,uint32_t id,const void*previous,uint32_t global){
 struct runtime*r=runtime();if(slot!=2||(id&65535)!=F2R_LEAF_ID||!r||__atomic_load_n(&r->init_state,__ATOMIC_ACQUIRE)!=INIT_READY)return 0;
 return global||parent(previous)||previous==r->leaf.state?r->leaf.state:0;
}
void f2r_vr_open(struct m3_native_menu_event*e){
 if(!e)return;
 uint32_t before=e->target;e->target=0x6b8;
 if(!parent(e->context)||row(e->context)!=12||before!=F2R_LEAF_ID)return;
 uint32_t word;struct f2_recall_fields f;struct runtime*r=runtime();if(!r||!fields(column(),&word,&f)||!leaf_init(r))return;
 FN(0x403adbac,void(*)(uint32_t,uint32_t))(2,F2R_LEAF_ID);
}
void f2r_vr_toggle(struct m3_native_menu_event*e){
 if(!e||!parent(e->context)||row(e->context)!=12||(e->event!=0x29d&&e->event!=0x2c0))return;
 uint32_t word;struct f2_recall_fields f;unsigned c=column();if(!fields(c,&word,&f))return;
 f.vr_enabled=!f.vr_enabled;if(commit(c,word,&f))FN(0x403b2bf8,void(*)(void*))(e->context);
}
void f2r_vr_aux(struct m3_native_menu_event*e){if(e&&parent(e->context))FN(0x403d66d4,void(*)(void*))(e->context);}
void f2r_release_refresh(void*ctx){
 FN(0x40531aa4,void(*)(void*))(ctx);if(!release_page(ctx))return;
 uint32_t word;struct f2_recall_fields f;if(!fields(column(),&word,&f))return;
 uint32_t ids[5]={0x3430,0x3435,0x343e,C15_UI,C30_UI},selected=0,native=0;
 if(f.release_override)selected=f.release_override==F2_RELEASE_C15?3:4;
 else if(raw(target_id(column()),&native)&&native<=2)selected=2-native;
 for(unsigned i=0;i<5;i++)FN(0x403b05d0,void(*)(void*,uint32_t,uint32_t))(ctx,ids[i],i==selected);
 /* The original S row is the event/style donor, not the Cxx icon. */
 for(unsigned i=3;i<5;i++){
  void*p=FN(0x40503de8,void*(*)(void*,uint32_t,uint32_t))(ctx,ids[i],0x4bec);
  if(p)FN(0x403f141c,void(*)(void*,uint32_t))(p,0);
 }
 if(FN(0x403ae600,uint32_t(*)(void))()!=1)FN(0x403b0560,void(*)(void*,uint32_t))(ctx,ids[selected]);
}
void f2r_release_submit(struct m3_native_menu_event*e){
 if(!e||!release_page(e->context))return;
 uint32_t old=e->target;e->target=0x6b8;
 if(old!=0x6b9||(e->event!=0x298&&e->event!=0x29d&&e->event!=0x2c0))return;
 unsigned c=column(),i=row(e->context);uint32_t word;struct f2_recall_fields f;if(i>=5||!fields(c,&word,&f))return;
 f.release_override=i==3?F2_RELEASE_C15:i==4?F2_RELEASE_C30:F2_RELEASE_NATIVE;
 if(i<3){
  uint32_t next;if(f2_recall_fields_encode(word,&f,&next)!=F2_FIELD_OK)return;
  uint32_t ids[2]={target_id(c),mask_id(c)},values[2]={2-i,next};
  if(!FN(0x403e74d8,uint32_t(*)(const uint32_t*,const uint32_t*,uint32_t))(ids,values,2))return;
  uint32_t a=0,b=0;if(!raw(ids[0],&a)||a!=values[0]||!raw(ids[1],&b)||b!=values[1])return;requalify();
 }else if(!commit(c,word,&f))return;
 e->target=0x6b9;
}
static void speed_submit(struct m3_native_menu_event*e,uintptr_t original){
 uint32_t accepted=FN(original,uint32_t(*)(struct m3_native_menu_event*))(e);
 if(!(accepted&1)){if(e)e->target=0x6b8;return;}
 uint32_t word;struct f2_recall_fields f;unsigned c=column();if(!fields(c,&word,&f)||!f.release_override)return;
 f.release_override=F2_RELEASE_NATIVE;if(!commit(c,word,&f)&&e)e->target=0x6b8;
}
void f2r_ch_submit(struct m3_native_menu_event*e){speed_submit(e,0x405110e0);}
void f2r_cl_submit(struct m3_native_menu_event*e){speed_submit(e,0x40511184);}
uint32_t f2r_getter_post(uint32_t id,uint32_t*out,uint32_t status){
 if(!(status&1)||!out)return status;
 struct f2_recall_overlay s={{0,0},0,0,0};
 if(id==0x06001792u||id==0x0600793du){unsigned c=id==0x0600793du;if(!raw(mask_id(c),&s.mask[c]))return status;}
 else if(id==0x07000675u||id==0x03000676u){
  s.active=FN(0x40f28424,uint32_t(*)(uint32_t))(0x4000012c);if(s.active!=1)return status;
  s.column=FN(0x40f28424,uint32_t(*)(uint32_t))(0x40000143);if(s.column>1)return status;
  struct runtime*r=runtime();if(!r)return status;
  uint32_t generation=__atomic_load_n(&r->guard_generation,__ATOMIC_ACQUIRE);
  if((generation&1)||__atomic_load_n(&r->vr_allowed,__ATOMIC_ACQUIRE)!=1||r->guard_column!=s.column||!raw(mask_id(s.column),&s.mask[s.column])||r->guard_mask!=s.mask[s.column]||generation!=__atomic_load_n(&r->guard_generation,__ATOMIC_ACQUIRE))return status;
  s.vr_allowed=1;r->gets++;
 }else return status;
 (void)f2_recall_overlay_value(&s,id,out);return status;
}
struct options {uint32_t id,flags,count,pad;uint32_t*values;};
static void notify_vr(struct runtime*r){uint32_t ids[2]={0x07000675,0x03000676};FN(0x40ec85c4,void(*)(uint32_t,const uint32_t*,uint32_t))(2,ids,0);if(r)r->notifies++;}
static uint32_t guard_begin(struct runtime*r){
 uint32_t expected=__atomic_load_n(&r->guard_generation,__ATOMIC_ACQUIRE);
 if((expected&1)||!__atomic_compare_exchange_n(&r->guard_generation,&expected,expected+1,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE))return UINT32_MAX;
 return __atomic_exchange_n(&r->vr_allowed,0,__ATOMIC_ACQ_REL);
}
static void guard_finish(struct runtime*r,uint32_t old,uint32_t permit){
 if(!r||old==UINT32_MAX)return;
 unsigned c=FN(0x40f28424,uint32_t(*)(uint32_t))(0x40000143);uint32_t word;struct f2_recall_fields f;
 if(permit&&FN(0x40f28424,uint32_t(*)(uint32_t))(0x4000012c)==1&&c<=1&&fields(c,&word,&f)&&f.vr_enabled){
  uint32_t values[4]={0};struct options o={0,0,0,0,values};
  uint32_t ok=FN(0x41018524,uint32_t(*)(uint32_t,struct options*,uint32_t))(0x07000675,&o,sizeof values);
  if(ok&&o.flags!=1&&o.count>0&&o.count<=4)for(unsigned i=0;i<o.count;i++)if(values[i]==f.vr_mode){r->guard_column=c;r->guard_mask=word;__atomic_store_n(&r->vr_allowed,1,__ATOMIC_RELEASE);break;}
 }
 __atomic_fetch_add(&r->guard_generation,1,__ATOMIC_RELEASE);
 if(old||__atomic_load_n(&r->vr_allowed,__ATOMIC_ACQUIRE))notify_vr(r);
}
static void requalify(void){struct runtime*r=runtime();if(r)guard_finish(r,guard_begin(r),1);}
void f2r_enter(const void*args){
 struct runtime*r=runtime();uint32_t old=0;if(r){old=guard_begin(r);r->enters++;}
 f2r_original_enter(args);guard_finish(r,old,1);
}
void f2r_exit(void){
 struct runtime*r=runtime();uint32_t old=0;if(r){old=guard_begin(r);r->exits++;}
 f2r_original_exit();guard_finish(r,old,0);
}
uint32_t f2r_vr_override_active(void){uint32_t probe=UINT32_MAX;(void)f2r_getter_post(0x07000675,&probe,1);return probe!=UINT32_MAX;}
static int save_column(const uint32_t*ids,uint32_t n){
 if(!ids||n!=23)return -1;
 return ids[0]==0x060014d8u?0:ids[0]==0x0600792au?1:-1;
}
uint32_t f2r_save_permission(void){
 uint32_t result=FN(0x4050c1bc,uint32_t(*)(void))();
 struct runtime*r=runtime();if(r){r->save_gate_calls++;r->save_gate_value=result;}
 return result; /* Observe the original result, never bypass it. */
}
void f2r_save_current_action(struct m3_native_menu_event*e){
 struct runtime*r=runtime();if(r&&e&&parent(e->context)&&row(e->context)==13)r->save_requests++;
 FN(0x405338ac,void(*)(struct m3_native_menu_event*))(e);
}
int32_t f2r_save_batch(const uint32_t*ids,const uint32_t*values,uint32_t n){
 struct runtime*r=runtime();if(r){r->save_batch_calls++;r->save_batch_count=n;r->save_first_id=ids&&n?ids[0]:0;r->save_precheck_fault=0;}
 int c=save_column(ids,n);if(c<0)return FN(0x410e43a0,int32_t(*)(const uint32_t*,const uint32_t*,uint32_t))(ids,values,n);
 uint32_t word,vr,selector,category,selection;struct f2_recall_fields f;
 if(!values||!fields((unsigned)c,&word,&f)||!raw(0x07000675,&vr)||vr>2||!raw(0x02000006,&selector)||!raw(0x06000108,&category)||!raw(0x06000109,&selection)){if(r)r->save_precheck_fault=1;return -1;}
 f.vr_mode=vr;f.release_override=selector==7&&category==1&&selection==6?F2_RELEASE_C15:selector==7&&category==1&&selection==3?F2_RELEASE_C30:F2_RELEASE_NATIVE;
 uint32_t next;if(f2_recall_fields_encode(word,&f,&next)!=F2_FIELD_OK)return -1;
 uint32_t all_ids[24],all_values[24];for(unsigned i=0;i<23;i++){all_ids[i]=ids[i];all_values[i]=values[i];}all_ids[23]=mask_id((unsigned)c);all_values[23]=next;
 int32_t rc=FN(0x410e43a0,int32_t(*)(const uint32_t*,const uint32_t*,uint32_t))(all_ids,all_values,24);
 if(r){r->save_return=(uint32_t)rc;r->last_error=rc?2:0;if(!rc)r->saves++;}return rc;
}
void f2r_save_notify(uint32_t n,const uint32_t*ids,uint32_t flags){
 int c=save_column(ids,n);if(c<0){FN(0x40ec85c4,void(*)(uint32_t,const uint32_t*,uint32_t))(n,ids,flags);return;}
 uint32_t all[24];for(unsigned i=0;i<23;i++)all[i]=ids[i];all[23]=mask_id((unsigned)c);
 FN(0x40ec85c4,void(*)(uint32_t,const uint32_t*,uint32_t))(24,all,flags);
 requalify();
}

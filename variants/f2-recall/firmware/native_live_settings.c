#include "native_live_settings.h"
#include "native_settings_view.h"
#include "native_settings_bank.h"
#include "native_settings_nor.h"
#include "native_settings_verify.h"
#include "native_runtime_layout.h"
#include "native_hud_publish.h"
#include "native_precapture_limit.h"
#ifndef M3_LIVE_TEST
#define LIVE ((struct m3_live_settings*)(uintptr_t)M3_LIVE_SETTINGS_ADDRESS)
#define NATIVE(a,t) ((t)(uintptr_t)(a))
#else
extern struct m3_live_settings m3_live_test_state;
extern uintptr_t m3_live_test_function(uintptr_t);
#define LIVE (&m3_live_test_state)
#define NATIVE(a,t) ((t)m3_live_test_function(a))
#endif
static void copy(volatile void*d,const void*s,size_t n){volatile uint8_t*out=d;const uint8_t*in=s;for(size_t i=0;i<n;i++)out[i]=in[i];}
static uint32_t state(void){return __atomic_load_n(&LIVE->state,__ATOMIC_ACQUIRE);}
static int ready(void){uint32_t s=state();return s==M3_LIVE_PRESENT||s==M3_LIVE_LEGACY;}
static void fence(uint32_t reason){__atomic_store_n(&LIVE->condition_selection,0,__ATOMIC_RELEASE);LIVE->fault=reason;__atomic_store_n(&LIVE->state,M3_LIVE_FAILED,__ATOMIC_RELEASE);}
static int take(struct m3_native_gate_token*t){return m3_native_gate_acquire(&m3_native_gate_rom532,&LIVE->gate,t)==M3_GATE_OK;}
static int give(struct m3_native_gate_token*t){if(m3_native_gate_release(&m3_native_gate_rom532,&LIVE->gate,t)==M3_GATE_OK)return 1;fence(9);return 0;}
static uint32_t token_owned(void*u){return m3_native_gate_owned(&LIVE->gate,u);}
static int owned(void*u){return m3_native_save_context_owned(u);}
static enum m3_save_selection select_setting(void*u,const struct m3_native_settings_schema*s,struct m3_hsfc_settings*out,uint32_t*force){
 if(!owned(u)||!ready())return M3_SAVE_REFUSE;
 if(s==&LIVE->request_schema){
  if(__atomic_load_n(&LIVE->request_active,__ATOMIC_ACQUIRE)!=3||LIVE->admissions||LIVE->expected_generation!=LIVE->committed.generation)return M3_SAVE_REFUSE;
  copy(out,&LIVE->desired,sizeof *out);
  *force=state()==M3_LIVE_LEGACY||LIVE->desired.option!=LIVE->committed.option||LIVE->desired.generation!=LIVE->committed.generation;
  return M3_SAVE_ENCODE;
 }
 if(state()==M3_LIVE_LEGACY)return M3_SAVE_NATIVE;
 copy(out,&LIVE->committed,sizeof *out);*force=0;return M3_SAVE_ENCODE;
}
static int accept(void*u,const struct m3_native_save_cookie*c){
 if(!owned(u)||!ready())return 0;
 copy(&LIVE->committed,&c->settings,sizeof c->settings);
 __atomic_store_n(&LIVE->condition_selection,((uint64_t)c->settings.codec<<32)|c->settings.option,__ATOMIC_RELEASE);
 __atomic_store_n(&LIVE->state,M3_LIVE_PRESENT,__ATOMIC_RELEASE);
 if(c->schema==&LIVE->request_schema)__atomic_store_n(&LIVE->request_result,1,__ATOMIC_RELEASE);
 LIVE->accepted++;return 1;
}
static void failed(void*u,const struct m3_native_save_cookie*c){(void)c;if(owned(u)){LIVE->failed++;fence(7);}}
static void release_failed(struct m3_native_save_frame*f){(void)f;fence(9);}
static const struct m3_native_save_binding binding={
 .gate_api=&m3_native_gate_rom532,.gate=&LIVE->gate,.user=LIVE,
 .callbacks={.select=select_setting,.tail={m3_native_tail_program,m3_native_tail_read,0},
 .verify=m3_native_saved_settings_verify,.accept=accept,.failed=failed},.release_failed=release_failed
};
uint32_t m3_native_save_begin(const struct m3_native_storage_descriptor*d,const struct m3_native_settings_schema*s,struct m3_native_save_frame*f){
 if(m3_native_settings_domain(d,s)){
  __atomic_fetch_add(&LIVE->save_calls,1,__ATOMIC_RELAXED);
  if(!ready())__atomic_fetch_add(&LIVE->saves_before_bootstrap,1,__ATOMIC_RELAXED);
 }
 /* Unready F calls are refused, never passed through an unknown private
  * record. Every other domain still executes the original core unchanged. */
 return m3_native_save_context_prepare(&binding,d,s,f);
}
void m3_live_bootstrap_before(uint64_t sp){
 uint32_t expected=M3_LIVE_COLD;
 __atomic_fetch_add(&LIVE->bootstrap_calls,1,__ATOMIC_RELAXED);
 if(!__atomic_compare_exchange_n(&LIVE->state,&expected,M3_LIVE_LOADING,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE)){fence(1);return;}
 LIVE->bootstrap_SP=sp;
 if(m3_native_gate_init(&m3_native_gate_rom532,&LIVE->gate,(const uint8_t*)"M3Fsave")==M3_GATE_OK)return;
 fence(2);
}
void m3_live_bootstrap_after(uint32_t original){
 LIVE->original_load_result=original;if(state()!=M3_LIVE_LOADING)return;
 struct m3_native_gate_token token={0};if(!take(&token)){fence(3);return;}
 struct m3_native_view_api va;struct m3_native_settings_view view;
 struct m3_native_bank_api ba;struct m3_native_bank_provider bp;struct m3_native_load_api la;
 struct m3_hsfc_settings found={UINT32_MAX,UINT32_MAX,0};
 if(!m3_native_view_bind_rom532(&va,&token,token_owned)||m3_native_settings_view_owned(&va,&view)!=M3_NATIVE_VIEW_OK||
    !m3_native_bank_bind_rom532(&ba,&token,token_owned)||!m3_native_load_bind_bank_provider(&la,&bp,&ba,(void*)(uintptr_t)0xa03478120ull)){
  fence(4);give(&token);return;
 }
 enum m3_native_load_result result=m3_native_settings_load_owned(&la,&view.storage,&view.schema,&found,&LIVE->load_receipt);
 LIVE->load_result=result;
 uint32_t next=M3_LIVE_PRESENT;
 if(result==M3_LOAD_ABSENT){
  uint32_t grade=NATIVE(0x403e74a8,uint32_t(*)(uint32_t))(0x04006f10);
  if(m3_settings_migrate_legacy(grade,&found)!=M3_RECORD_OK){fence(5);give(&token);return;}
  next=M3_LIVE_LEGACY;
 }else if(result!=M3_LOAD_OK){fence(6);give(&token);return;}
 copy(&LIVE->committed,&found,sizeof found);
 __atomic_store_n(&LIVE->condition_selection,((uint64_t)found.codec<<32)|found.option,__ATOMIC_RELEASE);
 __atomic_store_n(&LIVE->state,next,__ATOMIC_RELEASE);give(&token);
}
int m3_live_read(void*user,struct m3_hsfc_settings*out){
 (void)user;if(!out||!ready())return 0;struct m3_native_gate_token t={0};if(!take(&t))return 0;
 int ok=ready();if(ok)copy(out,&LIVE->committed,sizeof *out);return give(&t)&&ok;
}
int m3_live_busy(void*user){(void)user;return !ready()||__atomic_load_n(&LIVE->admissions,__ATOMIC_ACQUIRE)||__atomic_load_n(&LIVE->request_active,__ATOMIC_ACQUIRE);}
int m3_live_commit(void*user,uint64_t expected,const struct m3_hsfc_settings*next){
 (void)user;uint8_t check[80];
 if(!next||(next->codec!=3&&next->codec!=4)||m3_settings_encode(next,check,sizeof check)!=M3_RECORD_OK||!ready())return 0;
 uint32_t idle=0;if(!__atomic_compare_exchange_n(&LIVE->request_active,&idle,1,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE))return 0;
 LIVE->request_result=0;int ok=0;struct m3_native_gate_token t={0};
 if(!take(&t))goto done;
 if(!ready()||LIVE->admissions||LIVE->committed.generation!=expected||
   ((next->option==LIVE->committed.option&&next->codec==LIVE->committed.codec)?next->generation!=expected:(expected==UINT64_MAX||next->generation!=expected+1))){give(&t);goto done;}
 struct m3_native_view_api va;struct m3_native_settings_view view;
 if(!m3_native_view_bind_rom532(&va,&t,token_owned)||m3_native_settings_view_owned(&va,&view)!=M3_NATIVE_VIEW_OK){give(&t);goto done;}
 copy(&LIVE->request_schema,&view.schema,sizeof view.schema);copy(&LIVE->desired,next,sizeof *next);LIVE->expected_generation=expected;
 if(!give(&t))goto done;
 /* The original synchronous dispatcher wakes and joins its settings worker.
  * Its exact F-client call at410161BC claims the published request schema.
  * Never execute a native F client directly on the menu task's stack. */
 __atomic_store_n(&LIVE->request_active,2,__ATOMIC_RELEASE);
 NATIVE(0x410167bc,void(*)(void))();
 ok=__atomic_load_n(&LIVE->request_result,__ATOMIC_ACQUIRE)==1&&ready();
done:
 __atomic_store_n(&LIVE->request_active,0,__ATOMIC_RELEASE);
 if(ok){m3_precapture_selection_changed();m3_hud_selection_changed();}
 return ok;
}
int m3_live_admit(struct m3_hsfc_settings*out){
 if(!out||!ready())return 0;
 struct m3_native_gate_token t={0};if(!take(&t))return 0;
 int ok=ready()&&!__atomic_load_n(&LIVE->request_active,__ATOMIC_ACQUIRE)&&LIVE->admissions!=UINT32_MAX;
 if(ok){copy(out,&LIVE->committed,sizeof *out);__atomic_fetch_add(&LIVE->admissions,1,__ATOMIC_RELEASE);}
 return give(&t)&&ok;
}
void m3_live_retire(void){
 /* No flash/state reads: cleanup must still retire an admitted generation
  * after a save or native owner failure fenced new work. */
 uint32_t count=__atomic_load_n(&LIVE->admissions,__ATOMIC_ACQUIRE);
 do{if(!count){fence(10);return;}}while(!__atomic_compare_exchange_n(&LIVE->admissions,&count,count-1,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE));
}
uint32_t m3_live_hsfc_fine(uint32_t mode,uint32_t original){
 /* Original condition7 has already proved photo1D00==1 and HSFC1C26 in3..6.
  * Original C30 enum3 and C15 enum6 are distinct from external dial steps.
  * Other modes and an unavailable private selection keep the native byte. */
 if(mode!=3&&mode!=6)return original;
 uint64_t choice=__atomic_load_n(&LIVE->condition_selection,__ATOMIC_ACQUIRE);
 if((choice>>32)!=3&&(choice>>32)!=4)return original;
 uint32_t option=(uint32_t)choice;
 if(option==0||option==2||option==8)return 1;
 if(option==4||option==10)return 0;
 return original;
}
const struct m3_native_settings_schema *m3_live_worker_schema(const struct m3_native_settings_schema*s){
 if(!s||!ready()||s->count!=202||s->records!=(void*)(uintptr_t)0xa0295ffd8ull||s->auxiliary_count!=43||s->auxiliary!=(void*)(uintptr_t)0xa03f1df18ull)return s;
 uint32_t pending=2;
 if(!__atomic_compare_exchange_n(&LIVE->request_active,&pending,3,0,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE))return s;
 return &LIVE->request_schema;
}

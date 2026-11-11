#include "native_hud_publish.h"
#include "native_hud_quality.h"
#include "native_hud_remaining.h"
#include "native_live_settings.h"
#include "native_runtime_layout.h"
#ifndef FN
#define FN(a,t) ((t)(uintptr_t)(a))
#endif
#define TABLE 0xa029646a0ull
#define RECORD(i) ((uint8_t*)(uintptr_t)(TABLE+(i)*0x68ull))
static uint32_t r32(const void*p){return *(const uint32_t*)p;}
static void w32(void*p,uint32_t v){*(uint32_t*)p=v;}
static int plan(struct m3_hud_quality*p){
 uint64_t choice=__atomic_load_n(&((struct m3_live_settings*)(uintptr_t)
  M3_LIVE_SETTINGS_ADDRESS)->condition_selection,__ATOMIC_ACQUIRE);
 uint32_t photo=FN(0x406a3f54,uint32_t(*)(uint32_t))(0x1d00);
 uint32_t mode=FN(0x406a3f54,uint32_t(*)(uint32_t))(0x1c26);
 uint32_t special=FN(0x406a3f54,uint32_t(*)(uint32_t))(0x1c8c);
 return m3_hud_quality_plan(photo,mode,special,choice,p);
}
static uint32_t publish(uint8_t*record,uint32_t value){
 uint32_t*out=*(uint32_t**)(record+0x48);
 if(!out)out=(uint32_t*)(record+0x44);
 if(*out==value)return 0;
 *out=value;record[0x58]=1;return 1;
}
static int remaining_plan(uint32_t original,uint32_t*out){
 uint64_t choice=__atomic_load_n(&((struct m3_live_settings*)(uintptr_t)
  M3_LIVE_SETTINGS_ADDRESS)->condition_selection,__ATOMIC_ACQUIRE);
 return m3_hud_remaining_plan(
  FN(0x406a3f54,uint32_t(*)(uint32_t))(0x1d00),
  FN(0x406a3f54,uint32_t(*)(uint32_t))(0x1c26),
  FN(0x406a3f54,uint32_t(*)(uint32_t))(0x1c8c),
  r32((const void*)(uintptr_t)0xa03d9d218ull),choice,original,
  r32((const void*)(uintptr_t)0xa0293498cull),out);
}
uint32_t m3_hud_remaining_callback(uint32_t v,void*record){
 uint32_t value;
 if(record==RECORD(24)&&remaining_plan(v,&value))return publish(record,value);
 return FN(0x406aefa0,uint32_t(*)(uint32_t,void*))(v,record);
}
uint32_t m3_hud_quality_callback(uint32_t v,void*record){
 struct m3_hud_quality p;
 if((record==RECORD(112)||record==RECORD(113))&&plan(&p))
  return publish(record,p.published_code);
 return FN(0x406a5afc,uint32_t(*)(uint32_t,void*))(v,record);
}
uint32_t m3_hud_star_callback(uint32_t v,void*record){
 struct m3_hud_quality p;
 if(record==RECORD(116)&&plan(&p))return publish(record,p.star);
 return FN(0x406a5cb4,uint32_t(*)(uint32_t,void*))(v,record);
}
/* Native GUI queue mutex must already be held. The secondary GUI lock may
 * also be held, as in406B6F08. Duplicate indices and overflow follow the
 * native bounded queue/full-scan protocol; never grow the127-index array. */
static void queue_quality_locked(void){
 volatile uint32_t*n=(void*)(uintptr_t)0xa03ff6fd8ull;
 volatile uint32_t*indices=(void*)(uintptr_t)0xa03ff6fdcull;
 for(unsigned j=0;j<4;j++){
  uint32_t id=j==0?112:j==1?113:j==2?116:24;uint8_t*rec=RECORD(id);
  w32(rec+0x10,FN(0x406a3f54,uint32_t(*)(uint32_t))(r32(rec)));
  rec[0x29]=1;
  unsigned count=*n,found=0;
  if(count>127){*(volatile uint8_t*)(uintptr_t)0xa03ff6fd4ull=1;continue;}
  for(unsigned i=0;i<count;i++)if(indices[i]==id){found=1;break;}
  if(found)continue;
  if(count==127)*(volatile uint8_t*)(uintptr_t)0xa03ff6fd4ull=1;
  else {indices[count]=id;*n=count+1;}
 }
}
static void wake(void){FN(0x406a3ec4,void(*)(void*,uint32_t))((void*)(uintptr_t)0xa03ff6a30ull,4);}
uint32_t m3_hud_mode_callback(uint32_t v,void*record){
 uint32_t result=FN(0x406a62c8,uint32_t(*)(uint32_t,void*))(v,record);
 /* Exact record pointer hooks only; never globally replace this callback,
  * which also serves unrelated properties. The original mode stays intact. */
 if(record==RECORD(93)||record==RECORD(129)||record==RECORD(130)){
  queue_quality_locked();wake();
 }
 return result;
}
void m3_hud_selection_changed(void){
 /* Called only after the synchronous F worker joined and released its
  * owner. Match the original406B1FCC enqueue lock/event order. */
 FN(0x406a3eec,void(*)(void*))((void*)(uintptr_t)0xa03ff6a88ull);
 queue_quality_locked();
 FN(0x406a3ef0,void(*)(void*))((void*)(uintptr_t)0xa03ff6a88ull);
 wake();
}
void m3_hud_remaining_source_changed(void){
 uint32_t source=FN(0x406a3f54,uint32_t(*)(uint32_t))(0x2401),desired;
 if(!remaining_plan(source,&desired))return;
 FN(0x406a3eec,void(*)(void*))((void*)(uintptr_t)0xa03ff6a88ull);
 uint8_t*record=RECORD(24);uint32_t*out=*(uint32_t**)(record+0x48);
 if(!out)out=(uint32_t*)(record+0x44);
 int changed=*out!=desired;
 if(changed){
  /* Selection and mode use the same bounded native queue. This source hook
   * is needed when only the available budget changes and DSID2401 is equal. */
  volatile uint32_t*n=(void*)(uintptr_t)0xa03ff6fd8ull;
  volatile uint32_t*indices=(void*)(uintptr_t)0xa03ff6fdcull;
  w32(record+0x10,source);record[0x29]=1;
  uint32_t count=*n,found=0;
  if(count<=127)for(unsigned i=0;i<count;i++)if(indices[i]==24){found=1;break;}
  if(count>127||(!found&&count==127))*(volatile uint8_t*)(uintptr_t)0xa03ff6fd4ull=1;
  else if(!found){indices[count]=24;*n=count+1;}
 }
 FN(0x406a3ef0,void(*)(void*))((void*)(uintptr_t)0xa03ff6a88ull);
 if(changed)wake();
}

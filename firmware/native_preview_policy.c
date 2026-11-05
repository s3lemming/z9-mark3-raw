#include "native_preview_policy.h"
#include "native_live_settings.h"
#include "native_runtime_layout.h"
uint32_t m3_preview_kind(uint32_t kind,uint32_t metadata_kind,
 uint32_t mode,uint32_t special,uint64_t selection){
 uint32_t codec=(uint32_t)(selection>>32);
 if(kind==0&&metadata_kind==3&&(mode==3||mode==6)&&!special&&
    (codec==3||codec==4)&&(uint32_t)selection==0)return 1;
 return kind;
}
#ifndef M3_PREVIEW_POLICY_TEST
static uint32_t read32(uint64_t a){return *(const uint32_t*)(uintptr_t)a;}
void m3_native_preview_parameters(uint64_t r,uint32_t mode,uint32_t kind,
 uint64_t statistics,uint64_t parameters){
 uint64_t m=r?*(const uint64_t*)(uintptr_t)(r+0x790):0;
 if(m){
  uint64_t selection=__atomic_load_n(&((struct m3_live_settings*)(uintptr_t)
   M3_LIVE_SETTINGS_ADDRESS)->condition_selection,__ATOMIC_ACQUIRE);
  /* Use the current frame's M release mode, never the physical dial or a
   * residual Quick selection. Native mode (W1) remains unmodified. */
  kind=m3_preview_kind(kind,read32(m),read32(m+0x24a0),
   ((uint32_t(*)(uint32_t))(uintptr_t)0x403e74a8)(0x1c8c),selection);
 }
 ((void(*)(uint64_t,uint32_t,uint32_t,uint64_t,uint64_t))(uintptr_t)
  0x40eb2d9c)(r,mode,kind,statistics,parameters);
}
#endif

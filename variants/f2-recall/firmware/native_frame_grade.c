#include "native_frame_grade.h"
#include "native_live_settings.h"
#include "native_runtime_layout.h"
#ifdef M3_FRAME_GRADE_TEST
extern uint64_t m3_frame_grade_test_selection;
extern uint32_t m3_frame_grade_test_query(uint32_t);
#define CHOICE (&m3_frame_grade_test_selection)
#define QUERY(k) m3_frame_grade_test_query(k)
#else
#define CHOICE (&((struct m3_live_settings*)(uintptr_t)M3_LIVE_SETTINGS_ADDRESS)->condition_selection)
#define QUERY(k) ((uint32_t(*)(uint32_t))(uintptr_t)0x403e74a8)(k)
#endif
uint32_t m3_native_frame_grade_snapshot(uint32_t original){
 /* Preserve unavailable, future and RAW-only selections. RAW-only does not
  * request a standalone JPEG grade; keep its original native preview choice. */
 uint64_t choice=__atomic_load_n(CHOICE,__ATOMIC_ACQUIRE);
 uint32_t option=(uint32_t)choice;
 if(original>1||((choice>>32)!=3&&(choice>>32)!=4)||
    (option!=2&&option!=4&&option!=8&&option!=10))return original;
 /* The wrapper additionally checks M kind3. Native effective mode fences
  * C60/C120, non-photo processing and residual Quick selections. */
 if(QUERY(0x1d00)!=1)return original;
 uint32_t mode=QUERY(0x1c26);
 if((mode!=3&&mode!=6)||QUERY(0x1c8c)!=0)return original;
 return option==2||option==8?0:1;
}
uint32_t m3_native_imageinfo_grade_snapshot(uint32_t original,uint32_t kind){
 return kind==3?m3_native_frame_grade_snapshot(original):original;
}

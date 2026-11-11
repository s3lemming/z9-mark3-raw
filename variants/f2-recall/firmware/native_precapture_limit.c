#include "native_precapture_limit.h"
#include "native_live_settings.h"
#include "native_runtime_layout.h"
#ifdef M3_PRECAPTURE_TEST
extern uintptr_t m3_precapture_test_function(uintptr_t);
extern uint64_t m3_precapture_test_selection(void);
extern uint32_t m3_precapture_test_applied_mode(void);
#define FN(a,t) ((t)m3_precapture_test_function(a))
#define SELECTION() m3_precapture_test_selection()
#define APPLIED_MODE() m3_precapture_test_applied_mode()
#else
#define FN(a,t) ((t)(uintptr_t)(a))
#define SELECTION() __atomic_load_n(&((struct m3_live_settings*)(uintptr_t)M3_LIVE_SETTINGS_ADDRESS)->condition_selection,__ATOMIC_ACQUIRE)
#define APPLIED_MODE() (*(const uint32_t*)(uintptr_t)0xa03d9d218ull)
#endif
/* The existing global requested duration remains native and cross-version
 * compatible. Only the effective C30 FX HE-star duration is bounded. Thus
 * moving back to C15/DX/HE restores the requested duration without inventing
 * a second persistent preference or rewriting an unrelated mode's setting.
 * UI getter and the one native frame-count conversion use the same cap.
 * Never clear the native blocked flag, subtract its12-frame reserve, or
 * enlarge pool39. Menu enum2 is0.5s; enum3 is1s, not a frame count. */
static int limited(uint32_t mode){
 uint64_t choice=SELECTION();uint32_t option=(uint32_t)choice;
 if((choice>>32)!=3||(option!=0&&option!=2&&option!=4)||mode!=3)return 0;
 return FN(0x406a3f54,uint32_t(*)(uint32_t))(0x1d00)==1&&
        FN(0x406a3f54,uint32_t(*)(uint32_t))(0x603)==0&&
        FN(0x406a3f54,uint32_t(*)(uint32_t))(0x1c8c)==0;
}
uint32_t m3_precapture_property_get(uint32_t id){
 /* Exact unpatched destination of original403E74A8; no recursive getter. */
 uint32_t original=FN(0x410db64c,uint32_t(*)(uint32_t))(id);
 if(id!=0x02006401||original!=3)return original;
 uint32_t mode=FN(0x406a3f54,uint32_t(*)(uint32_t))(0x1c26);
 return limited(mode)?2:original;
}
uint32_t m3_precapture_frames(uint32_t requested){
 /* Exact original40F992B0 consumer, after original service property read.
  * Applied release identity, rather than a stale GUI Quick value, owns this
  * conversion. Original conversion still chooses the native per-mode table. */
 if(requested==3&&limited(APPLIED_MODE()))requested=2;
 return FN(0x40f9d61c,uint32_t(*)(uint32_t))(requested);
}
void m3_precapture_menu_refresh(void *ctx){
 if(ctx==(void*)(uintptr_t)0xa02905f50ull){
  uint32_t mode=FN(0x406a3f54,uint32_t(*)(uint32_t))(0x1c26);
  /* Restore native eligibility before its focus/mark callback. Otherwise
   * leaving the cap can ask native focus to select a still-disabled1s row. */
  FN(0x403b1f74,void(*)(void*,uint32_t,uint32_t))(ctx,0x35be,limited(mode));
 }
 FN(0x40525404,void(*)(void*))(ctx);
}
void m3_precapture_selection_changed(void){
 /* Publish through the original ID/value subscriber queue41017C30/C54,
  * whose registered040C0000 consumer40F98BD4 marks6401 dirty and re-reads
  * duration before40F992B0. The UI change-list publisher40EC85C4 did not
  * reach this consumer for an unchanged stored value in actual v23.
  * Called only after F worker join and request-owner release. No setter,
  * direct frame-cache writes, global getter hook, or buffer-guard bypass. */
 const uint32_t id=0x02006401;
 uint32_t requested=FN(0x410db64c,uint32_t(*)(uint32_t))(id);
 if(requested>3)return;
 uint32_t effective=requested==3&&limited(APPLIED_MODE())?2:requested;
 FN(0x41017c30,uint32_t(*)(uint32_t,uint32_t))(id,effective);
}
uint32_t m3_precapture_native_batch(const uint32_t *ids,const uint32_t *values,uint32_t count){
 /* Only original41017ED0 is redirected. Its borrowed arrays remain owned
  * until41017ED8 releases the message. Complete the original publication
  * and release its queue mutex before requesting another native update.
  * Original MAIN actual FX/DX messages prove full ID07000603, not DSID603
  * or a guessed typed property ID. Keep original return and all messages. */
 uint32_t rc=FN(0x41017c54,uint32_t(*)(const uint32_t*,const uint32_t*,uint32_t))(ids,values,count);
 if(rc||!ids||!values||!count||count>512)return rc;
 uint64_t choice=SELECTION();uint32_t option=(uint32_t)choice;
 if((choice>>32)!=3||(option!=0&&option!=2&&option!=4)||APPLIED_MODE()!=3)return rc;
 for(uint32_t i=0;i<count;i++){
  if(ids[i]!=0x07000603||values[i]>1)continue;
  if(FN(0x406a3f54,uint32_t(*)(uint32_t))(0x1d00)==1&&
     FN(0x406a3f54,uint32_t(*)(uint32_t))(0x603)<=1&&
     FN(0x406a3f54,uint32_t(*)(uint32_t))(0x1c8c)==0)
   m3_precapture_selection_changed();
  break;
 }
 return rc;
}

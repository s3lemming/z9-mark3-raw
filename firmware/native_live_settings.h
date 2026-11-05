#ifndef MARK3_NATIVE_LIVE_SETTINGS_H
#define MARK3_NATIVE_LIVE_SETTINGS_H
#include "native_settings_context.h"
#include "native_settings_load.h"
/* Integration state owns the bounded tail of the first startup-zeroed page,
 * before the audio anchor. The second page owns UI; native TTBR0 is unchanged. */
enum m3_live_state {M3_LIVE_COLD,M3_LIVE_LOADING,M3_LIVE_PRESENT,
                   M3_LIVE_LEGACY,M3_LIVE_FAILED};
struct m3_live_settings {
 uint32_t state,fault,bootstrap_calls,save_calls;
 uint32_t saves_before_bootstrap,accepted,failed,request_active;
 uint32_t request_result,admissions,load_result,original_load_result;
 struct m3_native_gate gate;
 struct m3_hsfc_settings committed,desired;
 uint64_t expected_generation,bootstrap_SP;
 struct m3_native_settings_schema request_schema;
 struct m3_native_load_receipt load_receipt;
 /* One release-published value for native condition readers that cannot
  * block on a settings transaction. Zero is unavailable; upper32 is menu codec HE4 or HE-star3. */
 uint64_t condition_selection;
};
#include "native_runtime_layout.h"
_Static_assert(sizeof(struct m3_live_settings)<=M3_LIVE_SETTINGS_LIMIT,"Settings reservation exhausted");
void m3_live_bootstrap_before(uint64_t);
void m3_live_bootstrap_after(uint32_t);
uint32_t m3_native_save_begin(const struct m3_native_storage_descriptor*,
 const struct m3_native_settings_schema*,struct m3_native_save_frame*);
int m3_live_read(void*,struct m3_hsfc_settings*);
int m3_live_commit(void*,uint64_t,const struct m3_hsfc_settings*);
/* Admission and menu commit use the same native owner. A successful begin
 * owns one admission until end, even when native E retention later fails. */
int m3_live_admit(struct m3_hsfc_settings*);
void m3_live_retire(void);
int m3_live_busy(void*);
uint32_t m3_live_hsfc_fine(uint32_t,uint32_t);
const struct m3_native_settings_schema *m3_live_worker_schema(const struct m3_native_settings_schema*);
#endif

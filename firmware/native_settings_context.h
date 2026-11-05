#ifndef MARK3_NATIVE_SETTINGS_CONTEXT_H
#define MARK3_NATIVE_SETTINGS_CONTEXT_H
#include "native_settings_save.h"
#include "native_gate.h"
#include "native_settings_context_abi.h"
struct m3_native_save_binding;
struct m3_native_save_frame {
 struct m3_native_save_cookie cookie;
 struct m3_native_save_api api;
 struct m3_native_gate_token token;
 const struct m3_native_save_binding *binding;
};
struct m3_native_save_binding {
 const struct m3_native_gate_api *gate_api;
 struct m3_native_gate *gate;
 void *user;
 /* Templates supply select/verify/accept/failed/tail methods. All methods
  * receive THIS frame as user; shared state is binding->user. Template user,
  * acquire/release and tail.context must beNULL, never a shared active cookie. */
 struct m3_native_save_api callbacks;
 /* Must atomically fence private reads/admission on an unknown/failed
  * unlock. Owner may be lost/still held: no owned-only API, I/O or retry. */
 void (*release_failed)(struct m3_native_save_frame*);
};
_Static_assert(sizeof(struct m3_native_save_frame)==M3_NATIVE_SAVE_FRAME_BYTES,"Assembly-owned invocation frame");
_Static_assert(offsetof(struct m3_native_save_frame,cookie)==0,"X28 cookie prefix");
_Static_assert(offsetof(struct m3_native_save_frame,api)==96,"Owned callback API");
_Static_assert(offsetof(struct m3_native_save_frame,token)==176,"Owned gate token");
/* Caller owns a disjoint, aligned invocation stack frame through save_end.
 * Does not initialize the live gate or bind native settings/NOR providers.
 * Integration's m3_native_save_begin(desc,schema,frame) calls this function
 * with an immutable binding. ALL F core invocations still need interception. */
uint32_t m3_native_save_context_prepare(const struct m3_native_save_binding*,const struct m3_native_storage_descriptor*,const struct m3_native_settings_schema*,struct m3_native_save_frame*);
int m3_native_save_context_owned(const struct m3_native_save_frame*);
#endif

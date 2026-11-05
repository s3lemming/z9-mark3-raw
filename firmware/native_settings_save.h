#ifndef MARK3_NATIVE_SETTINGS_SAVE_H
#define MARK3_NATIVE_SETTINGS_SAVE_H
#include "native_settings_abi.h"
#include "hsfc_settings_tail.h"
enum m3_save_selection {M3_SAVE_NATIVE,M3_SAVE_ENCODE,M3_SAVE_REFUSE};
struct m3_native_save_cookie;
struct m3_native_save_api {
 void *user;
 /* The real transaction owner must serialize ALL storage0F core invocations,
  * including ordinary native saves. Physical program-command locking alone
  * cannot implement this. The native binding is not supplied by this module. */
 int (*acquire)(void*);
 /* Return1 only after a successful owner release. If native release fails,
  * fence private reads/admission before returning0: the kernel owner may
  * still be held or unavailable. No I/O, nested save or blind unlock retry.
  * end() propagates0 and retires the cookie; it cannot prove an unlock. */
 int (*release)(void*);
 /* Coherent validated snapshot or explicit owned request schema. NATIVE is
  * only for an established absent/unowned extension; never silently classify
  * corrupt/future records as absent. Force1 requests durable private change. */
 enum m3_save_selection (*select)(void*,const struct m3_native_settings_schema*,
  struct m3_hsfc_settings*,uint32_t *force);
 struct m3_tail_api tail;
 /* Under the same transaction: prove selected native bank/counter, ALL
  * current native records AND exact private settings. For no-write native
  * returns, prove the existing committed snapshot, not just a valid footer.
  * This callback must not publish settings. */
 int (*verify)(void*,const struct m3_native_save_cookie*);
 /* Called ONLY after verification, while the SAME F transaction is still
  * owned, before release. Publish a coherent committed record/request receipt
  * here so an ordinary concurrent F save cannot snapshot the old selection.
  * The integration must hold its admission/request gate throughout and do
  * no nested save/I/O here. On failure, mark the private state unavailable
  * until reload; flash may already contain the new verified record. */
 int (*accept)(void*,const struct m3_native_save_cookie*);
 /* Any refused/failed extension invocation that acquired ownership reaches
  * this callback BEFORE release. Fence private reads/further saves until a
  * native accepted-bank reload, since failure after tail commit may leave
  * a different durable record than RAM. Never write flash or nest a save. */
 void (*failed)(void*,const struct m3_native_save_cookie*);
};
/* Exact stack layout shared with native_settings_save.S. No original frame,
 * padding, thread register or global cookie is repurposed. */
struct m3_native_save_cookie {
 const struct m3_native_storage_descriptor *descriptor;
 const struct m3_native_settings_schema *schema;
 const struct m3_native_save_api *api;
 struct m3_hsfc_settings settings;
 struct m3_tail_receipt tail_receipt;
 uint32_t enabled,force,locked,tail_called,tail_result,original_status,counter;
 uint64_t cursor,magic;
};
_Static_assert(sizeof(struct m3_native_save_cookie)==96,"Save wrapper stack cookie");
_Static_assert(offsetof(struct m3_native_save_cookie,enabled)==52,"Save wrapper enabled offset");
/* Return enabled0 native /1 extension /2 refused. Cookie lifetime includes
 * the whole original core call. End always releases any acquired ownership. */
uint32_t m3_native_save_prepare(const struct m3_native_save_api*,
 const struct m3_native_storage_descriptor*,const struct m3_native_settings_schema*,
 struct m3_native_save_cookie*);
uint32_t m3_native_save_force(struct m3_native_save_cookie*,uint32_t original);
void m3_native_save_footer(struct m3_native_save_cookie*,uint64_t,uint32_t);
uint32_t m3_native_save_end(struct m3_native_save_cookie*,uint32_t original);
/* Assembly passes a208B owned frame whose first96B are this cookie to the
 * integration entry m3_native_save_begin(desc,schema,frame). See
 * native_settings_context.h for per-call API/token preparation. No live
 * binding or hidden global implementation of begin is supplied here. */
#endif

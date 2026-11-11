#ifndef MARK3_NATIVE_MENU_CALLBACKS_H
#define MARK3_NATIVE_MENU_CALLBACKS_H
#include "native_menu_pages.h"
#include "hsfc_settings_record.h"
#include "format_policy.h"
struct m3_native_menu_event {
 void *context;uint32_t event,target;uint16_t control;uint8_t reserved[6];
};
struct m3_native_menu_ui {
 struct m3_native_pages *pages;
 uint32_t contracts; /* Integration supplies proved contracts, never test readiness. */
 void *user;
 /* One coherent, fully decoded/validated settings record under its native
  * owner/read lock. Missing, corrupt or unknown records return0; never migrate
  * them implicitly while refreshing a page. */
 int (*read_committed)(void*,struct m3_hsfc_settings*);
 int (*busy)(void*);
 /* Under the REAL native settings/owner lock: recheck expected generation
  * and in-flight state, persist, then publish. Return1 only after all succeed.
  * An unchanged selection still revalidates under that lock, without a NOR
  * rewrite. No RAM-only setter, silent unknown-record migration or ordinary property
  * write can implement this callback. That binding is not supplied here. */
 int (*commit)(void*,uint64_t,const struct m3_hsfc_settings*);
 uint32_t (*entry_style)(void);
 void (*focus)(void*,uint32_t);
 void (*mark)(void*,uint32_t,uint32_t);
};
enum m3_menu_ui_result {M3_UI_APPLIED,M3_UI_NOT_READY,M3_UI_INVALID,M3_UI_BUSY,M3_UI_COMMIT_FAILED};
/* Original refresh takes context directly; original submit takes a24-byte
 * event whose first field is that context and whose target is at+12.
 * Functions below do not install callbacks, infer locks, or publish widgets. */
enum m3_menu_ui_result m3_native_c15_refresh(const struct m3_native_menu_ui*,void*);
enum m3_menu_ui_result m3_native_c15_submit(const struct m3_native_menu_ui*,struct m3_native_menu_event*);
#endif

#ifndef MARK3_NATIVE_HE_MENU_H
#define MARK3_NATIVE_HE_MENU_H
#include "native_menu_router.h"
/* Actual HE candidate, distinct from the frozen JPEG-only diagnostic. The
 * five private rows share one independently loaded/persisted HE setting.
 * The two C60/C120 rows retain their original native property and actions. */
struct m3_he_menu_runtime {
 uint32_t init_state,fault,constructor_calls,private_lookup_calls;
 uint32_t blocked_events,submits,group_refreshes,leaf_refreshes;
 struct m3_native_pages pages;
 struct m3_native_menu_router router;
};
_Static_assert(sizeof(struct m3_he_menu_runtime)<=4096,"UI page exhausted");
void *m3_native_ctor_lookup_entry(uint32_t,uint32_t,const void*);
void *m3_he_menu_global_lookup(uint32_t,uint32_t);
void m3_he_menu_group_refresh(void*);
void m3_he_menu_group_values(void*);
void m3_he_menu_leaf_refresh(void*);
int m3_he_menu_group_condition(struct m3_native_menu_event*);
void m3_he_menu_group_open(struct m3_native_menu_event*);
int m3_he_menu_leaf_condition(struct m3_native_menu_event*);
void m3_he_menu_leaf_submit(struct m3_native_menu_event*);
#endif

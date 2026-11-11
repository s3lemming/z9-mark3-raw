#ifndef F2_RECALL_NATIVE_H
#define F2_RECALL_NATIVE_H
#include <stdint.h>
#include "native_menu_callbacks.h"
enum {F2R_LEAF_ID=0x6c6,F2R_HEAP_OFFSET=8320,F2R_HEAP_BYTES=1920};
void *f2r_menu_lookup(uint32_t,uint32_t,const void*,uint32_t);
void f2r_parent_refresh(void*);
void f2r_parent_values(void*);
void f2r_release_refresh(void*);
void f2r_release_submit(struct m3_native_menu_event*);
void f2r_vr_toggle(struct m3_native_menu_event*);
void f2r_vr_open(struct m3_native_menu_event*);
void f2r_vr_aux(struct m3_native_menu_event*);
void f2r_ch_submit(struct m3_native_menu_event*);
void f2r_cl_submit(struct m3_native_menu_event*);
uint32_t f2r_getter_post(uint32_t,uint32_t*,uint32_t);
void f2r_enter(const void*);
void f2r_exit(void);
uint32_t f2r_vr_override_active(void);
int32_t f2r_save_batch(const uint32_t*,const uint32_t*,uint32_t);
void f2r_save_notify(uint32_t,const uint32_t*,uint32_t);
uint32_t f2r_save_permission(void);
void f2r_save_current_action(struct m3_native_menu_event*);
#endif

#ifndef MARK3_NATIVE_MENU_PAGES_H
#define MARK3_NATIVE_MENU_PAGES_H
#include <stdint.h>
/* Native state-machine ancestors are preserved; visual nesting/back belongs
 * to native navigation history. Headers are immutable payload data.
 * Creation-time owned templates only. Nothing here installs hooks, allocates
 * state IDs/RAM, publishes a widget, commits settings or claims readiness. */
struct m3_native_page_templates {
 uint8_t group_state[96],leaf_state[96],group_holder[80],leaf_holder[80];
 uint8_t group_callbacks[40],leaf_callbacks[40];
 uint8_t group_rows[2][44],leaf_rows[2][44],events[3][16][32];
 uint32_t event_counts[3];
};
struct m3_native_page_bindings {
 /* Caller must reserve both IDs with its bounded provider AND navigation
  * adapter; ordinary native navigation ignores new targets above6B8. */
 uint32_t c15_state,c60_state;
 uint64_t group_callbacks[5],c15_callbacks[5];
 uint64_t group_aux_action,c15_submit;
};
struct m3_native_pages {
 uint32_t magic,version;
 uint8_t states[3][96],holders[3][80],callbacks[3][40];
 uint8_t rows[3][7][44];
 uint64_t event_pointers[3][7];
 uint8_t events[4][16][32];
};
enum m3_page_result {M3_PAGE_OK,M3_PAGE_INVALID,M3_PAGE_BUSY};
/* Caller owns serialized initialization; no native reader before publication.
 * Output must be fresh startup-owned aligned zero RAM, disjoint from both inputs.
 * Successful creation consumes it once; rebuilding a live page is rejected.
 * All validity checks finish before the first output write. */
enum m3_page_result m3_native_pages_build(struct m3_native_pages*,
 const struct m3_native_page_templates*,const struct m3_native_page_bindings*);
#endif

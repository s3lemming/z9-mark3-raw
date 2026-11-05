#ifndef MARK3_NATIVE_MENU_ROUTER_H
#define MARK3_NATIVE_MENU_ROUTER_H
#include "native_menu_callbacks.h"
/* Constructor-context routing and a separate bounded generic lookup provider.
 * Neither function installs a global hook or changes native navigation bounds.
 * IDs are caller-reserved. original_lookup must be an unhooked native entry or
 * proved trampoline; pointing it to a hook which calls this API recurses. */
struct m3_native_menu_router {
 struct m3_native_pages *pages;
 const void *original_parent;
 uint32_t c15_state,c60_state,magic,enabled;
 void *(*original_lookup)(uint32_t,uint32_t);
 void (*original_enter)(struct m3_native_menu_event*);
 void (*construct)(uint32_t,uint32_t);
 int (*original_condition)(struct m3_native_menu_event*);
};
enum m3_route_result {M3_ROUTE_OK,M3_ROUTE_INVALID,M3_ROUTE_BUSY,M3_ROUTE_DISABLED};
/* Startup-owned zero router and freshly built pages, before any widget or
 * reader. Validates first, then replaces eight owned group actions and their
 * conditions. A false original condition must set6B8, because the original
 * dispatcher otherwise still navigates after skipping the action.
 * This creates no firmware hook or native UI/NOR readiness qualification. */
enum m3_route_result m3_native_router_bind(struct m3_native_menu_router*,
 struct m3_native_pages*,const void *original_parent,uint32_t,uint32_t,
 void *(*lookup)(uint32_t,uint32_t),void (*enter)(struct m3_native_menu_event*),
 void (*construct)(uint32_t,uint32_t),int (*condition)(struct m3_native_menu_event*),
 uint64_t owned_open_action,uint64_t owned_condition);
void *m3_native_router_lookup(const struct m3_native_menu_router*,uint32_t,
 uint32_t,const void *previous_context);
/* Resolves only the two owned leaf IDs in slot2; unavailable returnsNULL.
 * All ordinary IDs/other slots retain original argument values/lookup. A real
 * generic-getter patch, trampoline, ID allocation and live UI remain unbound. */
void *m3_native_router_global_lookup(const struct m3_native_menu_router*,uint32_t,uint32_t);
enum m3_route_result m3_native_router_open(const struct m3_native_menu_router*,
 struct m3_native_menu_event*);
int m3_native_router_condition(const struct m3_native_menu_router*,
 struct m3_native_menu_event*);
#endif

#ifndef MARK3_NATIVE_SETTINGS_NOR_H
#define MARK3_NATIVE_SETTINGS_NOR_H
#include "native_settings_context.h"
/* Exact tail callbacks. context is this invocation's208B frame, not a global
 * settings object. Only its already-reserved88B tail is accessible; no erase,
 * bank arbitration, bootstrap or general-purpose flash interface is provided.
 * Native40CA8374 returns0 after its worker event; W2 counts halfwords. */
int m3_native_tail_program(void*,uint64_t,uint32_t);
int m3_native_tail_read(void*,uint64_t,void*,size_t);
#endif

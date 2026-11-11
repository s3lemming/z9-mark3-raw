#include "native_hud_publish.h"
#include <stdint.h>
#ifndef FN
#define FN(a,t) ((t)(uintptr_t)(a))
#endif
void m3_native_buffer_publish(uint32_t frames,uint32_t flags){
 /* Exact original40F9C754 call. Original counters, parameter publication,
  * cached comparison and allocation policy complete before any GUI enqueue. */
 FN(0x40f9cd08,void(*)(uint32_t,uint32_t))(frames,flags);
 m3_hud_remaining_source_changed();
}

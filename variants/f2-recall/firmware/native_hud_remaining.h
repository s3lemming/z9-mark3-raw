#ifndef M3_NATIVE_HUD_REMAINING_H
#define M3_NATIVE_HUD_REMAINING_H
#include <stdint.h>
/* Original adjusted available frame budget, not card capacity or byte count.
 * Leave native allocation/capture policy intact. GUI-only conservative bound. */
int m3_hud_remaining_plan(uint32_t photo,uint32_t mode,uint32_t special,
 uint32_t latched_mode,uint64_t choice,uint32_t original,uint32_t available,
 uint32_t *out);
#endif

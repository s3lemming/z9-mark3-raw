#ifndef M3_NATIVE_VIRTUAL_NRMAP_H
#define M3_NATIVE_VIRTUAL_NRMAP_H
#include <stddef.h>
#include <stdint.h>
/* Explicit synthetic-map compatibility producer, not the original DSP op6.
 * Caller owns Y and payload until the native NEF writer/commit/cancel joins.
 * Only an absent, uncompleted private map may be replaced. On rejection no
 * byte changes; valid native completed maps must instead use the native copy.
 * Fills the full native FX/DX canvas, then publishes descriptor and Y21 last.
 * Pixel0/128 have no established neutral/image-quality meaning. */
size_t m3_virtual_nrmap_fill(void *private_Y,size_t Y_bytes,uint32_t area,
 void *owned_payload,size_t capacity,uint32_t value);
#endif

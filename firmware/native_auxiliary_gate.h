#ifndef M3_NATIVE_AUXILIARY_GATE_H
#define M3_NATIVE_AUXILIARY_GATE_H
#include <stdint.h>
#include <stddef.h>
int m3_auxiliary_scope(uint32_t kind,uint32_t mode,uint32_t area,uint64_t selection);
uint32_t m3_auxiliary_native_scope(const void *M);
/* Completed original Y auxiliary only. Copy full native canvas including its
 * stride/crop into a private frame owner. No synthetic pixels or other photo.
 * 0 means absent/unqualified and leaves destination unchanged. */
size_t m3_auxiliary8_copy(const void *Y,size_t n,uint32_t area,void *descriptor,
 void *payload,size_t room);
#endif

#ifndef M3_NATIVE_SHOTINFO_GROUP1723_H
#define M3_NATIVE_SHOTINFO_GROUP1723_H
#include <stddef.h>
#include <stdint.h>
enum {M3_GROUP1723_METADATA_BYTES=0x4480,M3_GROUP1723_RETAIN_STAGE=1};
/* M17's native short form is four bytes when M24B8 is nonzero. */
struct m3_shotinfo_group1723_sample {
 uint64_t generation,source_M;
 uint32_t counter,sequence,mask,stage;
 uint8_t payload17[5632],payload23[48];
 _Alignas(8) uint8_t shadow_M[M3_GROUP1723_METADATA_BYTES];
};
typedef unsigned(*m3_shotinfo_group1723_ready)(const void*,unsigned);
typedef void(*m3_shotinfo_group1723_producer)(void*,unsigned);
unsigned m3_shotinfo_group1723_native_ready(const void*,unsigned);
void m3_shotinfo_group1723_native_build(void*,unsigned);
int m3_shotinfo_group1723_capture(struct m3_shotinfo_group1723_sample*,
 uint64_t,const void*,size_t,m3_shotinfo_group1723_ready,m3_shotinfo_group1723_producer);
int m3_shotinfo_group1723_publish(const struct m3_shotinfo_group1723_sample*,
 uint64_t,const void*,size_t,void*,size_t);
#endif

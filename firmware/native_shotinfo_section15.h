#ifndef MARK3_NATIVE_SHOTINFO_SECTION15_H
#define MARK3_NATIVE_SHOTINFO_SECTION15_H
#include <stddef.h>
#include <stdint.h>
/* Original40F6DE38 reads30B from the native service cache; the original
 * M15 builder takes the final8B, with no mutation or state consumption.
 * Retain the native current-cache semantics, not sensor-time alignment. */
struct m3_shotinfo_section15_sample {
 uint64_t generation,source_M;
 uint32_t counter,sequence,bytes,stage;
 uint8_t payload[8];
};
typedef void (*m3_shotinfo_section15_producer)(void*);
enum {M3_SECTION15_RETAIN_STAGE=1,M3_SECTION15_METADATA_BYTES=0x4480};
int m3_shotinfo_section15_capture(struct m3_shotinfo_section15_sample*,uint64_t,
 const void*,size_t,m3_shotinfo_section15_producer);
int m3_shotinfo_section15_publish(const struct m3_shotinfo_section15_sample*,
 uint64_t,const void*,size_t,void*,size_t);
#endif

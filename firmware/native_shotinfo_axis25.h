#ifndef MARK3_NATIVE_SHOTINFO_AXIS25_H
#define MARK3_NATIVE_SHOTINFO_AXIS25_H
#include <stddef.h>
#include <stdint.h>
/* Original40C8C7A0 serializes the axis service's current global snapshot.
 * Capture at retained-E admission, never at delayed NEF composition. This
 * preserves the original S reader's semantics; it does not establish that
 * the service history is atomic or aligned to the sensor exposure time. */
struct m3_shotinfo_axis25_sample {
 uint64_t generation,source_M;
 uint32_t counter,sequence,bytes,stage;
 uint8_t payload[108];
};
typedef void (*m3_shotinfo_axis25_producer)(void*,uint32_t*);
enum {M3_AXIS25_RETAIN_STAGE=1,M3_AXIS25_METADATA_BYTES=0x4480};
int m3_shotinfo_axis25_capture(struct m3_shotinfo_axis25_sample*,uint64_t,
 const void*,size_t,m3_shotinfo_axis25_producer);
int m3_shotinfo_axis25_publish(const struct m3_shotinfo_axis25_sample*,
 uint64_t,const void*,size_t,void*,size_t);
#endif

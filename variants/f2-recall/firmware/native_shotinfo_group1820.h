#ifndef MARK3_NATIVE_SHOTINFO_GROUP1820_H
#define MARK3_NATIVE_SHOTINFO_GROUP1820_H
#include <stddef.h>
#include <stdint.h>
/* Whole original40F537A0, with owned M18/M19/M20 destinations and M15
 * disabled in a shadow M. It retains the original current-service-cache
 * semantics, including its two shared mirror writes. No event consumption,
 * donor data, exposure-time alignment or atomic global snapshot is claimed. */
struct m3_shotinfo_group1820_sample {
 uint64_t generation,source_M;
 uint32_t counter,sequence,mask,stage;
 uint8_t payload18[500],payload19[160],payload20[464];
 _Alignas(8) uint8_t shadow_M[0x4480];
};
typedef void (*m3_shotinfo_group1820_producer)(void*);
enum {M3_GROUP1820_RETAIN_STAGE=1,M3_GROUP1820_METADATA_BYTES=0x4480};
int m3_shotinfo_group1820_capture(struct m3_shotinfo_group1820_sample*,uint64_t,
 const void*,size_t,m3_shotinfo_group1820_producer);
int m3_shotinfo_group1820_publish(const struct m3_shotinfo_group1820_sample*,
 uint64_t,const void*,size_t,void*,size_t);
#endif

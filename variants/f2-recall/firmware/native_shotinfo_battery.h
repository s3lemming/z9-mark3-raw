#ifndef MARK3_NATIVE_SHOTINFO_BATTERY_H
#define MARK3_NATIVE_SHOTINFO_BATTERY_H
#include <stddef.h>
#include <stdint.h>
/* Original 5.32 slot26 is serialized by40EBE56C from BatteryCheckService.c.
 * A sample belongs to one retained frame, never a later compose-time global.
 * This is an additive private NEF section, not an image algorithm repair. */
struct m3_shotinfo_battery_sample {
 uint64_t generation,source_M;
 uint32_t counter,sequence,bytes,stage;
 uint8_t payload[8];
};
typedef void (*m3_shotinfo_battery_producer)(void*,uint32_t*);
enum {M3_BATTERY_RETAIN_STAGE=1,M3_BATTERY_METADATA_BYTES=0x4480};
int m3_shotinfo_battery_capture(struct m3_shotinfo_battery_sample*,uint64_t,
 const void*,size_t,m3_shotinfo_battery_producer);
int m3_shotinfo_battery_publish(const struct m3_shotinfo_battery_sample*,
 uint64_t,const void*,size_t,void*,size_t);
#endif

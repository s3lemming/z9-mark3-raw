#ifndef MARK3_NATIVE_SHOTINFO_BANK12_H
#define MARK3_NATIVE_SHOTINFO_BANK12_H
#include "native_shotinfo_bank_snapshot.h"
/* An owned retained-frame sample. No borrowed bank pointer survives capture.
 * Integration into the native worker/allocator is a separate qualification. */
struct m3_shotinfo_bank12_sample {
 uint64_t generation,source_M;
 uint32_t counter,sequence,bytes,stage;
 uint8_t payload[800];
};
enum {M3_BANK12_METADATA_BYTES=0x4480,M3_BANK12_RETAIN_STAGE=1};
int m3_shotinfo_bank12_capture(struct m3_shotinfo_bank12_sample*,uint64_t,
 const void*,size_t,const volatile void*,const struct m3_bank_mutex_ops*);
int m3_shotinfo_bank12_publish(const struct m3_shotinfo_bank12_sample*,uint64_t,
 const void*,size_t,void*,size_t);
#endif

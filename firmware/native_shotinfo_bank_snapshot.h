#ifndef M3_NATIVE_SHOTINFO_BANK_SNAPSHOT_H
#define M3_NATIVE_SHOTINFO_BANK_SNAPSHOT_H
#include <stddef.h>
#include <stdint.h>
/* Source-copy primitive only; not linked into the active firmware candidate.
 * Caller must own destination storage and bind it to the captured frame.
 * Native banks0/1 contain2/8 source bytes, NOT serialized M0/M5 sections.
 * Bank2 contains800 source bytes copied directly into native M12. */
struct m3_bank_mutex_ops {
 void (*lock)(void *native_mutex,void *opaque);
 void (*unlock)(void *native_mutex,void *opaque);
 void *opaque;
};
enum {M3_BANK_INVALID=-1,M3_BANK_UNAVAILABLE=0,M3_BANK_COPIED=1};
int m3_bank_snapshot_current(const volatile void *descriptor,size_t descriptor_bytes,
 uint32_t expected_bytes,void *owned_output,size_t output_bytes,
 const struct m3_bank_mutex_ops *mutex);
/* Validate the entire owned record against native banks, including a header
 * before the payload. No record byte is written when validation fails. */
int m3_bank_snapshot_record(const volatile void *descriptor,size_t descriptor_bytes,
 uint32_t expected_bytes,void *owned_record,size_t record_bytes,
 size_t payload_offset,const struct m3_bank_mutex_ops *mutex);
#endif

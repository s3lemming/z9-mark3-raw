#ifndef M3_HSFC_SETTINGS_RECORD_H
#define M3_HSFC_SETTINGS_RECORD_H
#include <stddef.h>
#include <stdint.h>
/* One already-selected settings group. This does not infer native bank IDs,
 * allocate NOR space or supply a flash transaction/commit protocol. */
struct m3_hsfc_settings {uint32_t option,codec;uint64_t generation;};
enum m3_record_result {M3_RECORD_OK,M3_RECORD_ABSENT,M3_RECORD_INVALID,M3_RECORD_VERSION};
#define M3_SETTINGS_CARRIER_BYTES 80u
enum m3_record_result m3_settings_encode(const struct m3_hsfc_settings*,void*,size_t);
enum m3_record_result m3_settings_decode(const void*,size_t,struct m3_hsfc_settings*);
/* Only a separately proven ABSENT extension permits this first-use migration.
 * Invalid/unknown extensions must never be passed here as absent. */
enum m3_record_result m3_settings_migrate_legacy(uint32_t legacy_quality,struct m3_hsfc_settings*);
#endif

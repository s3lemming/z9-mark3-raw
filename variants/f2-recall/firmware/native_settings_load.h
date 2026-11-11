#ifndef MARK3_NATIVE_SETTINGS_LOAD_H
#define MARK3_NATIVE_SETTINGS_LOAD_H
#include "native_settings_abi.h"
#include "hsfc_settings_record.h"
enum m3_native_load_result {
 M3_LOAD_OK,M3_LOAD_ABSENT,M3_LOAD_INVALID,M3_LOAD_VERSION,M3_LOAD_UNSUPPORTED,
 M3_LOAD_NOT_OWNED,M3_LOAD_NATIVE_UNAVAILABLE,M3_LOAD_READ_FAILED,M3_LOAD_MALFORMED
};
#define M3_NATIVE_BANK_FIELDS_COMPLETE 7u
struct m3_native_load_api {
 void *user;
 /* The caller already owns the SAME complete F transaction, with native
  * records loaded and immutable. Native binding is not provided here. */
 uint32_t (*is_owned)(void*);
 /* Native selected-bank/accepted-end/counter authority, not private CRC or
  * generation arbitration. Return exact7 only after all3 outputs are valid;
  * an old boolean-positive/partial selection is rejected. */
 uint32_t (*select_native)(void*,const struct m3_native_storage_descriptor*,
  const struct m3_native_settings_schema*,uint64_t*,uint64_t*,uint32_t*);
 int (*read)(void*,uint64_t,void*,size_t);
};
struct m3_native_load_receipt {
 uint32_t status,native_records,native_footers,private_entries;
 uint32_t suspicious_runs,selected_bank,native_counter,covered_native_records;
 uint64_t bank,footer,scanned_bytes;
};
/* Caller owns aligned, disjoint output/receipt. Output changes only on OK.
 * Only ABSENT permits separate explicit legacy migration. Invalid/future/HE*
 * or ambiguous long opaque runs are never silently treated as absent.
 * Walk native boundaries; never search record payload bytes for magic. */
enum m3_native_load_result m3_native_settings_load_owned(
 const struct m3_native_load_api*,const struct m3_native_storage_descriptor*,
 const struct m3_native_settings_schema*,struct m3_hsfc_settings*,
 struct m3_native_load_receipt*);
#endif

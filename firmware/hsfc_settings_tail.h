#ifndef M3_HSFC_SETTINGS_TAIL_H
#define M3_HSFC_SETTINGS_TAIL_H
#include "hsfc_settings_record.h"
#define M3_SETTINGS_TAIL_BYTES (M3_SETTINGS_CARRIER_BYTES+8u)
/* Caller must already own the native flash transaction, reserve all88 bytes
 * inside erased space of the chosen journal, and supply the ORIGINAL native commit counter.
 * Callbacks return1 on success in this adapter's domain (native NOR returns0).
 * No native bank mapping, address allocation or firmware hook is implied. */
struct m3_tail_api {
 int (*program_word)(void*,uint64_t,uint32_t);
 int (*read)(void*,uint64_t,void*,size_t);
 void*context;
};
struct m3_tail_receipt {uint32_t program_calls,commit_word_issued,verified;};
enum m3_tail_result {M3_TAIL_COMMITTED,M3_TAIL_INVALID,M3_TAIL_PROGRAM_FAILED,M3_TAIL_VERIFY_FAILED};
enum m3_tail_result m3_settings_write_tail(const struct m3_tail_api*,uint64_t,size_t,
 const struct m3_hsfc_settings*,uint32_t,struct m3_tail_receipt*);
#endif

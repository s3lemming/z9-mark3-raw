#ifndef MARK3_NATIVE_SETTINGS_ABI_H
#define MARK3_NATIVE_SETTINGS_ABI_H
#include <stddef.h>
#include <stdint.h>
/* Z9 5.32 original storage ABI, recovered/tested at P397. Number0F below
 * is storageID, never property/UI/bufferID. This header installs nothing.
 * Padding remains native-owned and must NOT carry feature cookies. */
struct m3_native_settings_buffer {void *data;uint32_t bytes,padding;};
struct m3_native_settings_schema {
 uint32_t count,padding0;
 void *records;
 uint32_t auxiliary_count,padding1;
 void *auxiliary;
};
struct m3_native_storage_descriptor {
 uint64_t native0,native8,native16;
 uint32_t native24,padding28;
 uint64_t bank_A,bank_B;
 uint32_t bank_A_bytes,bank_B_bytes;
 /* Save client40D7173C leaves this unspecified; other native descriptor
  * consumers can use offset56. This is NOT a generic native-load ABI. */
 uint64_t native_unspecified56;
 void *flags;
 void *body;
 uint32_t body_bytes,padding84;
};
struct m3_native_settings_record {
 void *data;
 uint32_t bytes,padding12;
 uint64_t dirty;
};
struct m3_native_bank_area {
 uint32_t descriptor20,descriptor24;
 uint64_t bank_A,bank_B;
 uint32_t bytes,padding28;
 void *flags;
};
#define M3_NATIVE_SETTINGS_STORAGE_ID 0x0fu
#define M3_NATIVE_SETTINGS_BODY_BYTES 0x34040u
#define M3_NATIVE_SETTINGS_RECORDS 202u
#define M3_NATIVE_SETTINGS_AUXILIARY_RECORDS 43u
#define M3_NATIVE_SETTINGS_BANK_BYTES 0x40000u
static inline int m3_native_settings_domain(const struct m3_native_storage_descriptor*d,const struct m3_native_settings_schema*s){
 return d&&s&&d->bank_A==0x45e20000ull&&d->bank_B==0x46200000ull&&
 d->bank_A_bytes==M3_NATIVE_SETTINGS_BANK_BYTES&&d->bank_B_bytes==M3_NATIVE_SETTINGS_BANK_BYTES&&
 d->flags==(void*)(uintptr_t)0xa02926b2cull&&d->body==(void*)(uintptr_t)0xa03f1df14ull&&d->body_bytes==M3_NATIVE_SETTINGS_BODY_BYTES&&
 s->count==M3_NATIVE_SETTINGS_RECORDS&&s->records==(void*)(uintptr_t)0xa0295ffd8ull&&
 s->auxiliary_count==M3_NATIVE_SETTINGS_AUXILIARY_RECORDS&&s->auxiliary==(void*)(uintptr_t)0xa03f1df18ull;
}
_Static_assert(sizeof(void*)==8,"Original firmware pointer ABI");
_Static_assert(sizeof(struct m3_native_settings_buffer)==16,"Original buffer descriptor");
_Static_assert(sizeof(struct m3_native_settings_schema)==32,"Original schema");
_Static_assert(offsetof(struct m3_native_settings_schema,auxiliary)==24,"Original schema auxiliary");
_Static_assert(sizeof(struct m3_native_storage_descriptor)==88,"Original storage descriptor");
_Static_assert(sizeof(struct m3_native_settings_record)==24,"Original native record stride");
_Static_assert(sizeof(struct m3_native_bank_area)==40,"Original native bank-area descriptor");
_Static_assert(offsetof(struct m3_native_bank_area,flags)==32,"Original bank-area flags");
_Static_assert(offsetof(struct m3_native_storage_descriptor,flags)==64,"Original flags");
_Static_assert(offsetof(struct m3_native_storage_descriptor,body_bytes)==80,"Original body size");
/* Original4101A890 writes only u32 size and returns body pointer. Original
 *4101A960 fills schema but leaves padding1 untouched. Body size/capacity is
 * fixed; an80-byte serialized carrier must never enlarge that live buffer.
 * Original40D7173C(storageID,buffer*,schema*) returns bit0 of the save core.
 * Original40D74BF4 returns1 even after several no-write/failure paths.
 * A successful native return is NOT a persisted-record receipt or a lock. */
#endif

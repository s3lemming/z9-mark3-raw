#ifndef MARK3_NATIVE_SETTINGS_BANK_H
#define MARK3_NATIVE_SETTINGS_BANK_H
#include "native_settings_load.h"
struct m3_native_bank_api {
 void *user;
 uint32_t (*is_owned)(void*);
 int32_t (*choose)(uint32_t,const struct m3_native_bank_area*);
 uint64_t (*end)(const struct m3_native_settings_schema*,uint64_t,uint32_t);
 int (*read)(void*,uint64_t,void*,size_t);
};
struct m3_native_bank_provider {
 const struct m3_native_bank_api *api;
 const struct m3_native_settings_schema *native_global_schema;
};
/* Concrete5.32 ROM choose/end plus bounded mapped-NOR read, requiring the
 * caller's proved complete F owner callback. This does not initialize the
 * native global schema, create a transaction lock or perform a cold boot. */
int m3_native_bank_bind_rom532(struct m3_native_bank_api*,void*,uint32_t(*)(void*));
int m3_native_load_bind_bank_provider(struct m3_native_load_api*,
 struct m3_native_bank_provider*,const struct m3_native_bank_api*,
 const struct m3_native_settings_schema*);
uint32_t m3_native_bank_select(void*,const struct m3_native_storage_descriptor*,
 const struct m3_native_settings_schema*,uint64_t*,uint64_t*,uint32_t*);
#endif

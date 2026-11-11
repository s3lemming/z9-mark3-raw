#ifndef MARK3_NATIVE_SETTINGS_VIEW_H
#define MARK3_NATIVE_SETTINGS_VIEW_H
#include "native_settings_abi.h"
struct m3_native_settings_view {
 struct m3_native_settings_buffer buffer;
 struct m3_native_settings_schema schema;
 struct m3_native_storage_descriptor storage;
};
struct m3_native_view_api {
 void *user;
 uint32_t (*is_owned)(void*);
 void *(*buffer)(uint32_t*);
 void (*schema)(struct m3_native_settings_schema*);
 const void *(*storage)(uint32_t);
 const void *(*attributes)(uint32_t);
};
enum m3_native_view_result {M3_NATIVE_VIEW_OK,M3_NATIVE_VIEW_INVALID,
 M3_NATIVE_VIEW_NOT_OWNED,M3_NATIVE_VIEW_NOT_READY};
_Static_assert(sizeof(struct m3_native_settings_view)==136,"Owned native view");
/* Concrete original getter/registry addresses only. No initialization, lock,
 * save, flash access or native-ready result is implied by successful bind. */
int m3_native_view_bind_rom532(struct m3_native_view_api*,void*,uint32_t(*)(void*));
/* Caller already holds the complete native F transaction. All original
 * buffers/schema/registry are read under that ownership; output changes only
 * on OK. Native padding in the OWNED output is zero, never a feature cookie. */
enum m3_native_view_result m3_native_settings_view_owned(
 const struct m3_native_view_api*,struct m3_native_settings_view*);
#endif

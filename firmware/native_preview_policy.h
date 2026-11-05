#ifndef MARK3_NATIVE_PREVIEW_POLICY_H
#define MARK3_NATIVE_PREVIEW_POLICY_H
#include <stdint.h>
/* RAW-only may reuse the main JPEG storage, but must use the native NEF
 * preview rate-control profile. RAW+JPEG/JPEG retain their selected grade. */
uint32_t m3_preview_kind(uint32_t kind,uint32_t metadata_kind,
 uint32_t release_mode,uint32_t special,uint64_t committed_selection);
void m3_native_preview_parameters(uint64_t,uint32_t,uint32_t,uint64_t,uint64_t);
#endif

#ifndef F2_RECALL_FIELD_CODEC_H
#define F2_RECALL_FIELD_CODEC_H
#include <stdint.h>
/* Draft private encoding for the existing native32-bit recall selection mask.
 * 5.32 descriptor/image widths are proven4, but high-bit consumer compatibility
 * and native save/load remain prerequisites before firmware integration.
 * No new DSID, bank, NOR region or persistent padding is allocated here.
 */
#define F2_RECALL_NATIVE_BITS 0x00000fffu
enum f2_recall_release_override {F2_RELEASE_NATIVE,F2_RELEASE_C15,F2_RELEASE_C30};
struct f2_recall_fields {
    uint32_t native_mask;
    uint32_t vr_enabled;
    uint32_t vr_mode; /* native semantic Off0 / Normal1 / Sport2 */
    uint32_t release_override; /* private enum, never a native Quick/drive enum */
};
enum f2_recall_field_status {F2_FIELD_OK,F2_FIELD_LEGACY,F2_FIELD_INVALID,F2_FIELD_UNKNOWN};
enum f2_recall_field_status f2_recall_fields_decode(uint32_t,struct f2_recall_fields*);
/* Writes *out only after both source classification and target validation.
 * Native checkbox bits come exclusively from source, preventing a target
 * value edit from silently changing the original12 selected functions.
 */
enum f2_recall_field_status f2_recall_fields_encode(uint32_t,const struct f2_recall_fields*,uint32_t*);
#endif

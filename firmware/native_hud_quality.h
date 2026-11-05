#ifndef M3_NATIVE_HUD_QUALITY_H
#define M3_NATIVE_HUD_QUALITY_H
#include <stdint.h>
/* GUI record values, distinct from menu row IDs, codec IDs and M formats.
 * Only a committed HE selection and actual C15/C30 photo context are owned. */
struct m3_hud_quality {
 uint32_t native_type, jpeg_grade, star, published_code;
};
int m3_hud_quality_plan(uint32_t photo,uint32_t native_mode,
 uint32_t special_processing,uint64_t committed_choice,
 struct m3_hud_quality *out);
#endif

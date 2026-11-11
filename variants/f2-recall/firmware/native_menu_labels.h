#ifndef MARK3_NATIVE_MENU_LABELS_H
#define MARK3_NATIVE_MENU_LABELS_H
#include <stdint.h>
/* Separate from ordinary label/UI/property/policy number spaces.
 * mark1 owns C84/C85; mark2 picker owns C86..C9A. No shared source is changed.
 * These two IDs require the new-label-only resolver hook before use by rows.
 * Component tests do not qualify a rendered native page. */
#define M3_LABEL_C15_C30 0xc9cu
#define M3_LABEL_C60_C120 0xc9du
void m3_native_group_label(uint32_t language,uint32_t label,
                           const uint16_t **out,uint32_t *bytes);
#endif

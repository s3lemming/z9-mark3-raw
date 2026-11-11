#ifndef F2_RECALL_VALUE_OVERLAY_H
#define F2_RECALL_VALUE_OVERLAY_H
#include "recall_field_codec.h"
/* Draft read overlay, not linked into a firmware. The original persistent
 * current words stay unchanged; release targets are consumed by the original
 * recall eligibility checks. Native target3/6 reach original release-helper
 * profiles2/5, but the complete C30/C15 application remains to be qualified.
 * Callers must provide a coherent read of active/column and both bank masks.
 */
struct f2_recall_overlay {
    uint32_t mask[2];
    uint32_t active; /* original condition4000012C, exactly1 */
    uint32_t column; /* original condition40000143, normal0 / hold1 */
    uint32_t vr_allowed; /* same-activation native_options accepts target */
};
enum f2_recall_overlay_result { F2_OVERLAY_UNCHANGED, F2_OVERLAY_VALUE };
/* Native original_get succeeds before this function is used. Returns a value
 * only for the identified DSID; no setting writes, allocation, or recursion.
 * Failure or unrecognized extension leaves *out unchanged.
 */
enum f2_recall_overlay_result f2_recall_overlay_value(
    const struct f2_recall_overlay *, uint32_t id, uint32_t *out);
#endif

#include "recall_value_overlay.h"
static int fields(uint32_t word,struct f2_recall_fields*out){
    return f2_recall_fields_decode(word,out)==F2_FIELD_OK;
}
enum f2_recall_overlay_result f2_recall_overlay_value(
    const struct f2_recall_overlay*s,uint32_t id,uint32_t*out){
    if(!s||!out)return F2_OVERLAY_UNCHANGED;
    struct f2_recall_fields f;
    /* Unlike current VR, these IDs describe stored recall TARGETS. Their
     * value can be read while idle without applying a release mode. Both
     * original helpers still honor checkbox11, active and inhibit gates.
     * Match complete proven DSIDs, not all unrelated properties sharing a
     * low byte or category number.
     */
    if(id==0x06001792u||id==0x0600793du){
        unsigned c=id==0x0600793du;
        if(!fields(s->mask[c],&f)||f.release_override==F2_RELEASE_NATIVE)
            return F2_OVERLAY_UNCHANGED;
        /* Native recall-target enum is S0/CL1/CH2. It is not the
         * Quick enum C30=3/C15=6. Both HSFC choices require continuous
         * class CH2; recall_quick.S supplies their private submode after
         * the original recall gates. */
        *out=2u;
        return F2_OVERLAY_VALUE;
    }
    if(id!=0x07000675u&&id!=0x03000676u)return F2_OVERLAY_UNCHANGED;
    if(s->active!=1||s->column>1||s->vr_allowed!=1||
       !fields(s->mask[s->column],&f)||!f.vr_enabled)
        return F2_OVERLAY_UNCHANGED;
    *out=id==0x07000675u?f.vr_mode:(f.vr_mode!=0);
    return F2_OVERLAY_VALUE;
}

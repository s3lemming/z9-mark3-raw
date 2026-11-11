#include "recall_field_codec.h"
#define TAG 0xb0540000u
#define TAG_MASK 0xfffc0000u
#define RESERVED 0x00020000u
static int valid(const struct f2_recall_fields*s){
    return s&&s->vr_enabled<=1&&s->vr_mode<=2&&s->release_override<=F2_RELEASE_C30;
}
enum f2_recall_field_status f2_recall_fields_decode(uint32_t word,struct f2_recall_fields*out){
    if(!out)return F2_FIELD_INVALID;
    struct f2_recall_fields result={word&F2_RECALL_NATIVE_BITS,0,0,F2_RELEASE_NATIVE};
    if((word&TAG_MASK)!=TAG){
        uint32_t upper=word&~F2_RECALL_NATIVE_BITS;
        /* Both the initialized normal mask and untouched/toggled all-ones
         * hold defaults must mean no private options, never enable VR.
         */
        if(upper!=0&&upper!=~F2_RECALL_NATIVE_BITS)return F2_FIELD_UNKNOWN;
        *out=result;return F2_FIELD_LEGACY;
    }
    if(word&RESERVED)return F2_FIELD_INVALID;
    result.vr_enabled=(word>>12)&1u;
    result.vr_mode=(word>>13)&3u;
    result.release_override=(word>>15)&3u;
    if(!valid(&result))return F2_FIELD_INVALID;
    *out=result;return F2_FIELD_OK;
}
enum f2_recall_field_status f2_recall_fields_encode(uint32_t source,const struct f2_recall_fields*target,uint32_t*out){
    struct f2_recall_fields prior;
    if(!out||!valid(target))return F2_FIELD_INVALID;
    enum f2_recall_field_status status=f2_recall_fields_decode(source,&prior);
    if(status!=F2_FIELD_OK&&status!=F2_FIELD_LEGACY)return status;
    *out=TAG|(source&F2_RECALL_NATIVE_BITS)|(target->vr_enabled<<12)|
         (target->vr_mode<<13)|(target->release_override<<15);
    return F2_FIELD_OK;
}

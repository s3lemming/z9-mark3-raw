#include "format_policy.h"
#include <stddef.h>
/* Keep freestanding builds independent of compiler-generated memcpy even
 * with the platform register X18 reserved. These are private value copies;
 * the caller still owns publication and synchronization. */
static void policy_copy(volatile unsigned char *out,const void *in,size_t n){
    const unsigned char *p=in;
    for(size_t i=0;i<n;i++)out[i]=p[i];
}
/* P330: RAW+JPEG fine, RAW+JPEG normal, RAW, JPEG fine, JPEG normal. */
const uint32_t m3_hsfc_option_ids[M3_HSFC_OPTION_COUNT] = {2,4,0,8,10};
/* Labels are review names. Native integration must reuse Nikon localized resources.
 * HE defaults here are not a write to RAW recording: adapter uses the actual menu codec. */
const struct m3_option m3_format_options[13] = {
    {"RAW", {M3_RAW,0,0,M3_HE}},
    {"RAW + JPEG fine*", {M3_RAW_JPEG,0,1,M3_HE}},
    {"RAW + JPEG fine", {M3_RAW_JPEG,0,0,M3_HE}},
    {"RAW + JPEG normal*", {M3_RAW_JPEG,1,1,M3_HE}},
    {"RAW + JPEG normal", {M3_RAW_JPEG,1,0,M3_HE}},
    {"RAW + JPEG basic*", {M3_RAW_JPEG,2,1,M3_HE}},
    {"RAW + JPEG basic", {M3_RAW_JPEG,2,0,M3_HE}},
    {"JPEG fine*", {M3_JPEG,0,1,M3_HE}},
    {"JPEG fine", {M3_JPEG,0,0,M3_HE}},
    {"JPEG normal*", {M3_JPEG,1,1,M3_HE}},
    {"JPEG normal", {M3_JPEG,1,0,M3_HE}},
    {"JPEG basic*", {M3_JPEG,2,1,M3_HE}},
    {"JPEG basic", {M3_JPEG,2,0,M3_HE}}
};
enum m3_result m3_can_select(const struct m3_context *c,const struct m3_selection *s) {
    if(!c || !s || c->effective_mode>M3_MOVIE) return M3_INVALID;
    if(c->effective_mode!=M3_C15 && c->effective_mode!=M3_C30) return M3_STOCK;
    if(!c->photo || (c->fx_large!=M3_GEOMETRY_FX_LARGE &&
       c->fx_large!=M3_GEOMETRY_DX_LARGE) || c->special_processing) return M3_UNSUPPORTED;
    if((c->contracts&M3_REQUIRED_CONTRACTS)!=M3_REQUIRED_CONTRACTS) return M3_NOT_READY;
    if(c->fx_large==M3_GEOMETRY_DX_LARGE && !(c->contracts&M3_DX_CONTRACT)) return M3_NOT_READY;
    if(c->inflight) return M3_BUSY;
    if(s->format!=M3_RAW && s->format!=M3_RAW_JPEG && s->format!=M3_JPEG) return M3_UNSUPPORTED;
    if(s->jpeg_grade>2 || s->jpeg_star>1) return M3_INVALID;
    if(s->format!=M3_RAW && (s->jpeg_grade>1 || s->jpeg_star)) return M3_UNSUPPORTED;
    if(s->format!=M3_JPEG && s->codec!=M3_HE && s->codec!=M3_HE_STAR) return M3_UNSUPPORTED;
    if(c->fx_large==M3_GEOMETRY_DX_LARGE && s->format!=M3_JPEG &&
       s->codec==M3_HE_STAR && !(c->contracts&M3_HE_STAR_DX_CONTRACT)) return M3_NOT_READY;
    return M3_ALLOWED;
}
enum m3_result m3_snapshot(const struct m3_context *c,const struct m3_selection *s,
                         uint32_t primary,uint32_t role,uint32_t seq,
                         struct m3_selection *copy,uint32_t route[3]) {
    /* Existing queued frames forbid editing settings, not admitting the next
     * burst frame with the already committed selection. */
    struct m3_context admission;
    enum m3_result r;
    if(!c) return M3_INVALID;
    policy_copy((volatile unsigned char*)&admission,c,sizeof admission); admission.inflight=0;
    r=m3_can_select(&admission,s);
    if(r!=M3_ALLOWED) return r;
    if(!copy || !route || primary>1 || role>3) return M3_INVALID;
    policy_copy((volatile unsigned char*)copy,s,sizeof *copy); route[0]=primary; route[1]=role; route[2]=seq;
    return M3_ALLOWED;
}

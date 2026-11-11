/* PACK5 integration research. Original BSS descriptor reserves and clears
 * one added page. No external loader, no board/device acceptance claim. */
typedef unsigned int u32;
typedef unsigned long long u64;
extern u32 native_get(u32 id, u32 *value);       /* 41019238, status + output */
extern u32 native_set(u32 id, u32 value);        /* 40D57380, upper acceptance */
struct options { u32 id, flags, count, pad; u32 *values; };
extern u32 native_options(u32 id, struct options *out, u32 byte_capacity);
struct action_state {
    u32 magic, held, remembered, valid;
    u32 calls, edges, transactions, accepted;
    u32 current, target, readback, result;
    u32 error, option_flags, option_count, last_mask;
    u32 disabled, restored, rejected, releases;
    u32 reserved[12];
};
#define VR 0x07000675u
#define MAGIC 0x4d315631u
#include "mark2_audio_runtime.h"
#include "recall_native.h"
#define STATE ((volatile struct action_state *)((unsigned char *)m23_audio_runtime_base()+8192u))
_Static_assert(sizeof(struct action_state)==128,"mark1 state contract");
/* Error: 1 read, 2 initial OFF/no memory, 3 options/disabled,
 * 4 target absent, 5 native rejection, 6 readback mismatch, 7 wrong control.
 * 8 recall owns a qualified temporary VR overlay.
 * Memory validity means accepted settings, not a VR hardware acknowledgement.
 */
__attribute__((noinline)) void mark1_action(u32 active, u32 mask) {
    if(!m23_audio_runtime_ready()) return;
    volatile struct action_state *s=STATE;
    /* Native startup has cleared every field. Publish identity once; fail
     * closed on unexpected ownership/corruption instead of overwriting it. */
    if (s->magic==0) {
        volatile u32 *words=(volatile u32 *)s;
        for (u32 i=1;i<32;i++) if (words[i]!=0) return;
        s->magic=MAGIC;
    }
    if (s->magic!=MAGIC) return;
    s->calls++; s->last_mask=mask;
    if (mask!=0x4000) { s->error=7; return; }
    if (!active) { if(s->held) s->releases++; s->held=0; return; }
    if (s->held) return;
    s->held=1; s->edges++; s->error=0;
    /* An active recall owns its temporary VR. Do not silently change the
     * underlying saved VR while readback still returns the recalled target. */
    if(f2r_vr_override_active()){s->disabled++;s->error=8;return;}
    u32 mode=0, after=0;
    if (!native_get(VR,&mode) || mode>2) { s->error=1; return; }
    s->current=mode;
    u32 target=0;
    if(mode==0) {
        if(!s->valid || (s->remembered!=1 && s->remembered!=2)) {
            s->error=2; return;
        }
        target=s->remembered;
    }
    u32 values[4]={0,0,0,0};
    struct options o={0,0,0,0,values};
    u32 ok=native_options(VR,&o,sizeof(values));
    s->option_flags=o.flags; s->option_count=o.count;
    /* Flags==1 corresponds to the native disabled selector in the observed
     * menu. Other flag meanings and bank invalidation remain ACTION2 scope. */
    if(!ok || o.flags==1 || o.count==0 || o.count>4) {s->error=3;return;}
    ok=0;
    for(u32 i=0;i<o.count;i++) if(values[i]==target) ok=1;
    if(!ok) {s->error=4;return;}
    s->target=target; s->transactions++;
    u32 accepted=native_set(VR,target);
    s->result=accepted;
    if(!(accepted&1)) {s->rejected++;s->error=5;return;}
    if(!native_get(VR,&after) || after!=target) {
        s->valid=0;s->readback=after;s->error=6;return;
    }
    s->readback=after;s->accepted++;
    if(mode) {s->remembered=mode;s->valid=1;s->disabled++;}
    else {s->valid=0;s->restored++;}
}

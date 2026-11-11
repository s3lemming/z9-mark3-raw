/* Recall TARGETS use CH=2 in both columns; Quick6/3 belongs to the downstream native helper, not this read overlay. See P684 consumer contract. */
#include "recall_value_overlay.h"
#include <assert.h>
#include <stdio.h>
static uint32_t encode(unsigned vr,unsigned mode,unsigned release){
    struct f2_recall_fields f={0,vr,mode,release};uint32_t v=0;
    assert(f2_recall_fields_encode(0xbff,&f,&v)==F2_FIELD_OK);return v;
}
int main(void){
    unsigned cases=0;
    for(unsigned a=0;a<3;a++)for(unsigned c=0;c<3;c++)for(unsigned allowed=0;allowed<2;allowed++)
    for(unsigned enabled=0;enabled<2;enabled++)for(unsigned mode=0;mode<3;mode++){
        struct f2_recall_overlay s={{encode(enabled,mode,F2_RELEASE_C15),encode(enabled,2-mode,F2_RELEASE_C30)},a,c,allowed};
        uint32_t v=0xa5a5a5a5;
        assert(f2_recall_overlay_value(&s,0x6001792,&v)==F2_OVERLAY_VALUE&&v==2);
        assert(f2_recall_overlay_value(&s,0x600793d,&v)==F2_OVERLAY_VALUE&&v==2);
        v=0xa5a5a5a5;
        int ok=a==1&&c<2&&enabled&&allowed;
        assert(f2_recall_overlay_value(&s,0x7000675,&v)==(ok?F2_OVERLAY_VALUE:F2_OVERLAY_UNCHANGED));
        assert(v==(ok?(c?2-mode:mode):0xa5a5a5a5));
        v=0xa5a5a5a5;
        assert(f2_recall_overlay_value(&s,0x3000676,&v)==(ok?F2_OVERLAY_VALUE:F2_OVERLAY_UNCHANGED));
        assert(v==(ok?((c?2-mode:mode)!=0):0xa5a5a5a5));
        /* Original target remains available with a disabled release checkbox;
         * the original consumers decide whether to apply it. Pure read never
         * mutates the two stored masks or changes an unrelated property.
         */
        uint32_t before0=s.mask[0],before1=s.mask[1];
        for(unsigned i=0;i<0x8000;i++){
            v=0xa5a5a5a5;uint32_t id=0x04000000u+i;
            assert(f2_recall_overlay_value(&s,id,&v)==F2_OVERLAY_UNCHANGED&&v==0xa5a5a5a5);
        }
        assert(s.mask[0]==before0&&s.mask[1]==before1);cases++;
    }
    const uint32_t bad[]={0,0xbff,0xffffffff,0xb0560000,0xb0546000,0xb0558000,0xabcdef01};
    for(unsigned i=0;i<sizeof bad/sizeof *bad;i++){
        struct f2_recall_overlay s={{bad[i],bad[i]},1,0,1};
        const uint32_t ids[]={0x6001792,0x600793d,0x7000675,0x3000676};
        for(unsigned j=0;j<4;j++){
            uint32_t v=0xa5a5a5a5;assert(f2_recall_overlay_value(&s,ids[j],&v)==F2_OVERLAY_UNCHANGED&&v==0xa5a5a5a5);
        }cases++;
    }
    uint32_t v=7;assert(f2_recall_overlay_value(0,0x7000675,&v)==F2_OVERLAY_UNCHANGED&&v==7);
    struct f2_recall_overlay s={{0,0},0,0,0};assert(f2_recall_overlay_value(&s,0x7000675,0)==F2_OVERLAY_UNCHANGED);
    printf("%u overlay state and malformed/legacy cases passed; no firmware binding\n",cases);
    return 0;
}

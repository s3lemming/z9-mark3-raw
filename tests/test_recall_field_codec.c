#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "recall_field_codec.h"
int main(void){
    uint32_t count=0;
    for(uint32_t mask=0;mask<=F2_RECALL_NATIVE_BITS;mask++){
        for(uint32_t legacy=0;legacy<2;legacy++){
            uint32_t source=mask|(legacy?0xfffff000u:0u);
            struct f2_recall_fields prior;
            assert(f2_recall_fields_decode(source,&prior)==F2_FIELD_LEGACY);
            assert(prior.native_mask==mask&&!prior.vr_enabled&&!prior.vr_mode&&!prior.release_override);
            for(uint32_t enabled=0;enabled<2;enabled++)for(uint32_t vr=0;vr<3;vr++)for(uint32_t release=0;release<3;release++){
                struct f2_recall_fields want={0,enabled,vr,release},got;
                struct {uint32_t left,word,right;} out={0x5180aa55,0xdeadbeef,0x94f5c00a};
                assert(f2_recall_fields_encode(source,&want,&out.word)==F2_FIELD_OK);
                assert(out.left==0x5180aa55&&out.right==0x94f5c00a);
                assert((out.word&F2_RECALL_NATIVE_BITS)==mask);
                assert(f2_recall_fields_decode(out.word,&got)==F2_FIELD_OK);
                assert(got.native_mask==mask&&got.vr_enabled==enabled&&got.vr_mode==vr&&got.release_override==release);
                /* Native checkbox edits on an encoded record retain private
                 * targets; no separate save record or pointer is involved. */
                uint32_t edited=out.word^0x801u;
                assert(f2_recall_fields_decode(edited,&got)==F2_FIELD_OK);
                assert(got.native_mask==(mask^0x801u)&&got.vr_enabled==enabled&&got.vr_mode==vr&&got.release_override==release);
                count++;
            }
        }
    }
    struct f2_recall_fields s={1,0,0,0},before=s;uint32_t out=0xaabbccdd;
    const uint32_t rejected[]={0x40000bff,0x90540bff,0xb0560bff,0xb0546bff,0xb0558bff};
    for(unsigned i=0;i<sizeof rejected/sizeof rejected[0];i++){
        enum f2_recall_field_status r=f2_recall_fields_decode(rejected[i],&s);
        assert(r==F2_FIELD_INVALID||r==F2_FIELD_UNKNOWN);assert(!memcmp(&s,&before,sizeof s));
        r=f2_recall_fields_encode(rejected[i],&s,&out);assert(r==F2_FIELD_INVALID||r==F2_FIELD_UNKNOWN);assert(out==0xaabbccdd);
    }
    assert(f2_recall_fields_decode(0,NULL)==F2_FIELD_INVALID);
    assert(f2_recall_fields_encode(0,NULL,&out)==F2_FIELD_INVALID);
    assert(f2_recall_fields_encode(0,&s,NULL)==F2_FIELD_INVALID);
    for(unsigned k=1;k<=3;k++){
        struct f2_recall_fields invalid={0,0,0,0};
        if(k==1)invalid.vr_enabled=2;
        if(k==2)invalid.vr_mode=3;
        if(k==3)invalid.release_override=3;
        assert(f2_recall_fields_encode(0,&invalid,&out)==F2_FIELD_INVALID&&out==0xaabbccdd);
    }
    printf("{\"passed\":true,\"legacy_and_private_roundtrips\":%u,\"integrated\":false}\n",count);
    return 0;
}

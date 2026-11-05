#include "format_policy.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    struct m3_context c={M3_C15,6,1,1,0,0,M3_REQUIRED_CONTRACTS};
    struct m3_selection s={M3_RAW,0,0,M3_HE}, copy={99,99,99,99};
    uint32_t route[3]={99,99,99}, cases=0;
    for(unsigned mode=M3_C15;mode<=M3_C30;mode++) {
        c.effective_mode=mode;
        for(unsigned i=0;i<13;i++)for(unsigned codec=3;codec<=4;codec++) {
            s=m3_format_options[i].value;s.codec=codec;
            unsigned allowed=i==0 || i==2 || i==4 || i==8 || i==10;
            assert(m3_can_select(&c,&s)==(allowed?M3_ALLOWED:M3_UNSUPPORTED));cases++;
        }
    }
    /* A retained Quick setting cannot hijack ordinary S/CL/CH, or C60/C120. */
    for(unsigned mode=0;mode<=M3_MOVIE;mode++) if(mode!=M3_C15 && mode!=M3_C30) {
        c.effective_mode=mode;assert(m3_can_select(&c,&s)==M3_STOCK);cases++;
    }
    c.effective_mode=M3_C15;s=(struct m3_selection){M3_RAW,0,0,M3_HE};
    for(unsigned mask=0;mask<M3_REQUIRED_CONTRACTS;mask++) {
        c.contracts=mask;assert(m3_can_select(&c,&s)==M3_NOT_READY);cases++;
    }
    c.contracts=M3_REQUIRED_CONTRACTS;
    s.codec=M3_LOSSLESS;assert(m3_can_select(&c,&s)==M3_UNSUPPORTED);cases++;
    s.format=M3_JPEG;assert(m3_can_select(&c,&s)==M3_ALLOWED);cases++;
    s.format=M3_RAW;s.codec=M3_HE_STAR;
    c.fx_large=0;assert(m3_can_select(&c,&s)==M3_UNSUPPORTED);cases++;c.fx_large=1;
    c.special_processing=1;assert(m3_can_select(&c,&s)==M3_UNSUPPORTED);cases++;c.special_processing=0;
    c.photo=0;assert(m3_can_select(&c,&s)==M3_UNSUPPORTED);cases++;c.photo=1;
    c.inflight=1;assert(m3_can_select(&c,&s)==M3_BUSY);cases++;
    assert(m3_snapshot(&c,&s,2,1,7,&copy,route)==M3_INVALID);
    assert(copy.format==99 && route[0]==99);cases++;
    assert(m3_snapshot(&c,&s,1,2,7,&copy,route)==M3_ALLOWED);
    s.format=M3_JPEG;s.codec=M3_HE;
    assert(copy.format==M3_RAW && copy.codec==M3_HE_STAR && route[0]==1 && route[1]==2 && route[2]==7);cases++;
    c.inflight=0;s.jpeg_star=2;assert(m3_can_select(&c,&s)==M3_INVALID);cases++;
    s.jpeg_star=0;s.jpeg_grade=3;assert(m3_can_select(&c,&s)==M3_INVALID);cases++;
    assert(m3_can_select(0,&s)==M3_INVALID);cases++;
    printf("mark3 policy %u cases passed; native menu integration not exercised\n",cases);
}

#include "native_hud_remaining.h"
int m3_hud_remaining_plan(uint32_t photo,uint32_t mode,uint32_t special,
 uint32_t latched_mode,uint64_t choice,uint32_t original,uint32_t available,
 uint32_t *out){
 if(!out||photo!=1||(mode!=3&&mode!=6)||latched_mode!=mode||special||
    ((choice>>32)!=3&&(choice>>32)!=4)||original>0x7fffffffu||available>0x7fffffffu)return 0;
 uint32_t option=(uint32_t)choice;
 if(option!=0&&option!=2&&option!=4)return 0;
 *out=original<available?original:available;return 1;
}

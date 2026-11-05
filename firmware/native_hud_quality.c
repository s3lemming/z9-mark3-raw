#include "native_hud_quality.h"
/* P467: original406A5AFC and jump table4347A9E8 map native type0 to
 * RAW+JPEG/grade, type1 to RAW/code3, and type3 to JPEG/grade+5.
 * 1C1C is JPEG star, not the RAW compression enum. HE4/HE-star3 are checked separately.
 * This component is not yet linked: GUI publication and refresh must be
 * qualified together before a new package can claim a fixed liveview HUD. */
int m3_hud_quality_plan(uint32_t photo,uint32_t mode,uint32_t special,
 uint64_t choice,struct m3_hud_quality *out){
 if(!out||photo!=1||(mode!=3&&mode!=6)||special||((choice>>32)!=3&&(choice>>32)!=4))return 0;
 uint32_t type,grade=0,code;
 switch((uint32_t)choice){
 case 0:type=1;code=3;break;
 case 2:type=0;code=0;break;
 case 4:type=0;grade=1;code=1;break;
 case 8:type=3;code=5;break;
 case 10:type=3;grade=1;code=6;break;
 default:return 0;
 }
 out->native_type=type;out->jpeg_grade=grade;out->star=0;out->published_code=code;
 return 1;
}

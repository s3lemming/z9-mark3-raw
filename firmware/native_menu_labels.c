#include "native_menu_labels.h"
static const uint16_t c15_c30[]={'C','1','5','/','C','3','0',0};
static const uint16_t c60_c120[]={'C','6','0','/','C','1','2','0',0};
static const uint16_t raw_star[]={'R','A','W',':',' ','H','E',0x2605,0};
static const uint16_t raw_he[]={'R','A','W',':',' ','H','E',0};
void m3_native_group_label(uint32_t language,uint32_t label,
                           const uint16_t **out,uint32_t *bytes){
 /* Resolver wrapper calls this only for these exact new IDs. Group names
  * are identical in all29 original language directories, so no fallback
  * translation or global language setting is substituted. Length excludes
  * the UTF16 terminator, matching the proven native display ABI. */
 *out=0;*bytes=0;
 if(language>=29)return;
 if(label==M3_LABEL_C15_C30){*out=c15_c30;*bytes=14;}
 else if(label==M3_LABEL_C60_C120){*out=c60_c120;*bytes=16;}
 else if(label==0xc9e){*out=raw_star;*bytes=16;}
 else if(label==0xc9f){*out=raw_he;*bytes=14;}
}

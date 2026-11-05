/* Production bridge: no diagnostic state at the menu RAM address. Menu
 * enum3/4, native codec4/2 and MakerNote14/13 are distinct namespaces. */
#include "native_codec_geometry.h"
#include "native_live_settings.h"
#include "native_runtime_layout.h"
#ifndef FN
#define FN(a,t) ((t)(uintptr_t)(a))
#endif
typedef uint32_t u32;typedef uint64_t u64;
static u32 rd32(u64 a){return *(const u32*)(uintptr_t)a;}
static u64 rd64(u64 a){return *(const u64*)(uintptr_t)a;}
#ifndef M3_CODEC_LIVE_TEST
static u32 selected_raw_codec(void){
 u64 selection=__atomic_load_n(&((struct m3_live_settings*)(uintptr_t)
  M3_LIVE_SETTINGS_ADDRESS)->condition_selection,__ATOMIC_ACQUIRE);
 u32 option=(u32)selection;
 u32 codec=(u32)(selection>>32);
 return (codec==3||codec==4)&&(option==0||option==2||option==4)?codec:0;
}
#else
extern u32 m3_codec_test_selected_raw_codec(void);
#define selected_raw_codec m3_codec_test_selected_raw_codec
#endif
static int star_selected(void){return selected_raw_codec()==3;}
static int dx_release_selected(void){
 /* Geometry mode5 is also reachable by other HSFC releases. It is not a
  * C15/C30 identity. Recheck original photo/Quick and applied Cxx latch;
  * keep native sizes for C60/C120 and a stale Quick choice in ordinary S. */
 u32 photo=FN(0x406a3f54,u32(*)(u32))(0x1d00);
 u32 mode=FN(0x406a3f54,u32(*)(u32))(0x1c26);
 return photo==1&&m3_codec_mode(mode)&&mode==rd32(0xa03d9d218ull)&&
  FN(0x406a3f54,u32(*)(u32))(0x603)==1;
}
static int info_scope(u64 info){
 if(!info||rd32(info)!=3)return 0;
 /* Original410695EC/F0 consumes ImageInfo48 as release and398 as area.
  * Word8 is opaque here, not area. Actual DX info is398=1/48=6/8=4;
  * original410537EC allocation still uses raw release6, not profile5. */
 u32 area=rd32(info+0x398),mode=rd32(info+0x48);
 if(!area)return m3_codec_mode(mode);
 return area==1&&(m3_codec_mode(mode)||mode==5)&&dx_release_selected();
}
void m3_codec_tenc_config(u64 ctx){
 FN(0x40e91c38,void(*)(u64))(ctx);
 volatile u32*budget=(volatile u32*)(uintptr_t)0xa03d4ae10ull;
 if(ctx&&star_selected()&&rd32(ctx+0x10)==11&&
    info_scope(rd64(ctx+0x9e0))&&*budget==2)*budget=4;
}
u64 m3_codec_E_alloc(u32 frame,u32 pool,u32 size,u64 info){
 u32 codec=selected_raw_codec();
 if(codec&&pool==0x39&&info_scope(info)){
  u32 area=rd32(info+0x398),mode=rd32(info+0x48);
  if(!area&&codec==3&&size==M3_HE_REQUEST)size=M3_HE_STAR_REQUEST;
  else if(area==1&&((m3_codec_mode(mode)&&size==M3_HE_REQUEST)||
          (mode==5&&size==M3_HE_DX_REQUEST)))size=codec==3?M3_HE_STAR_DX_REQUEST:M3_HE_DX_REQUEST;
 }
 return FN(0x4104cd70,u64(*)(u32,u32,u32))(frame,pool,size);
}
u32 m3_codec_E_size(u32 kind,u32 mode){
 u32 original=FN(0x41053880,u32(*)(u32,u32))(kind,mode);
 u32 codec=selected_raw_codec();int star=codec==3;
 if(codec&&kind==1&&m3_codec_mode(mode)&&original==M3_HE_REQUEST&&dx_release_selected())
  return star?M3_HE_STAR_DX_REQUEST:M3_HE_DX_REQUEST;
 if(star&&kind==1&&mode==5&&original==M3_HE_DX_REQUEST&&!dx_release_selected())return original;
 return m3_codec_size(kind,mode,original,star);
}
u32 m3_codec_decoder_selector(u64 r){
 /* Decode the packet actually allocated for this frame. A later global
  * selection cannot turn a small retained HE packet into HE-star. */
 u64 e=r?rd64(r+0x748):0,a=e?rd64(e+8):0;
 if(!a)return 2;
 return m3_codec_decoder(rd32(e+0x30),rd32(e+0x34),rd32(e+0x38),
  rd64(a),rd64(e+0x40),rd32(a+8),1);
}

#ifndef MARK3_NATIVE_BURST_REFERENCE_H
#define MARK3_NATIVE_BURST_REFERENCE_H
#include <stddef.h>
#include <stdint.h>
/* Original5.32 MakerNote0056 leaf406E7340 reads same-frame M118/11C/120.
 * If11C is nonzero and118's slot differs from the current file's slot, the
 * leaf uses11C; otherwise it uses118. Slot helper4086FBE0 extracts bits29..30.
 * This bounded conversion accepts only known two-slot JPEG/NEF identifiers.
 * It changes the selected reference in an OWNED PRIVATE M, never the source,
 * sequence, folder/number, alternate slot or global settings. The caller must
 * prove that the corresponding NEF series actually exists. This does not
 * update DCF/NC_FLLST or qualify camera playback or series recognition.
 * RAW+JPEG keeps its native existing JPEG reference; use only for RAW-only.
 */
enum m3_burst_reference_result {M3_BURST_REFERENCE_OK,M3_BURST_REFERENCE_INVALID};
static inline uint32_t m3_burst_read32(const uint8_t *p){
 return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}
static inline enum m3_burst_reference_result m3_native_raw_burst_reference(
 void *private_M,size_t bytes,uint32_t JPEG_id,uint32_t NEF_id){
 uint8_t *m=private_M;
 if(!m||bytes<0x124||!JPEG_id||(JPEG_id&31u)!=0||(NEF_id&31u)!=2||
    (JPEG_id&~31u)!=(NEF_id&~31u)||(JPEG_id>>29)>1u)
  return M3_BURST_REFERENCE_INVALID;
 if(!m3_burst_read32(m+0x120))return M3_BURST_REFERENCE_OK;
 uint32_t first=m3_burst_read32(m+0x118),alt=m3_burst_read32(m+0x11c);
 size_t offset=alt&&((first>>29)&3u)!=((NEF_id>>29)&3u)?0x11c:0x118;
 uint32_t selected=m3_burst_read32(m+offset),kind=selected&31u;
 if(!selected||(selected>>29)!=(NEF_id>>29)||(kind!=0u&&kind!=2u))
  return M3_BURST_REFERENCE_INVALID;
 uint32_t next=(selected&~31u)|2u;
 for(unsigned i=0;i<4;i++)m[offset+i]=(uint8_t)(next>>(8*i));
 return M3_BURST_REFERENCE_OK;
}
#endif

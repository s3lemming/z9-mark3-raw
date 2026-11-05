/* Original Z9 5.32 packet geometries. Codec classes are native values,
 * never menu-policy numbers. Actual camera qualification is per run. */
#ifndef M3_NATIVE_CODEC_GEOMETRY_H
#define M3_NATIVE_CODEC_GEOMETRY_H
#include <stdint.h>
#define M3_HE_BYTES 17139200u
#define M3_HE_STAR_BYTES 28565504u
#define M3_HE_REQUEST 17310720u
#define M3_HE_STAR_REQUEST (M3_HE_STAR_BYTES + M3_HE_REQUEST - M3_HE_BYTES)
#define M3_HE_DX_BYTES 7316992u
#define M3_HE_STAR_DX_BYTES 12194816u
/* Original initialized RAW allocation table A02A3DC64, area1, codecs4/3.
 * These allocation sizes include room beyond the actual compressed strip. */
#define M3_HE_DX_REQUEST 7389184u
#define M3_HE_STAR_DX_REQUEST 12238336u
/* Actual pool57 descriptor capacity: requests round up to0x420A00 units.
 * P474 v25 original shared-submit descriptor returned0xC61E00, not the
 * requested0xBABE00. Bound the real three-unit slot, never an arbitrary FX
 * allocation, and still require the complete strip after its data offset. */
#define M3_HE_STAR_DX_CAPACITY (3u*0x420a00u)
static inline int m3_codec_mode(uint32_t mode) { return mode==3 || mode==6; }
static inline int m3_codec_scope(uint32_t kind,uint32_t cls,uint32_t area,uint32_t mode) {
 return kind==11 && cls==3 && ((area==0 && m3_codec_mode(mode)) ||
  (area==1 && mode==5));
}
static inline uint32_t m3_codec_size(uint32_t kind,uint32_t mode,uint32_t original,int star) {
 if(!star||kind!=1)return original;
 if(m3_codec_mode(mode)&&original==M3_HE_REQUEST)return M3_HE_STAR_REQUEST;
 if(mode==5&&original==M3_HE_DX_REQUEST)return M3_HE_STAR_DX_REQUEST;
 return original;
}
static inline int m3_codec_star_bytes(uint32_t bytes) {
 return bytes==M3_HE_STAR_BYTES || bytes==M3_HE_STAR_DX_BYTES;
}
static inline uint32_t m3_codec_decoder(uint32_t type,uint32_t format,uint32_t dimensions,
 uint64_t base,uint64_t data,uint32_t capacity,int star) {
 uint32_t needed=dimensions==((5520u<<16)|8280u)?M3_HE_STAR_BYTES:
  dimensions==((3608u<<16)|5408u)&&capacity<=M3_HE_STAR_DX_CAPACITY?M3_HE_STAR_DX_BYTES:0;
 if(star && type==3 && format==5 && needed && base && data>=base &&
    data-base<=capacity && needed<=capacity-(data-base) && capacity<=0x2000000u) return 4;
 return 2;
}
#endif

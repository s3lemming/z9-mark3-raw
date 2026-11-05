#ifndef MARK3_NATIVE_FRAME_GRADE_H
#define MARK3_NATIVE_FRAME_GRADE_H
#include <stdint.h>
/* Actual capture uses4105EB18 ImageInfo->M, not the alternate40ECCE6C
 * selector3 constructor. At4105ED60 M kind is initialized; M+24A0 is not.
 * Original ImageInfo+3D4 becomes M+290, then native JPEG coding/metadata. */
uint32_t m3_native_frame_grade_snapshot(uint32_t original);
uint32_t m3_native_imageinfo_grade_snapshot(uint32_t original,uint32_t kind);
#endif

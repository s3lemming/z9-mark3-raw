.text
.align 2
.global m3_native_imageinfo_grade_stub
.type m3_native_imageinfo_grade_stub,%function
/* Actual original4105ED60 STR W9,[X20,#290]. M kind is already initialized,
 * while mode+24A0 is later. Preserve every GPR, complete X8/X9 and NZCV. */
m3_native_imageinfo_grade_stub:
 stp x0,x1,[sp,#-176]!
 stp x2,x3,[sp,#16]
 stp x4,x5,[sp,#32]
 stp x6,x7,[sp,#48]
 stp x8,x9,[sp,#64]
 stp x10,x11,[sp,#80]
 stp x12,x13,[sp,#96]
 stp x14,x15,[sp,#112]
 stp x16,x17,[sp,#128]
 stp x18,x30,[sp,#144]
 mrs x10,nzcv
 str x10,[sp,#160]
 mov w0,w9
 ldr w1,[x20]
 bl m3_native_imageinfo_grade_snapshot
 ldr x10,[sp,#160]
 msr nzcv,x10
 ldr x1,[sp,#8]
 ldp x2,x3,[sp,#16]
 ldp x4,x5,[sp,#32]
 ldp x6,x7,[sp,#48]
 ldp x8,x9,[sp,#64]
 ldp x10,x11,[sp,#80]
 ldp x12,x13,[sp,#96]
 ldp x14,x15,[sp,#112]
 ldp x16,x17,[sp,#128]
 ldp x18,x30,[sp,#144]
 str w0,[x20,#0x290]
 ldr x0,[sp]
 add sp,sp,#176
 b m3_original_imageinfo_grade_continue
.size m3_native_imageinfo_grade_stub,.-m3_native_imageinfo_grade_stub
.section .note.GNU-stack,"",%progbits

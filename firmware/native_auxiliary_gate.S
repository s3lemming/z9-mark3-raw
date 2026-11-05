.text
.align 2
.macro SAVE_AUX
 sub sp,sp,#176
 stp x0,x1,[sp]
 stp x2,x3,[sp,#16]
 stp x4,x5,[sp,#32]
 stp x6,x7,[sp,#48]
 stp x8,x9,[sp,#64]
 stp x10,x11,[sp,#80]
 stp x12,x13,[sp,#96]
 stp x14,x15,[sp,#112]
 stp x16,x17,[sp,#128]
 stp x18,x30,[sp,#144]
 mrs x9,nzcv
 str x9,[sp,#160]
.endm
.macro RESTORE_AUX
 ldr x9,[sp,#160]
 msr nzcv,x9
 ldp x0,x1,[sp]
 ldp x2,x3,[sp,#16]
 ldp x4,x5,[sp,#32]
 ldp x6,x7,[sp,#48]
 ldp x8,x9,[sp,#64]
 ldp x10,x11,[sp,#80]
 ldp x12,x13,[sp,#96]
 ldp x14,x15,[sp,#112]
 ldp x16,x17,[sp,#128]
 ldp x18,x30,[sp,#144]
 add sp,sp,#176
.endm
.global m3_auxiliary8_gate
m3_auxiliary8_gate:
 b.eq 2f
 b m3_auxiliary8_allowed
2:
 SAVE_AUX
 bl m3_auxiliary_native_scope
 cbnz w0,1f
 RESTORE_AUX
 b m3_auxiliary8_rejected
1:
 RESTORE_AUX
 b m3_auxiliary8_allowed

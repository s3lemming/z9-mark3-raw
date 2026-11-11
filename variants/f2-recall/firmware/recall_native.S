.text
.align 2
.global f2r_getter_hook
.type f2r_getter_hook,%function
f2r_getter_hook:
 and w8,w0,#0xffff
 cmp w8,#0x675
 b.eq .Lslow
 cmp w8,#0x676
 b.eq .Lslow
 mov w9,#0x1792
 cmp w8,w9
 b.eq .Lslow
 mov w9,#0x793d
 cmp w8,w9
 b.ne f2r_original_get
.Lslow:
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
 bl f2r_original_get
 str x0,[sp,#160]
 mov w2,w0
 ldp x0,x1,[sp]
 bl f2r_getter_post
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
 ldr x0,[sp,#160]
 add sp,sp,#176
 ret
.size f2r_getter_hook,.-f2r_getter_hook
.global f2r_original_get
.type f2r_original_get,%function
f2r_original_get:
 .incbin "recall-original-get-prefix.bin"
 b f2r_native_get_continue
.size f2r_original_get,.-f2r_original_get
.global f2r_original_enter
.type f2r_original_enter,%function
f2r_original_enter:
 .incbin "recall-original-enter-prefix.bin"
 b f2r_native_enter_continue
.size f2r_original_enter,.-f2r_original_enter
.global f2r_original_exit
.type f2r_original_exit,%function
f2r_original_exit:
 .incbin "recall-original-exit-prefix.bin"
 b f2r_native_exit_continue
.size f2r_original_exit,.-f2r_original_exit
.section .note.GNU-stack,"",%progbits

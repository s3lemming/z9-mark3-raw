.text
.align 2
.global m3_live_bootstrap_stub
.type m3_live_bootstrap_stub,%function
/* Original40D6E5BC BL41016008, after its table+8 RTOS initialization.
 * Preserve all eight native argument registers across owner creation. */
m3_live_bootstrap_stub:
 stp x19,x30,[sp,#-80]!
 stp x0,x1,[sp,#16]
 stp x2,x3,[sp,#32]
 stp x4,x5,[sp,#48]
 stp x6,x7,[sp,#64]
#ifdef M3_INTEGRATED_AUDIO
 /* Arguments already saved; allocation does not move the native TTBR arena. */
 bl m23_audio_runtime_init
#endif
 add x0,sp,#80
 bl m3_live_bootstrap_before
 ldp x0,x1,[sp,#16]
 ldp x2,x3,[sp,#32]
 ldp x4,x5,[sp,#48]
 ldp x6,x7,[sp,#64]
 bl m3_original_settings_load
 mov w19,w0
 bl m3_live_bootstrap_after
 mov w0,w19
 ldp x19,x30,[sp],#80
 ret
.size m3_live_bootstrap_stub,.-m3_live_bootstrap_stub
.global m3_live_hsfc_fine_stub
.type m3_live_hsfc_fine_stub,%function
/* Original40D469C0 LDRB W19,[X8,#31]. W0 is the actual native1C26 value
 * queried just above. Preserve every other original register and NZCV. */
m3_live_hsfc_fine_stub:
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
 mrs x9,nzcv
 str x9,[sp,#160]
 ldrb w1,[x8,#31]
 bl m3_live_hsfc_fine
 mov w19,w0
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
 b m3_original_hsfc_fine_continue
.size m3_live_hsfc_fine_stub,.-m3_live_hsfc_fine_stub
.global m3_live_worker_client_stub
.type m3_live_worker_client_stub,%function
/* Original410161BC native worker F-client call, not the menu task. */
m3_live_worker_client_stub:
 stp x0,x1,[sp,#-32]!
 stp x2,x30,[sp,#16]
 mov x0,x2
 bl m3_live_worker_schema
 mov x2,x0
 ldp x0,x1,[sp]
 ldr x30,[sp,#24]
 add sp,sp,#32
 b m3_original_settings_client
.size m3_live_worker_client_stub,.-m3_live_worker_client_stub
.section .note.GNU-stack,"",%progbits

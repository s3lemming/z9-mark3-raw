#include "native_settings_context_abi.h"
.text
.align 2
.global m3_native_storage_save_stub
.type m3_native_storage_save_stub,%function
/* Whole original40D74BF4 entry; replay its displaced SUB in the trampoline.
 * Original core uses X19..25 and leaves callee-saved X28 intact. Cookie lives
 * on THIS invocation's208B stack frame(cookie96/API80/token24/binding8),
 * including nested calls, never in shared padding. Integration owns begin. */
m3_native_storage_save_stub:
 stp x28,x30,[sp,#-M3_NATIVE_SAVE_WRAPPER_BYTES]!
 add x28,sp,#16
 mov x2,x28
 bl m3_native_save_begin
 cmp w0,#2
 b.eq 1f
 ldp x0,x1,[x28]
 bl m3_native_storage_original
 b 2f
1: mov w0,wzr
2: mov w1,w0
 mov x0,x28
 bl m3_native_save_end
 ldp x28,x30,[sp],#M3_NATIVE_SAVE_WRAPPER_BYTES
 ret
.size m3_native_storage_save_stub,.-m3_native_storage_save_stub
.type m3_native_storage_original,%function
m3_native_storage_original:
 sub sp,sp,#0xe0
 b m3_native_original_core_continue
.size m3_native_storage_original,.-m3_native_storage_original
.global m3_native_save_force_stub
.type m3_native_save_force_stub,%function
/* Replace only original40D74D38 TBZ; original compare has just clobbered
 * caller-saved registers, and the continued core redefines them before use. */
m3_native_save_force_stub:
 mov w1,w0
 mov x0,x28
 bl m3_native_save_force
 tbz w0,#0,1f
 b m3_native_original_gate_continue
1: b m3_native_original_epilogue
.size m3_native_save_force_stub,.-m3_native_save_force_stub
.global m3_native_save_space_stub
.type m3_native_save_space_stub,%function
/* Original DC4 ADD w9,w9,8. Strict settings-domain calls add88. No original
 * bodybuffer or schema count is enlarged. X16..18 are dead here. */
m3_native_save_space_stub:
 ldr w16,[x28,#52]
 cmp w16,#1
 mov w17,#8
 mov w18,#88
 csel w16,w18,w17,eq
 add w9,w9,w16
 b m3_native_original_space_continue
.size m3_native_save_space_stub,.-m3_native_save_space_stub
.global m3_native_save_footer_stub
.type m3_native_save_footer_stub,%function
m3_native_save_footer_stub:
 ldr w16,[x28,#52]
 cmp w16,#1
 b.eq 1f
 mov x19,#0x56b0
 b m3_native_original_footer_continue
1: mov x0,x28
 mov x1,x20
 ldr w2,[sp,#0x9c]
 bl m3_native_save_footer
 b m3_native_original_epilogue
.size m3_native_save_footer_stub,.-m3_native_save_footer_stub
.section .note.GNU-stack,"",%progbits

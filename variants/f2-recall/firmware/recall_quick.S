/* P684: native targetCH2 selects continuous class. The validated private
 * mask selects QuickC30=3/C15=6 only after all original recall guards.
 * Non-CH native targets, invalid/legacy/private-native words and every
 * original downstream restriction retain the native behavior.
 * x8 and all originally untouched registers remain intact; x9/x10 restored. */
.text
.align 2
.global f2r_quick_normal_extend
.type f2r_quick_normal_extend,%function
f2r_quick_normal_extend:
 cmp w6,#2
 b.eq .Lnormal_admitted
 b f2r_quick_normal_cmp_continue
.Lnormal_admitted:
 stp x9,x10,[sp,#-16]!
 ldr w10,[sp,#32] /* original normal mask SP+16 */
 b .Lprivate
.size f2r_quick_normal_extend,.-f2r_quick_normal_extend
.global f2r_quick_hold_extend
.type f2r_quick_hold_extend,%function
f2r_quick_hold_extend:
 cmp w9,#2
 b.eq .Lhold_admitted
 b f2r_quick_hold_cmp_continue
.Lhold_admitted:
 stp x9,x10,[sp,#-16]!
 ldr w10,[sp,#72] /* original held mask SP+56 */
.Lprivate:
 lsr w9,w10,#18
 mov w1,#0x2c15 /* codec TAG B0540000 >>18, all14 tag bits */
 cmp w9,w1
 b.ne .Lnative_ch
 tbnz w10,#17,.Lnative_ch /* reserved must be0 */
 ubfx w9,w10,#13,#2
 cmp w9,#2
 b.hi .Lnative_ch /* invalid VR enum fails closed too */
 ubfx w9,w10,#15,#2
 cmp w9,#1
 b.eq .Lc15
 cmp w9,#2
 b.ne .Lnative_ch /* native0 or invalid3 */
 mov w1,#3
 b .Lselected
.Lc15:
 mov w1,#6
.Lselected:
 ldp x9,x10,[sp],#16
 b f2r_quick_common_continue
.Lnative_ch:
 ldp x9,x10,[sp],#16
 b f2r_quick_native_ch_continue
.size f2r_quick_hold_extend,.-f2r_quick_hold_extend
.section .note.GNU-stack,"",%progbits

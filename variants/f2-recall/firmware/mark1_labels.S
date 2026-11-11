/* AArch64 new-label-only UTF-16 resolver. QEMU placement is provisional.
 * ABI: w0=resource language, w1=label, x2=pointer out, x3=byte length out.
 * Original first instruction is displaced and replayed for every old label.
 * Resource languages 8/9 are Chinese; all other valid indices use English
 * for these two newly supplied strings, exactly as the v13 prototype did.
 */
.section .text.mark1_labels,"ax"
.global mark1_label_resolver
.type mark1_label_resolver,%function
mark1_label_resolver:
    sub w8,w1,#0xc84
    cmp w8,#1
    b.ls .Lnew
    mov x9,#0xab38
    b native_text_continue
.Lnew:
    cmp w0,#29
    b.hs .Linvalid_language
    mov w9,#0
    cmp w0,#8
    b.ne .Ltraditional
    mov w9,#1
    b .Lrecord
.Ltraditional:
    cmp w0,#9
    b.ne .Lrecord
    mov w9,#2
.Lrecord:
    adr x10,mark1_text_records
    add x10,x10,x9,lsl #5
    add x10,x10,w8,uxtw #4
    ldr x11,[x10]
    ldr w12,[x10,#8]
    str x11,[x2]
    str w12,[x3]
    ret
.Linvalid_language:
    str xzr,[x2]
    str wzr,[x3]
    ret
.size mark1_label_resolver,.-mark1_label_resolver

.section .rodata.mark1_labels,"a"
.balign 8
.global mark1_text_records
mark1_text_records:
    .quad mark1_en_title
    .word mark1_en_help-mark1_en_title,0
    .quad mark1_en_help
    .word mark1_en_end-mark1_en_help,0
    .quad mark1_sc_title
    .word mark1_sc_help-mark1_sc_title,0
    .quad mark1_sc_help
    .word mark1_sc_end-mark1_sc_help,0
    .quad mark1_tc_title
    .word mark1_tc_help-mark1_tc_title,0
    .quad mark1_tc_help
    .word mark1_tc_end-mark1_tc_help,0
mark1_en_title:
    .incbin "text_en_title.bin"
mark1_en_help:
    .incbin "text_en_help.bin"
mark1_en_end:
mark1_sc_title:
    .incbin "text_sc_title.bin"
mark1_sc_help:
    .incbin "text_sc_help.bin"
mark1_sc_end:
mark1_tc_title:
    .incbin "text_tc_title.bin"
mark1_tc_help:
    .incbin "text_tc_help.bin"
mark1_tc_end:

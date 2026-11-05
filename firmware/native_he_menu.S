.text
.align 2
.macro save_lookup_registers
 stp x0,x1,[sp,#-160]!
 stp x2,x3,[sp,#16]
 stp x4,x5,[sp,#32]
 stp x6,x7,[sp,#48]
 stp x8,x9,[sp,#64]
 stp x10,x11,[sp,#80]
 stp x12,x13,[sp,#96]
 stp x14,x15,[sp,#112]
 stp x16,x17,[sp,#128]
 stp x18,x30,[sp,#144]
.endm
.global m3_he_menu_global_stub
.type m3_he_menu_global_stub,%function
m3_he_menu_global_stub:
 cmp w0,#2
 b.ne m3_menu_original_lookup
 and w8,w1,#0xffff
 sub w8,w8,#0x6c4
 cmp w8,#1
 b.hi m3_menu_original_lookup
 save_lookup_registers
 bl m3_he_menu_global_lookup
 b .Llookup_return
.size m3_he_menu_global_stub,.-m3_he_menu_global_stub
.global m3_he_menu_ctor_stub
.type m3_he_menu_ctor_stub,%function
m3_he_menu_ctor_stub:
 cmp w0,#2
 b.ne m3_menu_original_lookup
 and w8,w1,#0xffff
 cmp w8,#0x2c
 b.eq .Lctor_owned
 sub w8,w8,#0x6c4
 cmp w8,#1
 b.hi m3_menu_original_lookup
.Lctor_owned:
 save_lookup_registers
 mov x2,x19
 bl m3_native_ctor_lookup_entry
.Llookup_return:
 /* The native getter normally writes only X0,X8..10,NZCV. Preserve even
  * caller-saved registers beyond AAPCS, including platform register X18.
  * Both slow paths have original slot2, so reproduce its scratch registers
  * and CMP2,6 flags while keeping the redirected result in X0. */
 ldr x1,[sp,#8]
 ldp x2,x3,[sp,#16]
 ldp x4,x5,[sp,#32]
 ldp x6,x7,[sp,#48]
 ldr x11,[sp,#88]
 ldp x12,x13,[sp,#96]
 ldp x14,x15,[sp,#112]
 ldp x16,x17,[sp,#128]
 ldp x18,x30,[sp,#144]
 add sp,sp,#160
 mov x8,#0x5de8
 movk x8,#0x32d,lsl #16
 movk x8,#0xa,lsl #32
 movk x8,#0,lsl #48
 ldr x8,[x8,#16]
 and w9,w1,#0xffff
 .inst M3_DISPLACED_ORR_WORD /* Original ORR W10,WZR,#96. */
 cmp w10,#100 /* Same NZCV as original slot2 CMP W0,#6. */
 ret
.size m3_he_menu_ctor_stub,.-m3_he_menu_ctor_stub
.global m3_he_menu_label_stub
.type m3_he_menu_label_stub,%function
m3_he_menu_label_stub:
 str x8,[sp,#-16]!
 mrs x8,nzcv
 str x8,[sp,#8]
 sub w8,w1,#0xc9c
 cmp w8,#3
 b.ls .Lnew_label
 ldr x8,[sp,#8]
 msr nzcv,x8
 ldr x8,[sp],#16
#ifdef M3_INTEGRATED_AUDIO
 /* mark2 handles only C86..C9A, then replays the original text instruction. */
 b mark2_discovery_label
#else
 mov x9,#0xab38
 b native_text_continue
#endif
.Lnew_label:
 ldr x8,[sp,#8]
 msr nzcv,x8
 ldr x8,[sp],#16
 b m3_native_group_label
.size m3_he_menu_label_stub,.-m3_he_menu_label_stub
.section .text.original_getter,"ax"
.align 2
.global m3_menu_original_lookup
.type m3_menu_original_lookup,%function
m3_menu_original_lookup:
 .incbin "original-getter.bin"
 .if (.-m3_menu_original_lookup)!=52
 .error "Complete original getter must be52 bytes."
 .endif
.size m3_menu_original_lookup,.-m3_menu_original_lookup
.section .rodata.menu_templates,"a"
.balign 8
.global m3_he_menu_templates
.type m3_he_menu_templates,%object
m3_he_menu_templates:
 .incbin "original532-menu-templates.bin"
 .if (.-m3_he_menu_templates)!=2156
 .error "Exact original native menu template required."
 .endif
.size m3_he_menu_templates,.-m3_he_menu_templates
.section .rodata.menu_link_row,"a"
.balign 4
.global m3_he_menu_submenu_link_row
.type m3_he_menu_submenu_link_row,%object
m3_he_menu_submenu_link_row:
 .incbin "original-submenu-link-row.bin"
 .if (.-m3_he_menu_submenu_link_row)!=44
 .error "Exact original PHOTO submenu-link row must be44 bytes."
 .endif
.size m3_he_menu_submenu_link_row,.-m3_he_menu_submenu_link_row
.section .note.GNU-stack,"",%progbits

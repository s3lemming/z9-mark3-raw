.section .text.entry,"ax"
.global mark1_entry
.type mark1_entry,%function
mark1_entry:
    stp x29,x30,[sp,#-16]!
    mov x29,sp
    mov w0,w22
    mov w1,w27
    bl mark1_action
    ldp x29,x30,[sp],#16
    b native_resume
.size mark1_entry,.-mark1_entry

import struct
from pathlib import Path

def generate(orig, O):
    BASE=0x40160000
    api=struct.unpack_from('<I',orig,0x40e355fc-BASE)[0]
    call=struct.unpack_from('<I',orig,0x40e355c4-BASE)[0];assert call&0xfc000000==0x94000000 and 0x40e355c4+((call&0x3ffffff)<<2)==0x40e355fc
    asm=['.section .text.hooks,"ax"']
    for name,func,pc,word in [('api','m2_api',0x40e355fc,api),('pcm','m2_pcm',0x40d25dd0,struct.unpack_from('<I',orig,0x40d25dd0-BASE)[0])]:
     assert struct.unpack_from('<I',orig,pc-BASE)[0]==word
     asm += [f'.global hook_{name}',f'hook_{name}:','sub sp,sp,#96','stp x0,x1,[sp]','stp x2,x3,[sp,#16]','stp x4,x5,[sp,#32]','stp x6,x7,[sp,#48]','str x30,[sp,#64]','mov x0,sp',f'bl {func}',*(['cbnz w0,api_rejected']if name=='api'else []),'ldp x0,x1,[sp]','ldp x2,x3,[sp,#16]','ldp x4,x5,[sp,#32]','ldp x6,x7,[sp,#48]',*(['bl original_api','str x0,[sp,#72]','mov x0,sp','bl m2_api_result','ldr x0,[sp,#72]','ldr x30,[sp,#64]','add sp,sp,#96','ret']if name=='api'else ['ldr x30,[sp,#64]','add sp,sp,#96',f'.inst {word}',f'b continue_{name}'])]
    for name,func,pc,word in [('flags','m2_flags',0x40f9f758,struct.unpack_from('<I',orig,0x40f9f758-BASE)[0]),('terminal','m2_terminal',0x40f9ff2c,struct.unpack_from('<I',orig,0x40f9ff2c-BASE)[0]),('fast_timer','m2_fast_timer',0x40f9fdf0,struct.unpack_from('<I',orig,0x40f9fdf0-BASE)[0])]:
     assert struct.unpack_from('<I',orig,pc-BASE)[0]==word
     asm += [f'.global hook_{name}',f'hook_{name}:','stp x0,x30,[sp,#-16]!',*(['mov x0,x30']if name=='terminal'else []),f'bl {func}','mov w9,w0','ldp x0,x30,[sp],#16','cbnz w9,1f',f'.inst {word}',f'b continue_{name}','1: ret']

    for name,func,pc in [('composite','m2_composite',0x40fa0130),('cxx_stop','m2_cxx_stop',0x40fa0198)]:
     word=struct.unpack_from('<I',orig,pc-BASE)[0]
     asm += [f'.global hook_{name}',f'hook_{name}:','sub sp,sp,#80','stp x0,x1,[sp]','stp x2,x3,[sp,#16]','stp x4,x5,[sp,#32]','stp x6,x7,[sp,#48]','str x30,[sp,#64]',f'bl {func}','ldp x0,x1,[sp]','ldp x2,x3,[sp,#16]','ldp x4,x5,[sp,#32]','ldp x6,x7,[sp,#48]','ldr x30,[sp,#64]','add sp,sp,#80',f'.inst {word}',f'b continue_{name}']
    # Input identity is checked before reading any displaced instruction.
    asm += ['.global hook_current','hook_current:','sub sp,sp,#128','stp x0,x1,[sp]','stp x2,x3,[sp,#16]','stp x4,x5,[sp,#32]','stp x6,x7,[sp,#48]','str x30,[sp,#64]','mov x0,sp','bl m2_frame_snapshot','ldp x0,x1,[sp]','ldp x2,x3,[sp,#16]','ldp x4,x5,[sp,#32]','ldp x6,x7,[sp,#48]','bl native_submit','str x0,[sp,#72]','mov x0,sp','bl m2_frame_commit','ldr x0,[sp,#72]','ldr x30,[sp,#64]','add sp,sp,#128','ret']

    asm+=['api_rejected:','ldr x30,[sp,#64]','add sp,sp,#96','mov w0,wzr','ret',
     '.global original_api','original_api:',f'.inst {api}','b continue_api']
    for i in range(64):
     asm += [f'.global notice_slot_{i}',f'notice_slot_{i}:','mov x2,x1','mov w1,w0',f'mov w0,#{i}','b m2_notice_slot']
    for name,pc in [('notice_sync_end',0x40e395b0),('notice_async_end',0x40e39728)]:
     word=struct.unpack_from('<I',orig,pc-BASE)[0]
     asm += ['.section .text.hooks,"ax"',f'.global hook_{name}',f'hook_{name}:','stp x0,x30,[sp,#-16]!','bl m2_notice_retired','ldp x0,x30,[sp],#16',f'.inst {word}',f'b continue_{name}']
    asm += ['.section .rodata.callbacks,"a"','.balign 8','.global callback_slots','callback_slots:']
    asm += [f'.quad notice_slot_{i}'for i in range(64)]
    label_word=struct.unpack_from('<I',orig,0x4032e57c-BASE)[0]
    asm += ['.section .text.labels,"ax"','.global mark2_discovery_label','mark2_discovery_label:',
     'cmp w1,#0xc86','b.lo 3f','cmp w1,#0xc9b','b.lo mark2_picker_label','3:',
     f'.inst {label_word}','b native_text_continue']
    (O/'hooks.S').write_text('\n'.join(asm)+'\n')

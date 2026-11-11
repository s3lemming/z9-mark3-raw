"""Extract authored-structure layout with AArch64 compiler; no firmware required."""
from pathlib import Path
import argparse,hashlib,json,struct,subprocess
P=Path(__file__).resolve().parents[1];ap=argparse.ArgumentParser();ap.add_argument('--output',required=True,type=Path);E=ap.parse_args().output.resolve()
assert not E.exists() and not E.is_relative_to(P),'New output outside repository required';E.mkdir(parents=True)
F=P/'variants/f2-recall/firmware'
fields={'pointer_bytes':'sizeof(void*)','runtime.bytes':'sizeof(struct runtime)','leaf.bytes':'sizeof(struct f2r_leaf)','event.bytes':'sizeof(struct m3_native_menu_event)','event.context':'offsetof(struct m3_native_menu_event,context)','event.target':'offsetof(struct m3_native_menu_event,target)','heap.bytes':'M23_AUDIO_BYTES','recall.heap_offset':'F2R_HEAP_OFFSET','recall.reserved_bytes':'F2R_HEAP_BYTES','frame.limit':'M3_FRAME_STATE_LIMIT','live_settings.limit':'M3_LIVE_SETTINGS_LIMIT'}
for n in ['magic','init_state','fault','vr_allowed','guard_column','guard_mask','enters','exits','gets','menu_commits','last_error','saves','notifies','guard_generation','leaf','save_gate_calls','save_gate_value','save_requests','save_batch_calls','save_batch_count','save_first_id','save_precheck_fault','save_return']:fields['runtime.'+n]='offsetof(struct runtime,'+n+')'
for n in ['state','holder','callbacks','rows','event_pointers','events']:fields['leaf.'+n]='offsetof(struct f2r_leaf,'+n+')'
(E/'layout.c').write_text('#include "native_runtime_layout.h"\n#include "recall_native.c"\n__attribute__((section(".rodata.porting_layout"))) const uint32_t porting_layout[]={'+','.join(fields.values())+'};\n')
commands=[]
for cmd in [['aarch64-linux-gnu-gcc','-std=c11','-O2','-ffreestanding','-fno-builtin','-mgeneral-regs-only','-ffixed-x18','-DM3_INTEGRATED_AUDIO=1','-I'+str(F),'-c',str(E/'layout.c'),'-o',str(E/'layout.o')],['aarch64-linux-gnu-objcopy','-O','binary','--only-section=.rodata.porting_layout',str(E/'layout.o'),str(E/'layout.bin')]]:
 q=subprocess.run(cmd,capture_output=True,text=True);commands.append(dict(argv=cmd,exit=q.returncode,stdout=q.stdout,stderr=q.stderr));(E/'commands.json').write_text(json.dumps(commands,indent=2));assert q.returncode==0,q.stderr
raw=(E/'layout.bin').read_bytes();assert len(raw)==len(fields)*4
layout=dict(zip(fields,struct.unpack('<'+'I'*len(fields),raw)));assert layout['pointer_bytes']==8 and layout['runtime.bytes']<=layout['recall.reserved_bytes'] and layout['runtime.save_gate_calls']==912
sources={p.relative_to(P).as_posix():hashlib.sha256(p.read_bytes()).hexdigest()for p in F.glob('*')if p.suffix in ['.c','.h']}
out=dict(schema='f2-authored-AArch64-layout-v1',compiler=subprocess.check_output(['aarch64-linux-gnu-gcc','-dumpfullversion'],text=True).strip(),layout=layout,sources=sources,scope='Compiled authored ABI only; no original firmware execution, hardware or cross-model address qualification')
(E/'layout.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(layout))

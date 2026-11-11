"""Exact original-input build; outputs are private and outside this repo."""
from pathlib import Path
import argparse,hashlib,json,struct,subprocess,sys
P=Path(__file__).resolve().parents[1]
ap=argparse.ArgumentParser();ap.add_argument('--module',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--version',choices=['5.32','5.60','5.61'],default='5.32');args=ap.parse_args()
if not __debug__:ap.error('Do not use Python -O')
profile=json.loads((P/'profile.json').read_text());raw=args.module.read_bytes()
assert hashlib.sha256(raw).hexdigest()==profile['original_module_SHA256'],'Exact original5.32 module required'
for name,h in profile['sources'].items():assert hashlib.sha256((P/name).read_bytes()).hexdigest()==h,name
B=args.output.resolve();assert not B.exists() and not B.is_relative_to(P.parents[1]),'New private output outside repository required';B.mkdir(parents=True)
for name,text in json.loads((P/'tools/authored-labels.json').read_text()).items():
 (B/(name+'.bin')).write_bytes(text.encode('utf-16-le'))
F=P/'firmware';AF=F/'mark2_audio';BASE=0x40160000;ENTRY=0x44f6c000;orig=raw[:-2];commands=[]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
recipe=json.loads((P/'tools/template-recipe.json').read_text())
chunks=[orig[c['offset']:c['offset']+c['bytes']] if 'offset' in c else bytes(c['zero_bytes']) for c in recipe['menu_chunks']]
blob=b''.join(chunks)+struct.pack('<3I',*recipe['event_counts']);assert hashlib.sha256(blob).hexdigest()==recipe['template_SHA256'];(B/'original532-menu-templates.bin').write_bytes(blob)
h=orig[recipe['header_offset']:recipe['header_offset']+recipe['header_bytes']];headers=[]
for edit in recipe['header_edits']:
 b=bytearray(h);struct.pack_into('<I',b,edit['offset'],edit['value']);headers.append(b)
assert hashlib.sha256(b''.join(headers)).hexdigest()==recipe['headers_binary_SHA256']
(B/'native_menu_headers.inc').write_text('static const uint8_t m3_native_page_headers[3][340] = {\n'+',\n'.join('{'+','.join(hex(x) for x in b)+'}' for b in headers)+'\n};\n')
(B/'original-getter.bin').write_bytes(orig[0x403d6618-BASE:0x403d664c-BASE]);(B/'original-submenu-link-row.bin').write_bytes(orig[0x431f30b4-BASE:0x431f30e0-BASE])
sys.path.insert(0,str(P/'tools'));from generate_audio import generate
generate(orig,B);(B/'audio_hooks.S').write_bytes((B/'hooks.S').read_bytes());(B/'backend_hooks.S').write_bytes((P/'tools/backend_hooks.S').read_bytes())
from recall_menu_patch import inputs as recall_inputs
recall_inputs(orig,B)
def run(argv):
 q=subprocess.run(argv,cwd=B,capture_output=True,text=True);commands.append(dict(argv=argv,exit=q.returncode,stdout=q.stdout,stderr=q.stderr));(B/'commands.json').write_text(json.dumps(commands,indent=2));assert q.returncode==0,q.stderr;return q.stdout
names=['mark1_toggle', 'native_frame_grade', 'native_he_menu', 'native_live_settings', 'native_settings_context', 'native_gate', 'native_settings_save', 'native_settings_nor', 'native_settings_verify', 'native_settings_view', 'native_settings_bank', 'native_settings_load', 'hsfc_settings_record', 'hsfc_settings_tail', 'native_menu_router', 'native_menu_pages', 'native_menu_labels', 'native_menu_callbacks', 'format_policy', 'native_he_backend', 'stock_nef_job', 'hsfc_geometry_mode', 'native_codec_live', 'native_preview_policy', 'native_hud_quality', 'native_hud_publish', 'native_auxiliary_gate', 'native_hud_remaining', 'native_buffer_publish', 'native_precapture_limit', 'mark2_audio_runtime', 'native_shotinfo_battery', 'native_shotinfo_bank12', 'native_shotinfo_bank_snapshot', 'native_shotinfo_axis25', 'native_shotinfo_section15', 'native_shotinfo_group1820', 'native_shotinfo_group1723', 'native_shotinfo_service1723', 'native_virtual_nrmap']
flags = ['-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', '-ffreestanding', '-fno-builtin', '-fno-stack-protector', '-fno-unwind-tables', '-fno-asynchronous-unwind-tables', '-mgeneral-regs-only', '-mstrict-align', '-fno-pic', '-march=armv8-a', '-mno-outline-atomics', '-ffixed-x18', '-fstack-usage', '-DM3_VIRTUAL_NRMAP=1', '-DM3_VIRTUAL_NRMAP_VALUE=0', '-DM3_NEF_S_KIND=1', '-DM3_SHOTINFO_BATTERY=1', '-DM3_SHOTINFO_BANK12=1', '-DM3_SHOTINFO_AXIS25=1', '-DM3_SHOTINFO_SECTION15=1', '-DM3_SHOTINFO_GROUP1820=1', '-DM3_SHOTINFO_GROUP1723=1', '-DM3_INTEGRATED_AUDIO=1', '-DM3_PROBE_ALLOW_DX=1', '-DM3_PROBE_ALLOW_FULL_HE_DX=0', '-I' + str(F), '-I' + str(AF), '-I' + str(B), '-DM3_DISPLACED_ORR_WORD=' + hex(struct.unpack_from('<I', orig, recipe['displaced_word_offset'])[0])]
names += ['recall_field_codec','recall_value_overlay','recall_native']
objects = []
for n in names:
    o = n + '.o'
    run(['aarch64-linux-gnu-gcc', *flags, '-c', str(F / (n + '.c')), '-o', o])
    objects.append(o)
for source, n in [(F / 'native_he_menu.S', 'menu_hooks.o'), (F / 'native_settings_save.S', 'settings_hooks.o'), (F / 'native_live_bootstrap.S', 'bootstrap.o'), (F / 'native_frame_grade.S', 'frame_grade_hooks.o'), (F / 'native_auxiliary_gate.S', 'auxiliary_hooks.o'), (B / 'backend_hooks.S', 'backend_hooks.o')]:
    run(['aarch64-linux-gnu-gcc', *flags, '-c', str(source), '-o', n])
    objects.append(n)
run(['aarch64-linux-gnu-gcc',*flags,'-c',str(F/'recall_native.S'),'-o','recall_hooks.o']);objects.append('recall_hooks.o')
run(['aarch64-linux-gnu-gcc',*flags,'-c',str(B/'recall_vr_header.c'),'-o','recall_vr_header.o']);objects.append('recall_vr_header.o')
for unit in ['adapter', 'picker', 'mark2_import', 'mark2_native_file', 'profile', 'profile_native', 'runtime']:
    o = 'audio_' + unit + '.o'
    run(['aarch64-linux-gnu-gcc', *flags, '-c', str(AF / (unit + '.c')), '-o', o])
    objects.append(o)
run(['aarch64-linux-gnu-gcc', *flags, '-c', 'audio_hooks.S', '-o', 'audio_hooks.o'])
objects.append('audio_hooks.o')
for unit in ['mark1_entry','mark1_labels']:
 run(['aarch64-linux-gnu-gcc',*flags,'-Wa,-I'+str(F),'-c',str(F/(unit+'.S')),'-o',unit+'.o']);objects.append(unit+'.o')

symbols = dict(native_get=0x41019238,native_set=0x40d57380,native_options=0x41018524,native_resume=0x40d43b48,m3_auxiliary8_allowed=1090937908, m3_auxiliary8_rejected=1090937900, codec_decoder_continue=1089130804, native_text_continue=1077077376, m3_native_original_core_continue=1087851512, m3_native_original_gate_continue=1087851836, m3_native_original_epilogue=1087852412, m3_native_original_space_continue=1087851976, m3_native_original_footer_continue=1087852316, m3_original_settings_load=1090609160, m3_original_settings_client=1087838012, m3_original_hsfc_fine_continue=1087662532, m3_original_imageinfo_grade_continue=1090907492, native_E_release=1090861180, native_compose=1080855960, retain_continue=1089131088)
symbols.update(f2r_quick_native_ch_continue=0x40ed81fc)
symbols.update(f2r_quick_normal_cmp_continue=0x40ed81c0,f2r_quick_hold_cmp_continue=0x40ed81f8,f2r_quick_common_continue=0x40ed81c4)
symbols.update(f2r_native_get_continue=0x41019248,f2r_native_enter_continue=0x40ed2768,f2r_native_exit_continue=0x40ed2858)
for name, value in json.loads((P / 'tools/audio-symbols.json').read_text()).items():
    assert name not in symbols or symbols[name] == value
    symbols[name] = value
(B / 'layout.ld').write_text(' '.join((f'{n} = 0x{v:x};' for n, v in symbols.items())) + ' SECTIONS { . = 0x44f6c000; .text : { *(.text*) } .rodata : { *(.rodata*) } /DISCARD/ : { *(.comment) *(.eh_frame*) *(.note*) } }\n')
run(['aarch64-linux-gnu-gcc',*flags,'-c',str(F/'recall_quick.S'),'-o','recall_quick.o']);objects.append('recall_quick.o')
run(['aarch64-linux-gnu-ld', '--build-id=none', '-T', 'layout.ld', *objects, '-o', 'menu.elf'])
assert not run(['aarch64-linux-gnu-nm', '-u', 'menu.elf']).strip()
s = {v[2]: int(v[0], 16) for line in run(['aarch64-linux-gnu-nm', '-n', 'menu.elf']).splitlines() if len((v := line.split())) == 3}
sections = run(['aarch64-linux-gnu-objdump', '-h', 'menu.elf'])
assert not any((line.split()[1] in ['.bss', '.data', '.got', '.tdata', '.tbss'] and int(line.split()[2], 16) for line in sections.splitlines() if len(line.split()) >= 3 and line.split()[0].isdigit()))
run(['aarch64-linux-gnu-objcopy', '-O', 'binary', 'menu.elf', 'menu.bin'])
(B / 'disassembly.txt').write_text(run(['aarch64-linux-gnu-objdump', '-d', 'menu.elf']))
code = (B / 'menu.bin').read_bytes()
end = ENTRY + len(code) + 4095 & ~4095
assert BASE + len(orig) < ENTRY and end < 1167589376
out = bytearray(orig)
out.extend(bytes(ENTRY - BASE - len(out)))
out.extend(code)
patches = []

def off(a):
    return a - 42913017856 if 42991648768 <= a < 42994561024 else a - BASE

def branch(pc, to, link=False):
    d = to - pc
    assert not d % 4 and -(1 << 27) <= d < 1 << 27
    return struct.pack('<I', (2483027968 if link else 335544320) | d // 4 & 67108863)

def patch(a, new, expected, consumer):
    i = off(a)
    old = orig[i:i + len(new)]
    guard = profile['patches'][hex(a)]
    assert len(new) == guard['bytes'] and hashlib.sha256(old).hexdigest() == guard['original_SHA256'] and (out[i:i + len(new)] == old)
    out[i:i + len(new)] = new
    patches.append(dict(VA=hex(a), bytes=len(new), original_SHA256=hashlib.sha256(old).hexdigest(), new_SHA256=hashlib.sha256(new).hexdigest(), consumer=consumer))

def audio_patch(a, new, why):
    i = off(a)
    patch(a, new, None, why)

def append(blob, align=16):
    out.extend(bytes(-len(out) % align))
    p = len(out)
    out.extend(blob)
    return BASE + p
rows = orig[1125685156 - BASE:1125685156 - BASE + 5 * 44]
new = bytearray(rows[-44:])
struct.pack_into('<H', new, 0, 3206)
struct.pack_into('<I', new, 24, 32288)
rp = append(rows + new)
ep = append(orig[1125685376 - BASE:1125685376 - BASE + 40] + 21 * orig[1125685376 - BASE:1125685376 - BASE + 8])
audio_patch(42992478656, struct.pack('<Q', rp), 'Sound rows: five originals byte-identical plus Load WAV from card')
audio_patch(42992478664, struct.pack('<Q', ep), '26 original-compatible event bindings; 20 displayed files')
audio_patch(42992478672, struct.pack('<H', 6), 'Initial six sound rows; scanned filenames published by native task')
for pc in [42992478680, 42992478688]:
    audio_patch(pc, struct.pack('<H', 416), 'Bound native scrolling list rectangle')
classes = orig[1114585893 - BASE:1114585893 - BASE + 3204]
cp = append(classes + 28 * classes[2366:2367], 1)
words = memoryview(orig[:20709376]).cast('I')
sites = []
for i, w in enumerate(words):
    if w & 4286578688 != 3531603968 or w >> 5 & 65535 != 15141:
        continue
    reg = w & 31
    value = w >> 5 & 65535
    pcs = [BASE + i * 4]
    for j in range(i + 1, min(i + 10, len(words))):
        n = words[j]
        if n & 4286578719 == 4068474880 | reg:
            shift = (n >> 21 & 3) * 16
            value = value & ~(65535 << shift) | (n >> 5 & 65535) << shift
            pcs.append(BASE + j * 4)
            if shift == 48:
                break
    if value != 1114585893:
        continue
    sites.append(pcs)
    for pc in pcs:
        word = struct.unpack_from('<I', orig, pc - BASE)[0]
        shift = (word >> 21 & 3) * 16
        audio_patch(pc, struct.pack('<I', word & ~(65535 << 5) | (cp >> shift & 65535) << 5), 'Extended label classifier; original entries unchanged')
assert sites
end = BASE + len(out) + 4095 & ~4095
for pc, name in [(1088640508, 'api'), (1087528400, 'pcm'), (1090123608, 'flags'), (1090125612, 'terminal'), (1090125296, 'fast_timer'), (1090126128, 'composite'), (1090126232, 'cxx_stop'), (1088656816, 'notice_sync_end'), (1088657192, 'notice_async_end')]:
    audio_patch(pc, branch(pc, s['hook_' + name]), 'Integrated mark2 v13 ' + name + '; displaced original instruction replayed')
for pc in [1125916664, 1125916696, 1125916856]:
    assert struct.unpack_from('<Q', orig, off(pc))[0] == 1079662560
    audio_patch(pc, struct.pack('<Q', s['mark2_discovery_submit']), 'Native sound selection preserves five original choices and CUSTOM_SOUND picker')
audio_patch(42992616104, struct.pack('<Q', s['mark2_sound_page_refresh']), 'P589 parent page entry restores confirmed custom sound and filename model before original405A54B4 values')
audio_patch(42992616152, struct.pack('<Q', s['mark2_sound_refresh']), 'Native sound refresh followed by selected custom filename')
audio_patch(1079661956, branch(1079661956, s['mark2_outer_type_text'], True), 'Only outer sound Type label CUS; native enum preserved')
root_rows = struct.unpack_from('<Q', orig, off(42992485600 + 16))[0]
root_HSFC = root_rows + 44
old_row = orig[off(root_HSFC):off(root_HSFC) + 44]
assert old_row == (B / 'original532-menu-templates.bin').read_bytes()[476:520]
assert struct.unpack_from('<I', old_row, 24)[0] == 11720
blank_link = bytearray(orig[1126117556 - BASE:1126117600 - BASE])
blank_link[:4] = old_row[:4]
blank_link[24:28] = old_row[24:28]
patch(root_HSFC, bytes(blank_link), None, 'Image-quality HSFC parent keeps original label/UI/event row, pure submenu link with no4BEC right-value widget; private C15/C30 and C60/C120 rows publish separate committed summaries')
patch(1077599684, branch(1077599684, s['m3_he_menu_ctor_stub'], True), None, 'Complete original public constructor wrapper retained')
patch(1077765656, branch(1077765656, s['m3_he_menu_global_stub']), None, 'Exact slot2 private6C4/5 only, other original getter GPR/NZCV unchanged')
patch(1077077372, branch(1077077372, s['m3_he_menu_label_stub']), None, 'C9C/C9D localized group headers only')
patch(1087825340, branch(1087825340, s['m3_live_bootstrap_stub'], True), None, 'After original RTOS init, create one F owner; unchanged native load then owned accepted-bank private load')
patch(1090609596, branch(1090609596, s['m3_live_worker_client_stub'], True), None, 'Original native worker claims one fully published menu request schema; original F-client runs on native worker stack')
patch(1087662528, branch(1087662528, s['m3_live_hsfc_fine_stub']), None, 'Original condition7 fine byte, native C30/C15 modes3/6 only; complete instruction register/NZCV preservation, actual DQT binding pending')
patch(1090907488, branch(1090907488, s['m3_native_imageinfo_grade_stub']), None, 'Actual39 observed native ImageInfo3D4 to M290 stores; Mkind3 and effective C15/C30 guards; every GPR/NZCV retained, no original6F10 change')
for pc, name, old, why in [(1087851508, 'm3_native_storage_save_stub', 'ff8303d1', 'Complete core invocation owner, separate224B wrapper'), (1087851832, 'm3_native_save_force_stub', '20120036', 'Private change forces native dirty gate'), (1087851972, 'm3_native_save_space_stub', '29210011', 'Reserve88 tail bytes only in exact F domain'), (1087852312, 'm3_native_save_footer_stub', '13d68ad2', 'Private tail last commit, original non-domain footer replay')]:
    patch(pc, branch(pc, s[name]), None, why)
patch(1089131076, branch(1089131076, s['hook_retain']) + bytes.fromhex('1f2003d51f2003d5'), None, 'Retain E only for admitted committed RAW setting')
patch(1089165464, branch(1089165464, s['hook_compose'], True), None, 'Original JPEG compose then private native NEF')
patch(1089107736, branch(1089107736, s['hook_write'], True), None, 'Admitted RAW-only or RAW+JPEG writer; native JPEG fallback is reported as RAW failure')
patch(1080861944, branch(1080861944, s['m3_probe_photo_completion'], True), None, 'Only original photo-completion event2 at RAW-only owned WRITING job is deferred until successful commit and JPEG reservation cancellation; original payload and dispatcher/guards preserved')
patch(1090861180, branch(1090861180, s['m3_probe_native_free']), None, 'Original E free observer and generation cleanup')
patch(1090909540, branch(1090909540, s['m3_hsfc_geometry_mode']), None, 'Existing C15/C30 DX native HE geometry mapping')
patch(1090937896, branch(1090937896, s['m3_auxiliary8_gate']), None, 'Scoped C15/C30 RAW auxiliary8 original initializer; original fallback and all registers/NZCV preserved')
patch(1089154120, branch(1089154120, s['m3_native_preview_parameters'], True), None, 'RAW-only original NEF preview RC; RAW+JPEG/JPEG fine-normal unchanged')
patch(1089067156, branch(1089067156, s['m3_codec_tenc_config'], True), None, 'Original Tenc config first; committed HE-star in guarded FX/DX C15/C30 image-info contract')
patch(1089015988, branch(1089015988, s['m3_codec_tenc_config'], True), None, 'Second original Tenc initializer uses the same guarded HE-star bridge')
patch(1090861108, branch(1090861108, s['hook_codec_alloc'], True), None, 'Exact E factory original HE request/pool39/FX or DX image-info scope changes to matching HE-star allocation')
for pc in [1089067204, 1090109524, 1090114120]:
    patch(pc, branch(pc, s['m3_codec_E_size'], True), None, 'Native E size accounting follows selected HE-star; original non-HE request sizes unchanged')
patch(1089130800, branch(1089130800, s['hook_codec_decoder']), None, 'Packet-qualified HE-star decoder; all other GPR/NZCV preserved')
patch(1090111316, branch(1090111316, s['m3_native_buffer_publish'], True), None, 'Original buffer publication unchanged first; enqueue GUI24 when native adjusted availability changes')
patch(1077834920, branch(1077834920, s['m3_precapture_property_get']), None, 'Original property getter target first; only02006401 enum3 becomes effective2 in committed RAW-bearing C30 FX HE-star, all other properties/modes/settings preserve native value')
patch(42992605072, struct.pack('<Q', s['m3_precapture_menu_refresh']), None, 'Exact original pre-duration state18F refresh: native mark/focus using effective getter, disable1s row35BE only for C30 FX HE-star and reenable on scope exit')
patch(1090097840, branch(1090097840, s['m3_precapture_frames'], True), None, 'Bound same effective pre-duration by applied C30/FX/RAW/HE-star before complete original per-mode frame conversion; no native buffer guard or pool changes')
patch(1090617040, branch(1090617040, s['m3_precapture_native_batch'], True), None, 'Complete original queued ID/value publication first and preserve return; original07000603 area batch in C30 HE-star RAW requests original6401 recompute after queue unlock, no derived-cache/persistent/reserve mutation')
for index, name, old in [(24, 'm3_hud_remaining_callback', 1080749984), (112, 'm3_hud_quality_callback', 1080711932), (113, 'm3_hud_quality_callback', 1080711932), (116, 'm3_hud_star_callback', 1080712372), (93, 'm3_hud_mode_callback', 1080713928), (129, 'm3_hud_mode_callback', 1080713928), (130, 'm3_hud_mode_callback', 1080713928)]:
    patch(42993075872 + index * 104 + 48, struct.pack('<Q', s[name]), None, 'Exact native GUI record callback only; scoped private quality, original fallback, native bounded refresh')
from recall_menu_patch import integrate as recall_integrate
recall_integrate(orig,out,s,append,patch,branch,B)
end = BASE+len(out)+4095 & ~4095
patch(42994520080, struct.pack('<IIQQQ', 2, 15, ENTRY, end, ENTRY), None, 'Explicit authored RX mapping')
patch(42994520288, struct.pack('<Q', ENTRY), None, 'Original resource RO upper boundary')
size = struct.unpack_from('<Q', orig, off(1075216616))[0]
assert size + 42994847744 == 43031711744 and 43031711744 + 8192 + 4096 + 16383 & ~16383 == 43031724032
patch(1075216616, struct.pack('<Q', size + 8192), None, 'Startup-zeroed D frame plus bounded D+C00 settings/E UI; native extra guard page included, TTBR0 unchanged')
payload = B / 'payload.bin'
from integrate_mark1 import integrate
integrate(P,B,orig,out,s,cp,patches)
payload.write_bytes(out)

import binascii
prefix=bytes(out)
assert len(prefix)==profile['expected_prefix_bytes'] and hashlib.sha256(prefix).hexdigest()==profile['expected_prefix_SHA256']
(B/'prefix.bin').write_bytes(prefix);final=bytearray(prefix+b'Ver.05.32.00\0')
if args.version!='5.32':
 for at in [0x14a4a8c,0x4d2049f,0x4e08780,len(final)-13]:
  assert final[at:at+13]==b'Ver.05.32.00\0';final[at:at+13]=('Ver.05.'+args.version.split('.')[1]+'.00\0').encode()
assert final[-13:]==('Ver.05.'+args.version.split('.')[1]+'.00\0').encode()
assert hashlib.sha256(final).hexdigest()==profile['versions'][args.version]['MAIN_SHA256']
(B/'payload.bin').write_bytes(final);(B/'eg1990_053200.bi').write_bytes(final+struct.pack('>H',binascii.crc_hqx(final,0)))
report=dict(passed=True,version=args.version,prefix_SHA256=hashlib.sha256(prefix).hexdigest(),MAIN_SHA256=hashlib.sha256(final).hexdigest(),bytes=len(final),patches=patches,device_qualified=False)
(B/'build-report.json').write_text(json.dumps(report,indent=2));print(json.dumps({k:v for k,v in report.items() if k!='patches'}))

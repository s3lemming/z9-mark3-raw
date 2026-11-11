"""Original-module-guarded f2 template relocation, namespace stays page-local."""
import hashlib,json,struct

def inputs(orig,B):
    base=0x40160000
    for n,p in [('get',0x41019238),('enter',0x40ed2758),('exit',0x40ed2848)]:
        raw=orig[p-base:p-base+16]
        assert len(raw)==16
        (B/f'recall-original-{n}-prefix.bin').write_bytes(raw)
    # Exact original f2 release-page header (six widgets,544 bytes).
    # Keep the native f2/button caption contract; replace its subtitle only.
    raw=bytearray(orig[0x42edee78-base:0x42edee78-base+544])
    assert struct.unpack_from('<I',raw,136)[0]==0x638 and struct.unpack_from('<I',raw,204)[0]==0x3d1
    struct.pack_into('<I',raw,204,0x5c8)
    state=orig[0xa02909d90-0x9fdd0b000:0xa02909d90-0x9fdd0b000+96]
    holder=orig[0xa028c2e70-0x9fdd0b000:0xa028c2e70-0x9fdd0b000+80]
    assert struct.unpack_from('<I',state)[0]==0x2350002 and struct.unpack_from('<H',holder,32)[0]==3
    generated='#include <stdint.h>\n'
    for name,data in [('f2r_vr_header',raw),('f2r_vr_state',state),('f2r_vr_holder',holder)]:
        generated+='const uint8_t '+name+'['+str(len(data))+']={'+','.join(hex(v) for v in data)+'};\n'
    (B/'recall_vr_header.c').write_text(generated)

def integrate(orig,out,s,append,patch,branch,B):
    base=0x40160000
    def at(a):
        if 0xa02808000<=a<0xa02acf000:return a-0x9fdd0b000
        assert base<=a<base+len(orig),hex(a)
        return a-base
    def rd(a,n):return orig[at(a):at(a)+n]
    def q(a):return struct.unpack('<Q',rd(a,8))[0]
    def event_table(p):
        rows=[]
        for i in range(32):
            row=bytearray(rd(p+i*32,32));event=struct.unpack_from('<Q',row)[0]
            assert event<=0xffff
            rows.append(row)
            if event==0x3ca:return rows
        raise AssertionError('Missing bounded native event terminal')
    changes=[]
    # Original12 checkbox positions retained; original Save row moves12->13.
    holder=0xa028c2970;old=rd(q(holder+16),13*44)
    vr=bytearray(old[7*44:8*44]);struct.pack_into('<I',vr,0,0x5c8);struct.pack_into('<I',vr,24,0x7ea0)
    rows=old[:12*44]+vr+old[12*44:]
    table=event_table(q(q(holder+24)+11*8));assert len(table)==22
    for row in table:
        event,action,condition,target=struct.unpack('<4Q',row)
        if event in (0x298,0x2b1):action=s['f2r_vr_open'];target=0x6c6
        elif event in (0x29d,0x2c0):action=s['f2r_vr_toggle'];target=0x6b8
        elif event==0x315:action=s['f2r_vr_aux']
        row[:]=struct.pack('<4Q',event,action,condition,target)
    ev=append(b''.join(table),8);old_ptrs=list(struct.unpack('<13Q',rd(q(holder+24),13*8)))
    saved=event_table(old_ptrs[12]);assert len(saved)==21
    for row in saved:
        if struct.unpack_from('<Q',row,8)[0]==0x405338ac:struct.pack_into('<Q',row,8,s['f2r_save_current_action'])
    save_events=append(b''.join(saved),8)
    rp=append(rows,4);ep=append(struct.pack('<14Q',*old_ptrs[:12],ev,save_events),8)
    for a,v,n in [(holder+16,rp,8),(holder+24,ep,8),(holder+32,14,2)]:patch(a,v.to_bytes(n,'little'),None,'P598 recall parent14 rows and original Save-current retained')
    for a,name in [(0xa028f1fd8,'f2r_parent_refresh'),(0xa028f1ff8,'f2r_parent_values')]:patch(a,struct.pack('<Q',s[name]),None,'P598 original12 values first, then owned VR checkbox/target and Cxx summary')
    # Release5 rows. Native CH/CL right-navigation remains to speed children.
    h=0xa028c2e70;old=rd(q(h+16),3*44);r15=bytearray(old[88:132]);r30=bytearray(r15)
    struct.pack_into('<I',r15,0,0x95e);struct.pack_into('<I',r15,24,0x7ea1)
    struct.pack_into('<I',r30,0,0x753);struct.pack_into('<I',r30,24,0x7ea2)
    ptr=[];sources=list(struct.unpack('<3Q',rd(q(h+24),24)))
    for i in range(5):
        table=event_table(sources[min(i,2)]);assert len(table)==16
        for row in table:
            event,action,condition,target=struct.unpack('<4Q',row)
            if action==0x4051121c:action=s['f2r_release_submit']
            row[:]=struct.pack('<4Q',event,action,condition,target)
        ptr.append(append(b''.join(table),8))
    rp=append(old+r15+r30,4);ep=append(struct.pack('<5Q',*ptr),8)
    for a,v,n in [(h+16,rp,8),(h+24,ep,8),(h+32,5,2)]:patch(a,v.to_bytes(n,'little'),None,'P598 release CH/CL/S/C15/C30; original first3 definitions unchanged')
    patch(0xa028f2258,struct.pack('<Q',s['f2r_release_refresh']),None,'P598 original speed values and five-row selection refresh')
    # Native speed children, copy exact bounded events and replace only the
    # original submit callback. Do not patch entries called by our wrappers.
    for state,old_action,new_name in [(0x236,0x405110e0,'f2r_ch_submit'),(0x237,0x40511184,'f2r_cl_submit')]:
        va=0xa02909790+(state-0x225)*96;h=q(va+80);n=struct.unpack('<H',rd(h+32,2))[0];old_ptrs=struct.unpack('<'+'Q'*n,rd(q(h+24),n*8));new=[];seen={};replaced=0
        for p in old_ptrs:
            if p not in seen:
                table=event_table(p)
                for row in table:
                    if struct.unpack_from('<Q',row,8)[0]==old_action:struct.pack_into('<Q',row,8,s[new_name]);replaced+=1
                seen[p]=append(b''.join(table),8)
            new.append(seen[p])
        assert replaced>0
        ep=append(struct.pack('<'+'Q'*n,*new),8);patch(h+24,struct.pack('<Q',ep),None,'P598 native speed submit clears private Cxx override only after original acceptance')
        changes.append(dict(state=hex(state),rows=n,replaced_events=replaced))
    for pc,name,link in [(0x41019238,'f2r_getter_hook',False),(0x40ed2758,'f2r_enter',False),(0x40ed2848,'f2r_exit',False),(0x410dc734,'f2r_save_batch',True),(0x410dc760,'f2r_save_notify',True)]:
        patch(pc,branch(pc,s[name],link),None,'P598 '+name+'; original trampolines or exact native batch/notify destination preserved')
    patch(0x4050ce44,branch(0x4050ce44,s['f2r_save_permission'],True),None,'P598 read-only original Save-current gate result diagnostics; original predicate and result unchanged')
    (B/'recall-integration.json').write_text(json.dumps(dict(original_MAIN_SHA256=hashlib.sha256(orig).hexdigest(),parent_rows=14,release_rows=5,private_state='slot2/6C6; guarded owned heap only',heap=dict(audio=[0,8192],mark1=[8192,8320],recall=[8320,10240],bytes=10240,extra_BSS_pages=0),speed_children=changes,save='One original24-field batch and one24-ID native notify; first23 values unchanged',VR='Read-only overlay; same-generation original availability and active/column guards; original base untouched',qualified=False),indent=2)+'\n')

    # Exact compare sites reached only after the native recall guards above.
    patch(0x40ed81bc,branch(0x40ed81bc,s['f2r_quick_normal_extend']),None,'P682 normal admitted C15/C30 Quick submode; original fallback and restrictions')
    patch(0x40ed81f4,branch(0x40ed81f4,s['f2r_quick_hold_extend']),None,'P682 held admitted C15/C30 Quick submode; original fallback and restrictions')

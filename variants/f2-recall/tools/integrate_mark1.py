"""Guarded private integration of archived mark1 tables with linked source."""
from pathlib import Path
import hashlib,json,struct

def integrate(P,B,orig,out,s,classification,patches):
    manifest=json.loads((P/'tools/mark1-menu-recipe.json').read_text())
    decoded={}
    for r in manifest['regions']:
        b=b''.join(orig[x['offset']:x['offset']+x['bytes']] if 'offset' in x else bytes.fromhex(x['authored_hex']) for x in r['parts'])
        assert len(b)==r['bytes'] and hashlib.sha256(b).hexdigest()==r['sha256']
        decoded[r['name']]=b
    base=0x40160000;mapping={};blobs=[]
    for r in manifest['regions']:
        name=r['name'];old=int(r['address'],0);raw=decoded[name]
        assert hashlib.sha256(raw).hexdigest()==r['sha256']
        if name in ['compiled_native_BSS_ACTION1_and_label_resolver','terminal_version_footer_repair','title_classification_3206']:continue
        out.extend(bytes(-len(out)%16));new=base+len(out);mapping[old]=(new,len(raw),name)
        blobs.append((len(out),raw,name));out.extend(raw)
    mapping[0x44f6c000]=(s['mark1_entry'],1,'linked source entry')
    def remap(value):
        for old,(new,n,_) in mapping.items():
            if old<=value<old+n:return new+value-old
        return value
    relocations=[]
    for off,raw,name in blobs:
        for j in range(0,len(raw)-7,4):
            v=struct.unpack_from('<Q',raw,j)[0];n=remap(v)
            if n!=v:
                struct.pack_into('<Q',out,off+j,n);relocations.append(dict(region=name,offset=j,old=hex(v),new=hex(n)))
    byname={name:new for _,(new,_,name) in mapping.items()}
    def at(a):return a-0x9fdd0b000 if 0xa02808000<=a<0xa02acf000 else a-base
    def write(a,new,why):
        i=at(a);before=bytes(out[i:i+len(new)]);assert len(before)==len(new)
        if before==new:return
        out[i:i+len(new)]=new;patches.append(dict(VA=hex(a),bytes=len(new),original_SHA256=hashlib.sha256(before).hexdigest(),new_SHA256=hashlib.sha256(new).hexdigest(),consumer='P594 '+why))
    shared={0x4032e57c,0x403e9664,0x403e9668,0xa02ac5010,0xa02ac50e0,0x401680e8}
    for row in manifest['patches']:
        a=int(row['address'],0)
        if a in shared:continue
        i=at(a);raw=bytes.fromhex(row['new_hex']);old=orig[i:i+len(raw)];assert hashlib.sha256(old).hexdigest()==row['original_SHA256'];why=row['meaning']
        assert orig[i:i+len(old)]==old and out[i:i+len(old)]==old,(hex(a),why,'unexpected baseline overlap')
        target=None
        if 'total table relocation' in why:target=byname['f2_allocation_table_122']
        elif 'allowed UI list relocation' in why:target=byname['Fn1_photo_allow_list_72']
        elif 'new primary dispatch table' in why:target=byname['primary_backend_dispatch_79']
        if target is not None:
            word=struct.unpack('<I',raw)[0];assert word&0x1f800000==0x12800000
            shift=((word>>21)&3)*16;raw=struct.pack('<I',(word&~(0xffff<<5))|(((target>>shift)&0xffff)<<5))
        elif len(raw)==8:raw=struct.pack('<Q',remap(struct.unpack('<Q',raw)[0]))
        write(a,raw,why)
    # Preserve the v39 C86..C9F classification while assigning only mark1 IDs.
    cls=next(r for r in manifest['regions'] if r['name']=='title_classification_3206')
    clsbytes=decoded[cls['name']]
    write(classification+3204,clsbytes[3204:3206],'merge only C84/C85 title classes')
    end=(base+len(out)+4095)&~4095
    write(0xa02ac5010,struct.pack('<IIQQQ',2,15,0x44f6c000,end,0x44f6c000),'extend existing single RX mapping over linked mark1 and relocated tables')
    assert struct.unpack_from('<Q',out,0x80e8)[0]==struct.unpack_from('<Q',orig,0x80e8)[0]+8192
    assert byname['primary_backend_dispatch_79']>0x44f6c000
    assert len(out)<0x45a80000-base
    (B/'mark1-integration.json').write_text(json.dumps(dict(passed_static=True,runtime_qualified=False,baseline_mark1_SHA256='f8e13d0b78cc4a63f5262ef1d5a1d49b6395f84627673f13e418b0daf9ae8031',heap=dict(audio=[0,8192],mark1=[8192,8320],recall=[8320,10240],bytes=10240,startup_BSS_pages=2,TTBR_origin_unchanged=True),mapping={hex(k):dict(new=hex(v[0]),bytes=v[1],name=v[2]) for k,v in mapping.items()},pointer_relocations=relocations,label_chain='v39 C9C-C9F -> mark2 C86-C9A -> mark1 C84/C85 -> original',mark1_action=hex(s['mark1_action']),mark1_entry=hex(s['mark1_entry']),RX_end=hex(end)),indent=2)+'\n')

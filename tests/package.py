"""Two synthetic version footers and malformed containers, no vendor input."""
from pathlib import Path
import binascii,hashlib,json,struct,sys
import numpy as np
P=Path(__file__).resolve().parents[1];sys.path.insert(0,str(P/'tools'))
from container import build,transform
from readback import verify
from package_support import NAMES
tables=[np.array([(i*m+7)%256 for i in range(256)],dtype=np.uint8) for m in [3,5,7]];key=hashlib.sha256(b'synthetic only').digest()[:8]
modules={name:b'synthetic-'+str(i).encode()+b'Ver.05.32.00\0' for i,name in enumerate(NAMES)};modules={n:b+struct.pack('>H',binascii.crc_hqx(b,0)) for n,b in modules.items()};body=bytearray(b' '*32+struct.pack('>II',6,240)+bytes(200));cursor=240
for i,(name,b) in enumerate(modules.items()):body[48+32*i:64+32*i]=name.encode().ljust(16,b'\0');struct.pack_into('>II',body,64+32*i,cursor,len(b));body.extend(b);cursor+=len(b)
body.extend(struct.pack('>H',binascii.crc_hqx(body,0))+bytes(14));plain=hashlib.sha1(body+key).digest()+bytes(12)+body;stock=transform(plain,tables);checks=0
for version in [b'05.32.00',b'05.60.00']:
 payload=b'new authored synthetic payload'+b'Ver.'+version+b'\0';h=hashlib.sha256(payload).hexdigest();encoded,_=build(stock,payload,tables,key,modules);report,out=verify(encoded,tables,key,plain,modules,h,4096,version);assert report['passed'] and out==payload;checks+=1
 for offset in [0,20,100,len(encoded)-1]:
  bad=bytearray(encoded);bad[offset]^=1
  try:verify(bad,tables,key,plain,modules,h,4096,version)
  except ValueError:checks+=1
  else:raise AssertionError('Malformed package accepted')
 try:verify(encoded,tables,key,plain,modules,h,4096,b'05.01.00')
 except ValueError:checks+=1
 else:raise AssertionError('Wrong expected version accepted')
print('SYNTHETIC_PACKAGE_PASS checks='+str(checks))

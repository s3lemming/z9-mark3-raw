"""Explicit local material only; no downloads, camera or installation commands."""
from pathlib import Path
import argparse,hashlib,json
from container import build,transform
from readback import verify
from package_support import read_material,stock_modules
P=Path(__file__).resolve().parents[1];ap=argparse.ArgumentParser();ap.add_argument('operation',choices=['pack','verify']);ap.add_argument('--stock',type=Path,required=True);ap.add_argument('--tables',type=Path,required=True);ap.add_argument('--integrity-material',type=Path,required=True);ap.add_argument('--version',choices=['5.32','5.60'],default='5.32');ap.add_argument('--payload',type=Path);ap.add_argument('--output',type=Path);ap.add_argument('--candidate',type=Path);a=ap.parse_args()
if not __debug__:ap.error('Do not use Python -O')
profile=json.loads((P/'profile.json').read_text());stock=a.stock.read_bytes();assert hashlib.sha256(stock).hexdigest()=='2f13e75d38cd5b65da5b1f21a7828bc8a65ab17dc4c1b435b040ee62d2ed1d81'
tables,key=read_material(a.tables,a.integrity_material);plain=transform(stock,tables);modules=stock_modules(plain,key);identity=profile['versions'][a.version]
if a.operation=='pack':
 assert a.payload and a.output;out=a.output.resolve();assert not out.exists() and not out.is_relative_to(P)
 payload=a.payload.read_bytes();assert hashlib.sha256(payload).hexdigest()==identity['MAIN_SHA256'];encoded,_=build(stock,payload,tables,key,modules)
else:
 assert a.candidate;encoded=a.candidate.read_bytes()
report,extracted=verify(encoded,tables,key,plain,modules,identity['MAIN_SHA256'],0x5820000,('05.'+a.version.split('.')[1]+'.00').encode())
assert hashlib.sha256(encoded).hexdigest()==identity['package_SHA256'] and len(encoded)==101941123
if a.operation=='pack':
 with a.output.open('xb') as f:f.write(encoded)
print(json.dumps(dict(passed=True,version=a.version,package_SHA256=report['package_SHA256'],MAIN_SHA256=report['payload_SHA256'],other_five_modules_unchanged=True,physical_acceptance=False)))

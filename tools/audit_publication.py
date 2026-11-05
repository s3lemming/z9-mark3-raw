"""Fail-closed inventory; only four documented PNGs and one PDF are binary."""
from pathlib import Path
import hashlib,json,re,sys
P=Path(__file__).resolve().parents[1];declared=json.loads((P/'public-files.json').read_text());allowed=set(declared['files'])|{'public-files.json'};actual=set();errors=[];rows=[]
for p in P.rglob('*'):
 rel=p.relative_to(P).as_posix()
 if rel=='.git' or rel.startswith('.git/'):continue
 if p.is_symlink():errors.append([rel,'symlink']);continue
 if p.is_dir():continue
 actual.add(rel)
 if rel not in allowed:errors.append([rel,'undeclared']);continue
 b=p.read_bytes();h=hashlib.sha256(b).hexdigest()
 if rel!='public-files.json' and h!=declared['files'][rel]['SHA256']:errors.append([rel,'changed bytes'])
 if p.suffix=='.png':
  if not rel.startswith('docs/assets/screens/') or not b.startswith(b'\x89PNG\r\n\x1a\n') or len(b)>1000000:errors.append([rel,'unexpected image'])
 elif p.suffix=='.pdf':
  if rel!='docs/v39-user-manual.zh-CN.pdf' or not b.startswith(b'%PDF-') or len(b)>3000000:errors.append([rel,'unexpected PDF'])
 else:
  try:s=b.decode('utf8')
  except UnicodeError:errors.append([rel,'not UTF8']);continue
  if len(b)>150000 or b'\0' in b or b'\r\n' in b or s.startswith('\ufeff'):errors.append([rel,'invalid source encoding/size'])
  for label,pattern in [('private path',r'(?i)(?:(?<![a-z])[C-Z]:[\\/]|/mnt/[c-z]/|/root/[^ /]+/|/home/[^ /]+/)'),('credential',r'(?i)(?:gh[pousr]_[A-Za-z0-9]{20,}|github_pat_[A-Za-z0-9_]{20,}|-----BEGIN (?:RSA |OPENSSH |EC )?PRIVATE KEY-----)')]:
   if re.search(pattern,s):errors.append([rel,label])
 rows.append(dict(path=rel,bytes=len(b),SHA256=h))
for rel in sorted(allowed-actual):errors.append([rel,'missing'])
result=dict(passed=not errors,files=rows,errors=errors);print(json.dumps(result,indent=2));raise SystemExit(0 if result['passed'] else 1)

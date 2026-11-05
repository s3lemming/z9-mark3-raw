"""Synthetic host contracts using actual authored C; no firmware required."""
from pathlib import Path
import argparse,json,subprocess
P=Path(__file__).resolve().parents[1];ap=argparse.ArgumentParser();ap.add_argument('--output',type=Path,required=True);ap.add_argument('--only',choices=['policy','settings','nrmap','precapture','package']);a=ap.parse_args();E=a.output.resolve();assert not E.exists() and not E.is_relative_to(P);E.mkdir(parents=True)
cases=[('policy',['format_policy.c'],[]),('settings',['hsfc_settings_record.c'],[]),('nrmap',['native_virtual_nrmap.c','native_auxiliary_gate.c'],[]),('precapture',['native_precapture_limit.c'],['-DM3_PRECAPTURE_TEST'])]
files=['test_format_policy.c','test_hsfc_settings_record.c','test_native_virtual_nrmap.c','test_precapture_policy.c'];rows=[]
for (name,sources,flags),test in zip(cases,files):
 if a.only and name!=a.only:continue
 out=E/name
 for argv in [['gcc','-std=c11','-O1','-g','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-sanitize-recover=all',*flags,'-I'+str(P/'firmware'),str(P/'tests'/test),*[str(P/'firmware'/s) for s in sources],'-o',str(out)],[str(out)]]:
  q=subprocess.run(argv,capture_output=True,text=True);rows.append(dict(test=name,argv=argv,exit=q.returncode,stdout=q.stdout,stderr=q.stderr));(E/'results.json').write_text(json.dumps(rows,indent=2)+'\n');print(q.stdout+q.stderr,flush=True);assert q.returncode==0,name
if not a.only or a.only=='package':
 q=subprocess.run(['python3','-B',str(P/'tests/package.py')],capture_output=True,text=True);rows.append(dict(test='package',exit=q.returncode,stdout=q.stdout,stderr=q.stderr));(E/'results.json').write_text(json.dumps(rows,indent=2)+'\n');print(q.stdout+q.stderr);assert q.returncode==0
print('All synthetic contracts passed; no camera or numerical image-quality qualification implied.')

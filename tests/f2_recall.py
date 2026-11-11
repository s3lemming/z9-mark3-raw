"""Actual authored recall codec/overlay contracts; no firmware or camera."""
from pathlib import Path
import argparse,subprocess,json
P=Path(__file__).resolve().parents[1];a=argparse.ArgumentParser();a.add_argument('--output',required=True,type=Path);E=a.parse_args().output.resolve();assert not E.exists() and not E.is_relative_to(P);E.mkdir(parents=True);F=P/'variants/f2-recall/firmware';rows=[]
for name,sources in [('field_codec',['recall_field_codec.c']),('value_overlay',['recall_field_codec.c','recall_value_overlay.c'])]:
 for cmd in [['gcc','-std=c11','-O1','-g','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-sanitize-recover=all','-no-pie','-I'+str(F),str(P/'tests'/('test_recall_'+name+'.c')),*[str(F/s)for s in sources],'-o',str(E/name)],[str(E/name)]]:
  q=subprocess.run(cmd,capture_output=True,text=True);rows.append(dict(argv=cmd,exit=q.returncode,stdout=q.stdout,stderr=q.stderr));(E/'results.json').write_text(json.dumps(rows,indent=2));assert q.returncode==0,q.stderr;print(q.stdout)

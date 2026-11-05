"""Render the public manual using a locally licensed CJK TrueType font."""
from pathlib import Path
import argparse,hashlib,json
from xml.sax.saxutils import escape
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.platypus import SimpleDocTemplate,Paragraph,Spacer,PageBreak,Table,TableStyle,Image,KeepTogether
P=Path(__file__).resolve().parents[1];ap=argparse.ArgumentParser();ap.add_argument('--font',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);args=ap.parse_args();out=args.output.resolve();assert not out.is_relative_to(P),'Write authoring output outside repo first';out.parent.mkdir(parents=True,exist_ok=True)
source=json.loads((P/'docs/manual-content.json').read_text(encoding='utf8'));pages=source['pages'];pdfmetrics.registerFont(TTFont('CJK',str(args.font)))
NAVY=colors.HexColor('#14283f');TEAL=colors.HexColor('#177b81');INK=colors.HexColor('#253448');GRAY=colors.HexColor('#647589');PALE=colors.HexColor('#edf4f7')
base=dict(fontName='CJK',wordWrap='CJK',splitLongWords=True,alignment=TA_LEFT)
styles=dict(body=ParagraphStyle('Body',fontSize=10.2,leading=16,textColor=INK,spaceAfter=10,**base),caption=ParagraphStyle('Caption',fontSize=8.2,leading=12,textColor=GRAY,spaceAfter=11,**base),table=ParagraphStyle('Cell',fontSize=9.1,leading=14,textColor=INK,**base),th=ParagraphStyle('HeadCell',fontSize=9.1,leading=14,textColor=colors.white,**base),call=ParagraphStyle('Callout',fontSize=10,leading=16,textColor=NAVY,**base),h=ParagraphStyle('Heading',fontSize=23,leading=31,textColor=NAVY,spaceAfter=8,**base),k=ParagraphStyle('Kicker',fontSize=9,leading=15,textColor=TEAL,spaceAfter=18,**base),cover=ParagraphStyle('Cover',fontSize=37,leading=48,textColor=colors.white,spaceAfter=18,**base),white=ParagraphStyle('White',fontSize=13,leading=21,textColor=colors.white,**base))
def txt(s,style='body'):return Paragraph(escape(s).replace('\n','<br/>'),styles[style])
def header(canvas,doc):
 canvas.saveState();w,h=A4
 if doc.page==1:
  canvas.setFillColor(NAVY);canvas.rect(0,h-350,w,350,fill=1,stroke=0);canvas.setFillColor(TEAL);canvas.rect(45,h-43,45,4,fill=1,stroke=0)
 else:
  canvas.setStrokeColor(colors.HexColor('#d5e0e8'));canvas.line(44,h-36,w-44,h-36);canvas.setFillColor(GRAY);canvas.setFont('CJK',8);canvas.drawString(45,h-28,'Z9 MARK3 / 集成 v39 / 使用手册')
 canvas.setStrokeColor(colors.HexColor('#d5e0e8'));canvas.line(44,40,w-44,40);canvas.setFont('CJK',7.6);canvas.setFillColor(GRAY);canvas.drawString(45,26,'github.com/s3lemming/z9-mark3-raw');canvas.drawRightString(w-45,26,f'{doc.page:02d} / {len(pages):02d}');canvas.restoreState()
story=[]
for i,page in enumerate(pages):
 if i:story.append(PageBreak())
 if page.get('cover'):
  story.extend([Spacer(1,17),txt(page['kicker'],'white'),Spacer(1,17),txt('v39<br/>功能与使用手册'.replace('<br/>','\n'),'cover'),txt(page['blocks'][0]['text'],'white'),Spacer(1,96)])
  blocks=page['blocks'][1:]
 else:story.extend([txt(page['title'],'h'),txt(page['kicker'],'k')]);blocks=page['blocks']
 for b in blocks:
  kind=b['type']
  if kind=='p':story.append(txt(b['text']))
  elif kind=='callout':
   t=Table([[txt(b['text'],'call')]],colWidths=[493]);t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,-1),PALE),('BOX',(0,0),(-1,-1),0.6,colors.HexColor('#d5e6e9')),('LEFTPADDING',(0,0),(-1,-1),12),('RIGHTPADDING',(0,0),(-1,-1),12),('TOPPADDING',(0,0),(-1,-1),10),('BOTTOMPADDING',(0,0),(-1,-1),10)]));story.extend([t,Spacer(1,13)])
  elif kind=='image':
   p=P/b['file'];from PIL import Image as PILImage
   with PILImage.open(p) as im:w,h=im.size
   width=457;height=width*h/w;story.extend([Image(str(p),width=width,height=height),Spacer(1,5),txt(b['caption'],'caption')])
  elif kind=='table':
   rows=[[txt(c,'th' if j==0 else'table') for c in row] for j,row in enumerate(b['rows'])];widths=b.get('widths');widths=[x*493/sum(widths) for x in widths] if widths else None
   t=Table(rows,colWidths=widths,repeatRows=1,hAlign='LEFT');t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),NAVY),('ROWBACKGROUNDS',(0,1),(-1,-1),[colors.white,colors.HexColor('#f3f6f9')]),('VALIGN',(0,0),(-1,-1),'TOP'),('GRID',(0,0),(-1,-1),0.3,colors.HexColor('#d8e2eb')),('LEFTPADDING',(0,0),(-1,-1),8),('RIGHTPADDING',(0,0),(-1,-1),8),('TOPPADDING',(0,0),(-1,-1),7),('BOTTOMPADDING',(0,0),(-1,-1),7)]));story.extend([t,Spacer(1,14)])
doc=SimpleDocTemplate(str(out),pagesize=A4,leftMargin=51,rightMargin=51,topMargin=55,bottomMargin=54,title=source['title'],author='Z9 mark3 project',subject='Integrated v39 user manual with provenance-checked menu screenshots',creator='Z9 mark3 documentation build',pageCompression=1)
doc.build(story,onFirstPage=header,onLaterPages=header)
from pypdf import PdfReader
r=PdfReader(out);assert len(r.pages)==len(pages),(len(r.pages),len(pages))
texts=[p.extract_text() or '' for p in r.pages]
for i,(page,text) in enumerate(zip(pages,texts)):
 assert page['title'] in text or page.get('cover'),(i,page['title'])
 assert len(text)>80,i
assert all(word in '\n'.join(texts) for word in ['C15/C30','Nrmap','0.5','CUS','30,530','5.60'])
audit=dict(passed=True,pages=len(r.pages),PDF_SHA256=hashlib.sha256(out.read_bytes()).hexdigest(),bytes=out.stat().st_size,all_expected_titles_present=True,text_chars=[len(t) for t in texts],screenshots=4,font_embedded=True,visual_render_review_still_required=True)
out.with_suffix('.audit.json').write_text(json.dumps(audit,indent=2)+'\n');print(json.dumps(audit))

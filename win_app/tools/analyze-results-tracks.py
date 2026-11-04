from pathlib import Path
import json,math
base=(Path(__file__).parent/'analyze-results-name-flow.py').read_text();ns={'__file__':str(Path(__file__).parent/'analyze-results-name-flow.py')};exec(compile(base[:base.index('gold=json.loads(')],'font-model','exec'),ns)
def advance(text):return sum(15 if c=='\u265b'else 9.90234375 if c=='\u2248'else ns['f32'](ns['units'][ord(c)]*ns['f32'](.015))for c in text)
def minimum(text):
 words=[];start=0
 for i,c in enumerate(text):
  if c==' ':words.append(text[start:i]);start=i+1
  elif c=='-':words.append(text[start:i+1]);start=i+1
 words.append(text[start:]);return math.ceil(max(advance(x)for x in words)*64)/64
gold=json.loads((ns['pack']/'native-layout-goldens.json').read_text(encoding='utf-8'));cases=[];bad=[]
for g in gold:
 mins=[max([0]+[26+minimum(t)for row in g['lists'][kind]['rows']for t in [row['name'],row['value']]])for kind in ['res-standings','res-coins']]
 # In this bounded profile headings/caption are proved nonbinding separately;
 # do not claim generic min-content for arbitrary other-slot text.
 left,right=594*1.3/2.3,594/2.3
 if mins[0]+mins[1]>594:left,right=mins
 elif mins[0]>left:left,right=mins[0],594-mins[0]
 elif mins[1]>right:left,right=594-mins[1],mins[1]
 left,right=math.floor(left*64)/64,math.floor(right*64)/64
 c=dict(fixture=g['fixture'],scale=g['scale'],offset=g['offset'],minimums=mins,computed=[left,right],observed=g['columnWidths'],matches=[left,right]==g['columnWidths']);cases.append(c)
 if not c['matches']:bad.append(c)
report=dict(observations=len(cases),mismatches=len(bad),cases=cases,examples=bad[:10],scope='Hypothesis for observed source text/profile only; label/caption nonbinding and generic float min-content still require explicit qualification.')
(Path(__file__).parent/'results-tracks-analysis.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps({k:v for k,v in report.items()if k!='cases'},ensure_ascii=True))

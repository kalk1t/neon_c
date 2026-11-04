from pathlib import Path
import json,sys,struct,math
sys.path.insert(0,str(Path(__file__).parent/'fonttools'))
from fontTools.ttLib import TTFont
p=Path(r'C:\Users\User\Projects\neon_c\win_app');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1');font=TTFont(p/'assets/ui-theme/Orbitron-700.ttf');cmap=font.getBestCmap();units={cp:font['hmtx'][g][0] for cp,g in cmap.items()}
def f32(x):return struct.unpack('<f',struct.pack('<f',x))[0]
H=21.296875
def flow(text,width,floats):
 pens=[0]
 for c in text:pens.append(pens[-1]+(15 if c=='\u265b' else 9.90234375 if c=='\u2248' else f32(units[ord(c)]*f32(.015))))
 start=0;y=0;out=[]
 for _ in range(160):
  if start==len(text):return out,y
  active=[f for f in floats if f[1]<y+H and f[1]+f[3]>y];right=min([width]+[f[0] for f in active])
  if pens[-1]-pens[start]<=right:end=len(text);next_start=end
  else:
   choices=[]
   for i in range(start,len(text)):
    if text[i]==' ':end=i;next_start=i+1
    elif text[i]=='-':end=i+1;next_start=i+1
    else:continue
    if end>start and pens[end]-pens[start]<=right:choices.append((end,next_start))
   if choices:end,next_start=choices[-1]
   elif active:
    y=min(f[1]+f[3] for f in active);continue
   else:raise ValueError(('unqualified overflow',text,width))
  out.append((start,end,0,y+1,math.ceil((pens[end]-pens[start])*64)/64,19));start=next_start;y+=H
 raise ValueError('loop')
gold=json.loads((pack/'native-layout-goldens.json').read_text(encoding='utf-8'));cases=[];bad=[];lines=0;cleared=0;multiline=0
for o in gold:
 for name,l in o['lists'].items():
  prior=[]
  for r in l['rows']:
   box=r['box'];floats=[(f['x']-box['x'],f['y']-box['y'],f['width'],f['height']) for f in prior]
   predicted,height=flow(r['name'],box['width'],floats);expected=[(v['x']-box['x'],v['y']-box['y'],v['width'],v['height']) for v in r['nameLineRects']];actual=[x[2:] for x in predicted]
   okay=actual==expected and height==box['height'];lines+=len(predicted);cleared+=bool(predicted and predicted[0][3]>1);multiline+=len(predicted)>1
   item=dict(fixture=o['fixture'],scale=o['scale'],offset=o['offset'],list=name,row=r['ordinal'],text=r['name'],width=box['width'],floats=floats,predicted=predicted,height=height,expected=expected,expected_height=box['height'],matches=okay)
   cases.append(item)
   if not okay:bad.append(item)
   prior.append(r['floatBox'])
report=dict(cases=len(cases),lines=lines,leading_clearance_rows=cleared,multiline_rows=multiline,mismatches=len(bad),examples=bad[:8],inputs='Measured prior-row float boxes only, not current value placement. Name flow outputs compared with independent name lines/li height.')
(Path(__file__).parent/'results-name-flow-analysis.json').write_text(json.dumps(dict(summary=report,cases=cases),indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=True)[:8000])

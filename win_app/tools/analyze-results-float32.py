from pathlib import Path
import sys,json,math,struct
sys.path.insert(0,str(Path(__file__).parent/'fonttools'))
from fontTools.ttLib import TTFont
p=Path(r'C:\Users\User\Projects\neon_c\win_app');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1');font=TTFont(p/'assets/ui-theme/Orbitron-700.ttf');cmap=font.getBestCmap();units={cp:font['hmtx'][g][0] for cp,g in cmap.items()}
def f32(x):return struct.unpack('<f',struct.pack('<f',x))[0]
observations=json.loads((pack/'observations.json').read_text(encoding='utf-8'));report=[]
for accumulate in ['double','float']:
 for scale_first in [False,True]:
  for approx in [9.90625,9.9]:
   nodes=0;errors=0;characters=0;bad_characters=0;examples=[];bad_primary_nodes=0
   for o in observations:
    for name,l in o['lists'].items():
     for row in l['rows']:
      for kind,ns in [('name',row['directTextNodes']),('value',row['valueSpan']['textNodes'])]:
       for node in ns:
        if len(node['rangeClientRectsViewport'])!=1:continue
        text=node['text'];origin=node['rangeClientRectsViewport'][0]['x'];pen=0;bad=[]
        for i,(c,info) in enumerate(zip(text,node['characterRectsViewport'])):
         cp=ord(c);advance=approx if cp==8776 else 15 if cp==9819 else f32(units[cp]*f32(15/1000)) if scale_first else f32(units[cp]*15/1000)
         if cp==8776:advance=f32(advance)
         end=f32(pen+advance) if accumulate=='float' else pen+advance
         if len(info['rects'])==1:
          b=info['rects'][0];left=math.floor(pen*64)/64;right=math.ceil(end*64)/64;actual=[b['x']-origin,b['x']-origin+b['width']];characters+=1
          if abs(left-actual[0])>.001 or abs(right-actual[1])>.001:bad.append(dict(c=c,index=i,pen=pen,end=end,predicted=[left,right],actual=actual));bad_characters+=1
         pen=end
        nodes+=1
        if bad:
         errors+=1;bad_primary_nodes+='\u2248' not in text and '\u265b' not in text
         if len(examples)<2:examples.append(dict(fixture=o['fixture'],text=text,bad=bad[:4]))
   report.append(dict(accumulate=accumulate,scale_first=scale_first,approx=approx,nodes=nodes,bad_nodes=errors,bad_primary_nodes=bad_primary_nodes,characters=characters,bad_characters=bad_characters,examples=examples))
(Path(__file__).parent/'results-float32-analysis.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=True)[:10500])

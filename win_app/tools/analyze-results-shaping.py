from pathlib import Path
import sys,json,math,hashlib,struct
sys.path.insert(0,str(Path(__file__).parent/'fonttools'))
from fontTools.ttLib import TTFont
p=Path(r'C:\Users\User\Projects\neon_c\win_app');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1')
f=TTFont(p/'assets/ui-theme/Orbitron-700.ttf');cmap=f.getBestCmap();advance={cp:f['hmtx'][g][0]*15/f['head'].unitsPerEm for cp,g in cmap.items()};advance.update({9819:15,8776:9.90625})
byglyph={g:cp for cp,g in cmap.items()};kern={}
for lookup in f['GPOS'].table.LookupList.Lookup:
 if lookup.LookupType!=2:continue
 for sub in lookup.SubTable:
  assert sub.Format==1 and sub.ValueFormat1==4 and sub.ValueFormat2==0
  for left,pairs in zip(sub.Coverage.glyphs,sub.PairSet):
   for pair in pairs.PairValueRecord:
    if left in byglyph and pair.SecondGlyph in byglyph:kern[(byglyph[left],byglyph[pair.SecondGlyph])]=pair.Value1.XAdvance*15/1000
observations=json.loads((pack/'observations.json').read_text(encoding='utf-8'));results=[]
for o in observations:
 for listname,l in o['lists'].items():
  for row in l['rows']:
   for kind,nodes in [('name',row['directTextNodes']),('value',row['valueSpan']['textNodes'])]:
    for node in nodes:
     text=node['text'];boxes=node['rangeClientRectsViewport']
     if len(boxes)!=1:continue
     if any(ord(c) not in advance for c in text):raise ValueError(text)
     expected=boxes[0]['width'];plain=sum(advance[ord(c)] for c in text);kvalue=sum(kern.get((ord(a),ord(b)),0) for a,b in zip(text,text[1:]));actual=math.ceil((plain+kvalue)*64)/64
     chars=node['characterRectsViewport'];pen=0;char_errors=[]
     for i,(c,info) in enumerate(zip(text,chars)):
      rects=info['rects'];adj=kern.get((ord(c),ord(text[i+1])),0) if i+1<len(text) else 0
      end=pen+advance[ord(c)]+adj
      if len(rects)==1:
       observed_x=rects[0]['x']-boxes[0]['x'];observed_right=observed_x+rects[0]['width'];left=math.floor(pen*64)/64;right=math.ceil(end*64)/64
       if abs(observed_x-left)>.001 or abs(observed_right-right)>.001:char_errors.append(dict(i=i,char=c,pen=pen,end=end,predicted=[left,right],observed=[observed_x,observed_right]))
      pen=end
     results.append(dict(fixture=o['fixture'],scale=o['scale'],offset=o['offset'],list=listname,row=row['index'],kind=kind,text=text,observed=expected,plain=plain,kern=kvalue,predicted=actual,error=actual-expected,char_errors=char_errors))
bad=[r for r in results if abs(r['error'])>.001];badchars=[r for r in results if r['char_errors']]
report=dict(nodes=len(results),width_mismatches=len(bad),character_mismatch_nodes=len(badchars),nonzero_kern_nodes=sum(r['kern']!=0 for r in results),worst_width=max(abs(r['error']) for r in results),examples=bad[:8],char_examples=badchars[:5])
(Path(__file__).parent/'results-shaping-analysis.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=True)[:6500])

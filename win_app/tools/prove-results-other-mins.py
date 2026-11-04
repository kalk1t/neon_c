from pathlib import Path
import json,hashlib,sys,math
sys.path.insert(0,str(Path(__file__).parent/'fonttools'))
from fontTools.ttLib import TTFont
p=Path(r'C:\Users\User\Projects\neon_c\win_app');primary=p/'assets/ui-theme/Orbitron-700.ttf';fallback=Path(r'C:\Windows\Fonts\ariblk.ttf');font=TTFont(primary);fb=TTFont(fallback);cmap=font.getBestCmap();fcmap=fb.getBestCmap();pairs={}
used_glyphs={cmap[ord(c)]for c in 'STANDINGSCOINRACEBONUS+0123456789stndrh '}
for lookup in font['GPOS'].table.LookupList.Lookup:
 if lookup.LookupType==4:
  for sub in lookup.SubTable:assert not used_glyphs.intersection(sub.MarkCoverage.glyphs)
  continue
 assert lookup.LookupType==2
 for sub in lookup.SubTable:
  assert sub.Format==1 and sub.ValueFormat1==4 and sub.ValueFormat2==0
  for left,ps in zip(sub.Coverage.glyphs,sub.PairSet):
   for pair in ps.PairValueRecord:pairs[left,pair.SecondGlyph]=pairs.get((left,pair.SecondGlyph),0)+pair.Value1.XAdvance
assert not font['GSUB'].table.FeatureList.FeatureCount
def bound(text,size,tracking):
 width=0
 for c in text:
  if ord(c)in cmap:width+=font['hmtx'][cmap[ord(c)]][0]*size/font['head'].unitsPerEm
  else:
   assert c=='\u00b7';width+=fb['hmtx'][fcmap[ord(c)]][0]*size/fb['head'].unitsPerEm
 for a,b in zip(text,text[1:]):width+=max(0,pairs.get((cmap.get(ord(a)),cmap.get(ord(b))),0))*size/font['head'].unitsPerEm
 # Full max-content including all positive kern and a generous unit of layout
 # rounding per character is an upper bound on min-content for these runs.
 return math.ceil((width+len(text)*tracking+len(text)/64)*64)/64
def ordinal(n):return str(n)+('th'if 10<n%100<14 else {1:'st',2:'nd',3:'rd'}.get(n%10,'th'))
labels=[dict(text=t,bound=bound(t,15,2))for t in ['STANDINGS','COIN RACE']];captions=[]
for count in range(1,12):
 for rank in range(1,count+1):
  bonus={1:15,2:8,3:4}.get(rank,0)if count>1 else 0;text='COIN RACE '+ordinal(rank)+(f' \u00b7 BONUS +{bonus}'if bonus else '');captions.append(dict(count=count,rank=rank,text=text,bound=bound(text,12,1)))
mx=max(x['bound']for x in captions+labels);assert mx<258.25
matching=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3\recipes\font-matching-v3.json');assert json.loads(matching.read_text(encoding='utf-8'))['weight600Uses700']
report=dict(fonts=[dict(path=str(f),sha256=hashlib.sha256(f.read_bytes()).hexdigest())for f in [primary,fallback]],weight600_face_evidence=dict(path=str(matching),sha256=hashlib.sha256(matching.read_bytes()).hexdigest()),labels=labels,captions=captions,all_canonical_roster_rank_combinations=len(captions),maximum_full_text_bound=mx,nonbinding_min_content_upper_bound=258.25,scope='Numeric font metadata only, no fallback binary/outline copied or runtime-loaded. Conservative full-text widths for current fixed labels and canonical count/rank captions; no exact text width, height/raster, arbitrary caption or account-state qualification.')
(Path(__file__).parent/'results-other-mins-proof.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(labels=labels,combinations=len(captions),maximum_bound=mx,limit=258.25)))

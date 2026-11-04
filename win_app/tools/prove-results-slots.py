from pathlib import Path
import json,sys,hashlib,math
sys.path.insert(0,str(Path(__file__).parent/'fonttools'))
from fontTools.ttLib import TTFont
p=Path(r'C:\Users\User\Projects\neon_c\win_app');faces={}
for weight in [700,900]:
 f=p/f'assets/ui-theme/Orbitron-{weight}.ttf';t=TTFont(f);pairs=[]
 for lookup in t['GPOS'].table.LookupList.Lookup:
  if lookup.LookupType==2:
   for sub in lookup.SubTable:
    for ps in sub.PairSet:
     for pair in ps.PairValueRecord:pairs.append(pair.Value1.XAdvance)
  else:assert lookup.LookupType==4
 faces[weight]=dict(path=str(f),sha256=hashlib.sha256(f.read_bytes()).hexdigest(),em=t['head'].unitsPerEm,ascent=t['hhea'].ascent,descent=-t['hhea'].descent,cmap=t.getBestCmap(),units=t['hmtx'].metrics,max_positive=max([0]+pairs))
fb=TTFont(r'C:\Windows\Fonts\ariblk.ttf');fc=fb.getBestCmap();dot=fb['hmtx'][fc[183]][0]/fb['head'].unitsPerEm
def upper(text,size,weight):
 f=faces[weight];return math.ceil((sum((dot if c=='\u00b7'else f['units'][f['cmap'][ord(c)]][0]/f['em'])*size for c in text)+max(0,len(text)-1)*f['max_positive']*size/f['em']+len(text)/64)*64)/64
def ordinal(n):return str(n)+('th'if n%100 in [11,12,13]else {1:'st',2:'nd',3:'rd'}.get(n%10,'th'))
places=[dict(count=n,position=i,kills=k,text=f'{ordinal(i)} OF {n} \u00b7 {k} TAKEDOWN'+(''if k==1 else 'S'))for n in range(2,12)for i in range(1,n+1)for k in range(10)]
place_bound=max(upper(c['text'],24,900)for c in places)
# Conservative digit-metric bounds, not sampling rounding-sensitive clock times.
def clock_bound(size,weight,minute_digits):
 f=faces[weight];digit=max(f['units'][f['cmap'][cp]][0]for cp in range(48,58));punct=sum(f['units'][f['cmap'][cp]][0]for cp in [58,46]);count=minute_digits+6
 return ((minute_digits+4)*digit+punct+count*f['max_positive'])*size/f['em']+count/64
total_bound=clock_bound(32,900,3);lap_bound=upper('LAP 2 \u2014 ',16,700)+clock_bound(16,700,2);group_bound=2*lap_bound+28
assert max(place_bound,total_bound,group_bound)<616
report=dict(fonts=[{k:f[k]for k in ['path','sha256','em','ascent','descent','max_positive']}for f in faces.values()],profile='Exactly two finite laps0..3600 seconds, count1..11, valid finish/coin ranks, takedowns0..9, empty online block and hidden unlock/next. Other states need a separate profile, never truncation.',bounds=dict(place=place_bound,total=total_bound,two_lap_group=group_bound,limit=616),place_combinations=len(places),scope='Offline numeric font bounds for unwrapped source slot profile, not browser/native pixel or arbitrary account/wrapped-text acceptance.')
(Path(__file__).parent/'results-slots-proof.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(bounds=report['bounds'],place_combinations=len(places))))

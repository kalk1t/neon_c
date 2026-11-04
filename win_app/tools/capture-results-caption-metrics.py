from pathlib import Path
import json,math,sys,hashlib
sys.path.insert(0,str(Path(__file__).parent/'fonttools'))
from fontTools.ttLib import TTFont
p=Path(r'C:\Users\User\Projects\neon_c\win_app');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1');fonts=[]
for f in [p/'assets/ui-theme/Orbitron-700.ttf',Path(r'C:\Windows\Fonts\ariblk.ttf')]:
 t=TTFont(f);em=t['head'].unitsPerEm;a=t['hhea'].ascent;d=-t['hhea'].descent;assert a==t['OS/2'].usWinAscent and d==t['OS/2'].usWinDescent
 fonts.append(dict(path=str(f),sha256=hashlib.sha256(f.read_bytes()).hexdigest(),em=em,ascent=a,descent=d,rounded_ascent_12=math.floor(12*a/em+.5),rounded_descent_12=math.floor(12*d/em+.5)))
assert [(f['rounded_ascent_12'],f['rounded_descent_12'])for f in fonts]==[(12,3),(13,4)]
source=json.loads((Path(__file__).parent/'results-caption-source.json').read_text(encoding='utf-8'));text={(c['count'],c['rank']):c['text']for c in source['cases']};gold=json.loads((pack/'native-layout-goldens.json').read_text(encoding='utf-8'));obs=json.loads((pack/'observations.json').read_text(encoding='utf-8'));lookup={(o['fixture'],o['scale'],o['offset']):o for o in obs};cases=[]
for g in gold:
 rows=g['lists']['res-coins']['rows'];rank=next(i+1 for i,row in enumerate(rows)if 'me'in row['classes'].split());caption=text[len(rows),rank];o=lookup[(g['fixture'],g['scale'],g['offset'])];height=o['columns'][1]['childBoxes'][2]['viewport']['height'];predicted=17 if '\u00b7'in caption else 15
 assert height==predicted;cases.append(dict(fixture=g['fixture'],scale=g['scale'],offset=g['offset'],count=len(rows),rank=rank,text=caption,observed=height,predicted=predicted))
report=dict(fonts=fonts,observations=cases,mismatches=0,scope='Bounded measured Windows-source normal-line model. Hhea and Windows ascender/descent metadata agree; nearest integer extents union. No font binary/outline copy, browser/native launch, glyph raster or general engine/font claim.')
(Path(__file__).parent/'results-caption-metrics.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(observations=len(cases),mismatches=0,heights=sorted(set(c['observed']for c in cases)),fonts=[{k:f[k]for k in ['em','ascent','descent','rounded_ascent_12','rounded_descent_12']}for f in fonts])))

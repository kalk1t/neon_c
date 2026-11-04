from pathlib import Path
import json,sys,math,struct
sys.path.insert(0,str(Path(__file__).parent/'fonttools'))
from fontTools.ttLib import TTFont
p=Path(r'C:\Users\User\Projects\neon_c\win_app');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1');font=TTFont(p/'assets/ui-theme/Orbitron-700.ttf');cmap=font.getBestCmap();units={cp:font['hmtx'][g][0] for cp,g in cmap.items()}
def f32(x):return struct.unpack('<f',struct.pack('<f',x))[0]
def measure(text):return sum(15 if c=='\u265b' else 9.90234375 if c=='\u2248' else f32(units[ord(c)]*f32(.015)) for c in text)
threshold=json.loads((pack/'threshold-search.json').read_text(encoding='utf-8'));out=[]
for column in threshold:
 for kind in ['fit','over','exactFit']:
  x=column[kind];raw=measure(x['name']);out.append(dict(column=column['column'],kind=kind,name=x['name'],raw=raw,floor=math.floor(raw*64)/64,nearest=math.floor(raw*64+.5)/64,ceil=math.ceil(raw*64)/64,float_width=x['floatWidth'],available=x['liWidth'],source_relocated=x['floatRelocated']))
print(json.dumps(out,ensure_ascii=True))

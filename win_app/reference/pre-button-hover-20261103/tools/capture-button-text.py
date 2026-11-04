from pathlib import Path
import json,hashlib,struct,re,sys
p=Path(r'C:\Users\User\Projects\neon_c');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3');out=Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
manifest=json.loads((pack/'manifest.json').read_text());assert sha(pack/'manifest.json')=='e8eca0c29f6f392ca1a81ef602663de6438847db12306594011bb4677dfd0443';names=['assets.json','source/style.css','fonts/Orbitron-700.ttf']
for name in names:assert sha(pack/name)==next(a['sha256'] for a in manifest['assets'] if a['file']==name)
cssfile=p/'game/src/style.css';css=cssfile.read_text(encoding='utf-8');old=(pack/'source/style.css').read_text(encoding='utf-8');rules=[]
for selector in ['button {','button:hover, button:focus {','button:disabled {','body:not(.touch) #menu .menu-card > button {']:
 a=css.index(selector);rule=css[a:css.index('}',a)+1];assert rule in old;rules.append(rule)
a=css.index('@media (max-height: 820px)');b=css.index('{',a)+1;depth=1
while depth:
 if css[b]=='{':depth+=1
 elif css[b]=='}':depth-=1
 b+=1
media=css[a:b];assert media in old and 'font-size: 18px' in media

assets=json.loads((pack/'assets.json').read_text());states=[]
for state in ['default','hover','focus','pressed','disabled','disabled-hover']:
 v=next(v for v in assets if v['id']=='pause/buttons/resume/'+state and v['scale']==1);s=v['observed']['styles'];padding=s['padding'].split();left=float(padding[-1].removesuffix('px'));rgba=list(map(int,re.findall(r'\d+',s['color'])))+[round(float(s['opacity'])*255)];hover=state in ['hover','focus','pressed','disabled-hover'];enabled='disabled' not in state
 states.append('{'+','.join(map(str,[float(s['fontSize'].removesuffix('px')),float(s['letterSpacing'].removesuffix('px')),left+2,int(hover),int(enabled)]+rgba))+'}')
f=p/'win_app/assets/ui-theme/Orbitron-700.ttf';assert sha(f)==sha(pack/'fonts/Orbitron-700.ttf')
b=f.read_bytes();n=struct.unpack_from('>H',b,4)[0];tables={b[12+i*16:16+i*16].decode():struct.unpack_from('>I',b,20+i*16)[0] for i in range(n)};units=struct.unpack_from('>H',b,tables['head']+18)[0];asc,desc=struct.unpack_from('>hh',b,tables['hhea']+4);metrics=struct.unpack_from('>H',b,tables['hhea']+34)[0]
cm=tables['cmap'];num=struct.unpack_from('>H',b,cm+2)[0];subs=[cm+struct.unpack_from('>I',b,cm+8+8*i)[0] for i in range(num)];sub=next(s for s in subs if struct.unpack_from('>H',b,s)[0]==4);seg=struct.unpack_from('>H',b,sub+6)[0]//2
def glyph(cp):
 for k in range(seg):
  end=struct.unpack_from('>H',b,sub+14+2*k)[0];start=struct.unpack_from('>H',b,sub+16+2*seg+2*k)[0]
  if start<=cp<=end:
   delta=struct.unpack_from('>h',b,sub+16+4*seg+2*k)[0];word=sub+16+6*seg+2*k;off=struct.unpack_from('>H',b,word)[0]
   if not off:return (cp+delta)&65535
   g=struct.unpack_from('>H',b,word+off+2*(cp-start))[0];return (g+delta)&65535 if g else 0
 raise ValueError(cp)
def advance(text):return sum(struct.unpack_from('>H',b,tables['hmtx']+4*min(glyph(ord(c)),metrics-1))[0] for c in text)/units


labels=['RACE / OFFLINE TRIAL','TIME ATTACK / SOLO','FREE ROAM / ONLINE','WORLD RANKING','SHOP','QUIT','BACK','RESUME','RESTART','MAIN MENU','RACE AGAIN [ENTER]'];widths=[];ideal_widths=[]
for label in labels:
 for size in [18,20]:
  expected=advance(label)*size+(len(label)-1)*3;encoded=json.dumps(label);widths.append('{'+','.join([encoded,str(size),str(expected)])+'}');ideal_widths.append(dict(label=label,css_size=size,width_without_trailing_tracking=expected))
(out/'button_text_fixture.h').write_text('/* Six reused Chromium computed button styles; independent hmtx widths, not pixel baselines. */\nstatic const ButtonStyleCase style_cases[]={'+','.join(states)+'};\nstatic const ButtonWidthCase width_cases[]={\n'+',\n'.join(widths)+'\n};\n')
receipt=dict(version=json.loads((p/'game/package.json').read_text())['version'],source=dict(path=str(cssfile),sha256=sha(cssfile)),package=str(pack),manifest_sha256=sha(pack/'manifest.json'),verified_files=[dict(path=str(pack/name),sha256=sha(pack/name)) for name in names]+[dict(path=str(f),sha256=sha(f))],rules=rules,compact_media=media,reused_computed_styles=6,independent_width_cases=len(widths),font_metrics=dict(units=units,ascent=asc,descent=desc),widths=ideal_widths,scope='Six prepared default/hover/focus/pressed/disabled style endpoints.18px compact menu derives from identical current media CSS, not fresh browser measurement. Independent TTF em advances omit kerning. Nominal hhea baseline is not a measured Chromium glyph baseline. No graphics or event engine run.')
(out/'button-text-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(dict(styles=6,width_cases=len(widths),font_metrics=receipt['font_metrics'])))

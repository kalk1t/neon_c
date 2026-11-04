from pathlib import Path
import json,hashlib,struct,sys
p=Path(r'C:\Users\User\Projects\neon_c');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v2');p3=pack.parent/'RoadstersSharedAssets-v3';out=Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
m=json.loads((pack/'manifest.json').read_text(encoding='utf-8'));m3=json.loads((p3/'manifest.json').read_text(encoding='utf-8'));assert sha(p3/'manifest.json')=='e8eca0c29f6f392ca1a81ef602663de6438847db12306594011bb4677dfd0443';verified=[]
for directory,manifest,names in [(pack,m,['assets.json','source/style.css']),(p3,m3,['fonts/Rajdhani-500.ttf','fonts/Orbitron-700.ttf'])]:
 for name in names:
  f=directory/name;assert sha(f)==next(a['sha256'] for a in manifest['assets'] if a['file']==name);verified.append(dict(path=str(f),sha256=sha(f)))
cssfile=p/'game/src/style.css';css=cssfile.read_text(encoding='utf-8');old=(pack/'source/style.css').read_text(encoding='utf-8');rules=[]
for selector in ['.controls {','kbd {']:
 a=css.index('\n'+selector)+1;assert a>0;rule=css[a:css.index('}',a)+1];assert rule in old;rules.append(rule)
a=css.index('@media (max-height: 820px)');b=css.index('{',a)+1;depth=1
while depth:
 if css[b]=='{':depth+=1
 elif css[b]=='}':depth-=1
 b+=1
media=css[a:b];assert media in old and 'line-height: 1.6' in media
assets=json.loads((pack/'assets.json').read_text(encoding='utf-8'));labels=['MOUSE','RMB','W','S','SHIFT','F','C','R','ESC','A','D'];keys=[];observed=[]
for label in labels:
 v=next(v for v in assets if v['kind']=='keycap' and v['scale']==1 and v['observed']['text']==label);s=v['observed']['styles'];r=v['observed']['rect'];assert s['fontSize']=='12px' and s['fontWeight']=='700' and s['borderWidth']=='1px' and s['borderRadius']=='3px' and s['padding']=='2px 6px' and s['letterSpacing']=='normal';keys.append('{'+','.join([json.dumps(label),str(r['width']),str(r['height'])])+'}');observed.append(dict(label=label,rect=r,styles=s))
def font_advance(f):
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
 
 
 return advance

texts=[' steer + climb/dive ',' hold to look around',' thrust ',' brake ',' boost ',' spin dodge ',' camera ',' reset ',' pause',' slide sideways'];widths=[];advances=[]
for face,file,size in [(0,'Rajdhani-Medium.ttf',15),(1,'Orbitron-700.ttf',12)]:
 f=p/'win_app/assets/ui-theme'/file;assert sha(f)==sha(p3/'fonts'/('Rajdhani-500.ttf' if face==0 else file));verified.append(dict(path=str(f),sha256=sha(f)));advance=font_advance(f)
 for text in texts if face==0 else labels:widths.append('{'+','.join([str(face),json.dumps(text),str(size),str(advance(text)*size)])+'}');advances.append(dict(face=face,text=text,width=advance(text)*size))
keywidths={v['label']:v['rect']['width'] for v in observed};textwidths={v['text']:v['width'] for v in advances if v['face']==0};lines=[[('MOUSE',1),(texts[0],0),('RMB',1),(texts[1],0)],[('W',1),(texts[2],0),('S',1),(texts[3],0),('SHIFT',1),(texts[4],0),('A',1),('D',1),(texts[9],0)],[('F',1),(texts[5],0),('C',1),(texts[6],0),('R',1),(texts[7],0),('ESC',1),(texts[8],0)]];rowwidths=[sum(keywidths[t]+4 if k else textwidths[t] for t,k in row) for row in lines];assert max(rowwidths)<470
(out/'menu_help_fixture.h').write_text('/* Existing browser keycap boxes/styles and independent TTF widths for supported native help subset. */\nstatic const KeySource key_sources[]={'+','.join(keys)+'};\nstatic const HelpWidthCase help_widths[]={\n'+',\n'.join(widths)+'\n};\nstatic const double ideal_row_widths[]={'+','.join(map(str,rowwidths))+'};\n')
receipt=dict(version=json.loads((p/'game/package.json').read_text(encoding='utf-8'))['version'],source=dict(path=str(cssfile),sha256=sha(cssfile)),package=str(pack),manifest_sha256=sha(pack/'manifest.json'),font_package=str(p3),font_manifest_sha256=sha(p3/'manifest.json'),verified_files=verified,rules=rules,compact_media=media,keys=observed,width_cases=len(widths),native_rows=lines,ideal_row_widths=rowwidths,scope='Nine supplied browser keycap boxes/computed styles; existing fonts with independent hmtx widths. Supported native help subset reflows into3 rows; source combat/cockpit/chat/mute controls excluded until integrated. Nominal shared baseline from hhea, not browser pixel baseline. No PNG/GPU/raster or engine event measurement.')
(out/'menu-help-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(dict(keys=9,width_cases=len(widths),ideal_row_widths=rowwidths)))

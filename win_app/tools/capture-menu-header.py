from pathlib import Path
import json,hashlib,struct,sys,subprocess
p=Path(r'C:\Users\User\Projects\neon_c');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v2');p3=pack.parent/'RoadstersSharedAssets-v3';out=Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
m=json.loads((pack/'manifest.json').read_text(encoding='utf-8'));m3=json.loads((p3/'manifest.json').read_text(encoding='utf-8'));assert sha(p3/'manifest.json')=='e8eca0c29f6f392ca1a81ef602663de6438847db12306594011bb4677dfd0443';verified=[]
for directory,manifest,names in [(pack,m,['recipes/measured-layouts.json','source/style.css']),(p3,m3,['fonts/Orbitron-900.ttf','fonts/Orbitron-700.ttf'])]:
 for name in names:
  f=directory/name;assert sha(f)==next(a['sha256'] for a in manifest['assets'] if a['file']==name);verified.append(dict(path=str(f),sha256=sha(f)))
cssfile=p/'game/src/style.css';css=cssfile.read_text(encoding='utf-8');old=(pack/'source/style.css').read_text(encoding='utf-8');rules=[]
for selector in ['.menu-card {','.brand {','.brand span {','.track-name {','.best-line {','.best-line b {']:
 a=css.index('\n'+selector)+1;assert a>0;rule=css[a:css.index('}',a)+1];assert rule in old;rules.append(rule)
a=css.index('@media (max-height: 820px)');b=css.index('{',a)+1;depth=1
while depth:
 if css[b]=='{':depth+=1
 elif css[b]=='}':depth-=1
 b+=1
media=css[a:b];assert media in old
layouts=json.loads((pack/'recipes/measured-layouts.json').read_text(encoding='utf-8'));cases=[];summaries=[]
for name in ['desktop-menu','compact-menu']:
 v=next(v for v in layouts if v['name']==name);card=v['observed']['.menu-card'];brand=card['children'][0];track=card['children'][1];buttons=[c for c in card['children'] if c['tag']=='BUTTON'];assert len(buttons)==5
 values=[v['viewport']['width'],v['viewport']['height'],float(brand['styles']['fontSize'].removesuffix('px')),brand['relativeRect']['height']/2,track['relativeRect']['y'],track['relativeRect']['height']];rects=['{'+','.join(str(c['relativeRect'][k]) for k in ['x','y','width','height'])+'}' for c in buttons];cases.append('{'+','.join(map(str,values))+',{'+','.join(rects)+'}}');summaries.append(dict(name=name,card_rect=card['rect'],brand=brand['relativeRect'],track=track['relativeRect'],buttons=[c['relativeRect'] for c in buttons]))
node=p/'win_app/third_party/emsdk-6.0.13/emsdk-35659964bc2762fffec0d257997fb61475f765b7/node/24.19.0_64bit/node.exe';js="const {ROADS}=await import('file:///C:/Users/User/Projects/neon_c/game/src/roads.js');console.log(JSON.stringify(ROADS.map(r=>({id:r.id,name:r.name,gates:r.rings||r.checkpoints.length}))));";roads=json.loads(subprocess.run([str(node),'--input-type=module','-e',js],capture_output=True,text=True,check=True,encoding='utf-8').stdout)
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
widths=[]
for face,file in enumerate(['Orbitron-900.ttf','Orbitron-700.ttf']):
 f=p/'win_app/assets/ui-theme'/file;assert sha(f)==sha(p3/'fonts'/file);verified.append(dict(path=str(f),sha256=sha(f)));advance=font_advance(f)
 values=[(t,size,4) for t in ['ROADSTERS','RACE'] for size in [46,64]]+[('--:--.--',14,2)] if face==0 else [(f"ROAD {r['id']} / {r['name']} / {r['gates']} GATES",13,3) for r in roads]+[('BEST LAP ',14,2)]
 for text,size,spacing in values:widths.append('{'+','.join([str(face),json.dumps(text),str(size),str(spacing),repr(advance(text)*size+(len(text)-1)*spacing)])+'}')
(out/'menu_header_fixture.h').write_text('/* Verified existing desktop/compact browser relative header/button boxes; independent TTF widths. */\nstatic const MenuSource menu_sources[]={'+','.join(cases)+'};\nstatic const MenuWidthCase menu_widths[]={\n'+',\n'.join(widths)+'\n};\n')
receipt=dict(version=json.loads((p/'game/package.json').read_text(encoding='utf-8'))['version'],source=dict(path=str(cssfile),sha256=sha(cssfile)),roads=dict(path=str(p/'game/src/roads.js'),sha256=sha(p/'game/src/roads.js')),package=str(pack),manifest_sha256=sha(pack/'manifest.json'),font_package=str(p3),font_manifest_sha256=sha(p3/'manifest.json'),verified_files=verified,rules=rules,compact_media=media,reused_relative_layouts=summaries,width_cases=len(widths),scope='Prepared source header/track/button relative geometry, selected current CSS identical; native ASCII offline labels retain semantics and use source outer box heights. Whole browser card/paint/account/settings/footer not ported; native global centering uses its own shorter content. Independent TTF advances omit kerning. Native glyph baseline/glow/raster remain unqualified.')
(out/'menu-header-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(dict(boxes=2,independent_widths=len(widths))))

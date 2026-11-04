from pathlib import Path
import struct,json,hashlib,re
p=Path(r'C:\Users\User\Projects\neon_c');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3');out=Path(__file__).parent/'spin-readout-fixture';out.mkdir(exist_ok=True)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
assert sha(pack/'manifest.json')=='e8eca0c29f6f392ca1a81ef602663de6438847db12306594011bb4677dfd0443';m=json.loads((pack/'manifest.json').read_text());names=['assets.json','source/style.css','recipes/measured-layouts.json','fonts/Orbitron-700.ttf']
for name in names:assert sha(pack/name)==next(a['sha256'] for a in m['assets'] if a['file']==name),name
current=(p/'game/src/style.css').read_text(encoding='utf-8');prior=(pack/'source/style.css').read_text(encoding='utf-8');rules=[]
for selector in ['.wpn {','.wpn .key {','.attack .ring.emp {','.attack .ring.emp b {','.attack .ring.spin b {','.attack .ring {']:
 a=current.index(selector);rule=current[a:current.index('}',a)+1];assert rule in prior;rules.append(rule)
html=(p/'game/index.html').read_text(encoding='utf-8');text=re.search(r'id="spin-ring".*?<div class="key">([^<]+)</div>',html,re.S)[1];assert text=='DODGE · F'
layouts=json.loads((pack/'recipes/measured-layouts.json').read_text());boxes=[]
for name,z in [('desktop-race-hud',1),('compact-race-hud',.8)]:
 v=next(v for v in layouts if v['name']==name);panel=v['observed']['.panel.attack'];group=panel['children'][-1]['rect'];ring=74 if z==1 else 59.1875;gap=6 if z==1 else 4.796875
 r=panel['rect'];cx=r['x']+group['x']+group['width']/2;cy=r['y']+group['y']+ring/2;key_top=r['y']+group['y']+ring+gap;key_h=group['height']-ring-gap
 boxes.append('{'+','.join(map(str,[v['viewport']['width'],v['viewport']['height'],z,r['x']+r['width'],r['y'],r['height'],cx,cy,key_top,key_h]))+'}')
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

# Existing browser26px fallback advance, retained from verified native handoff.
data=p/'win_app/src/presentation/rr_subtext_data.h';native=data.read_text();assert '8.671875000f' in native
widths=[]
for value in [text,'F','DODGE F','DODGE · F · F']:
 for size in [7.2,9,14,26]:
  for spacing in [0,1.2,1.5,3]:
   dot=value.count('·');ascii_text=value.replace('·','');ideal=advance(ascii_text)*size+dot*8.671875*size/26+len(value)*spacing
   encoded='"'+''.join(chr(b) if 32<=b<127 and b not in [34,92] else '\\'+format(b,'03o') for b in value.encode())+'"'
   widths.append('{'+','.join([encoded,str(size),str(spacing),repr(ideal),str(len(ascii_text)),str(len(value))])+'}')
(out/'spin_readout_fixture.h').write_text('/* Verified source group boxes; independent sfnt advances + retained26px fallback advance, not9px browser raster metrics. */\nstatic const SpinBoxCase box_cases[]={'+','.join(boxes)+'};\nstatic const WidthCase width_cases[]={\n'+',\n'.join(widths)+'\n};\nstatic const char source_key[]="DODGE \\302\\267 F";\n')
receipt={'version':json.loads((p/'game/package.json').read_text())['version'],'package':str(pack),'manifest_sha256':sha(pack/'manifest.json'),'verified_files':[{'path':str(pack/n),'sha256':sha(pack/n)} for n in names],'sources':[{'path':str(p/'game/src/style.css'),'sha256':sha(p/'game/src/style.css')},{'path':str(p/'game/index.html'),'sha256':sha(p/'game/index.html')},{'path':str(f),'sha256':sha(f)},{'path':str(data),'sha256':sha(data)}],'rules':rules,'source_key':text,'reused_layout_states':2,'width_cases':len(widths),'scope':'Prepared full weapon-panel SPIN group offsets/right inset and line heights, current relevant CSS/HTML unchanged. Independent hmtx ASCII widths plus existing browser26px middot advance scaled to caption size. Not fresh9px fallback hinting, glyph baselines, GPU group-opacity/glow or full weapons-panel acceptance.'}
(out/'spin-readout-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps({'boxes':2,'width_cases':len(widths),'key':text}))

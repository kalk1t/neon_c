from pathlib import Path
import struct,json,hashlib,math
o=Path(__file__).parent/'hud-readouts-fixture';o.mkdir(exist_ok=True)
p=Path(r'C:\Users\User\Projects\neon_c');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3')
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
m=json.loads((pack/'manifest.json').read_text());assert sha(pack/'manifest.json')=='e8eca0c29f6f392ca1a81ef602663de6438847db12306594011bb4677dfd0443'
names=['assets.json','source/style.css','recipes/measured-layouts.json','recipes/font-matching.json']
for name in names:assert sha(pack/name)==next(a['sha256'] for a in m['assets'] if a['file']==name),name
current=(p/'game/src/style.css').read_text(encoding='utf-8');prior=(pack/'source/style.css').read_text(encoding='utf-8');selectors=['  --font:','* {','.panel {','.lbl {','#bar {','.speed {','#speed {','.unit {','.lap {','.lap .row {','.lap .row b {','.lap .cp b {','#time {','@media (max-width: 1180px), (max-height: 720px) {']
rules=[]
for prefix in selectors:
 def selected(s):
  if '{' in prefix:
   start=s.index(prefix);return s[start:s.index('}',start)+1]
  return next(l for l in s.splitlines() if l.startswith(prefix))
 a=selected(current);b=selected(prior);assert a==b,prefix;rules.append(a)
assets=json.loads((pack/'assets.json').read_text());speed=next(a for a in assets if a['id']=='hud/speed-panel/normal' and a['scale']==1);lap=next(a for a in assets if a['id']=='hud/lap-panel/race' and a['scale']==1)
assert speed['observed']['rect']['height']==201 and lap['observed']['rect']['height']==153
children=lap['observed']['children']+speed['observed']['children'][:3]
for c in children:assert c['rect']['x']==24
layouts=json.loads((pack/'recipes/measured-layouts.json').read_text());boxes=[]
for item in layouts:
 v=item['viewport'];obs=item.get('observed',{})
 if v.get('touch') or '.panel.lap' not in obs or '.panel.speed' not in obs:continue
 a=obs['.panel.lap']['rect'];b=obs['.panel.speed']['rect']
 if a['width']<=0 or b['width']<=0:continue
 children=obs['.panel.lap']['children'][:3]+obs['.panel.speed']['children'][:3]
 boxes.append('{'+str(v['width'])+','+str(v['height'])+',{'+','.join(str(a[k]) for k in ['x','y','width','height'])+'},{'+','.join(str(b[k]) for k in ['x','y','width','height'])+'},{'+','.join('{'+','.join(str(c['rect'][k]) for k in ['x','y','width','height'])+'}' for c in children)+'}}')
assert boxes
fm=json.loads((pack/'recipes/font-matching.json').read_text());sample=fm['sample'];faces=[('Orbitron-500.ttf','Orbitron-500.ttf',500,64,'Orbitron','500'),('Orbitron-700.ttf','Orbitron-700.ttf',700,64,'Orbitron','700'),('Orbitron-900.ttf','Orbitron-900.ttf',900,64,'Orbitron','900'),('Rajdhani-Medium.ttf','Rajdhani-500.ttf',0,32,'Rajdhani','500')]
face_rows=[];widths=[];fonts=[]
for index,(native,source,weight,base,family,fw) in enumerate(faces):
 f=p/'win_app/assets/ui-theme'/native;assert sha(f)==sha(pack/'fonts'/source);assert sha(pack/'fonts'/source)==next(a['sha256'] for a in m['assets'] if a['file']=='fonts/'+source)
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
 chrome=fm['widths'][family][fw];face_rows.append('{'+','.join([json.dumps(native),str(weight),str(base),str(units),str(asc),str(desc)])+'}');fonts.append({'path':str(f),'sha256':sha(f),'units':units,'ascent':asc,'descent':desc,'ratio':(asc-desc)/units})
 widths.append('{'+','.join([str(index),str(fm['size']),'0',json.dumps(sample),repr(chrome),'1'])+'}')
 for text in ['210','01:01.24','1/2','19/19','LAP','CHECKPOINT','SPEED','km/h']:
  for size in [15,20,34,40,64]:widths.append('{'+','.join([str(index),str(size),'0',json.dumps(text),repr(advance(text)*size),'0'])+'}')
flat=[c['rect'] for c in children]
(o/'hud_readouts_fixture.h').write_text('/* Reused verified Chrome boxes/Canvas widths and independent raw TTF metrics/advances. */\nstatic const BoxCase box_cases[]={\n'+',\n'.join(boxes)+'\n};\nstatic const FaceCase face_cases[]={'+','.join(face_rows)+'};\nstatic const WidthCase width_cases[]={\n'+',\n'.join(widths)+'\n};\n')
receipt={'package':str(pack),'manifest_sha256':sha(pack/'manifest.json'),'verified_files':[{'path':str(pack/name),'sha256':sha(pack/name)} for name in names],'current_css':{'path':str(p/'game/src/style.css'),'sha256':sha(p/'game/src/style.css')},'matching_css':rules,'fonts':fonts,'reused_layout_states':len(boxes),'reused_chrome_canvas_widths':4,'font_table_width_cases':len(widths)-4,'scope':'Reused checksum-verified asset-team Chromium panel/child rectangles and four Canvas widths; selected current CSS unchanged. Independent sfnt head/hhea/cmap/hmtx parser supplies native-face metrics/advances. Nominal baseline model, not fresh browser ink/baseline or GPU/raster acceptance.'}
(o/'hud-readouts-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps({'boxes':len(boxes),'chrome_widths':4,'ttf_widths':len(widths)-4,'fonts':[f['ratio'] for f in fonts]}))


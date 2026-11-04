from pathlib import Path
import json,hashlib,struct,zlib,math
p=Path(r'C:\Users\User\Projects\neon_c');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3');out=Path(__file__).parent/'hud-frame-fixture';out.mkdir(exist_ok=True)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
assert sha(pack/'manifest.json')=='e8eca0c29f6f392ca1a81ef602663de6438847db12306594011bb4677dfd0443';manifest=json.loads((pack/'manifest.json').read_text());assets=json.loads((pack/'assets.json').read_text());records=[next(v for v in assets if v['id']==id and v['scale']==1) for id in ['hud/lap-panel/race','hud/speed-panel/normal']]
files=['assets.json','source/style.css']+[v['chrome'] for v in records]
for file in files:assert sha(pack/file)==next(e['sha256'] for e in manifest['assets'] if e['file']==file),file
css=(p/'game/src/style.css').read_text(encoding='utf-8');prior=(pack/'source/style.css').read_text(encoding='utf-8');rules=[]
for prefix in ['* {','.panel {']:
 start=css.index(prefix);rule=css[start:css.index('}',start)+1];assert rule in prior;rules.append(rule)
assert 'border: 2px solid var(--c)' in rules[1] and 'polygon(18px 0' in rules[1]

def decode(file):
 data=file.read_bytes();at=8;packed=b''
 assert data[:8]==b'\x89PNG\r\n\x1a\n'
 while at<len(data):
  n=struct.unpack_from('>I',data,at)[0];name=data[at+4:at+8];chunk=data[at+8:at+8+n];crc=struct.unpack_from('>I',data,at+8+n)[0];assert zlib.crc32(name+chunk)==crc;at+=12+n
  if name==b'IHDR':w,h,depth,ctype,compression,filtering,interlace=struct.unpack('>IIBBBBB',chunk);assert(depth,ctype,compression,filtering,interlace)==(8,6,0,0,0)
  if name==b'IDAT':packed+=chunk
 raw=zlib.decompress(packed);assert len(raw)==h*(w*4+1);rows=[];prev=bytearray(w*4);at=0
 for y in range(h):
  filt=raw[at];at+=1;row=bytearray(raw[at:at+w*4]);at+=w*4
  for i in range(w*4):
   left=row[i-4] if i>=4 else 0;up=prev[i];ul=prev[i-4] if i>=4 else 0
   if filt==1:predict=left
   elif filt==2:predict=up
   elif filt==3:predict=(left+up)//2
   elif filt==4:
    q=left+up-ul;ds=[abs(q-left),abs(q-up),abs(q-ul)];predict=[left,up,ul][ds.index(min(ds))]
   else:assert filt==0;predict=0
   row[i]=(row[i]+predict)&255
  rows.append(row);prev=row
 return w,h,rows
rows=[];diagnostics=[];sizes=[]
for index,v in enumerate(records):
 width,height,pixels=decode(pack/v['chrome']);rect=v['pixelContentRect'];w=rect['width'];h=rect['height'];sizes.append('{'+str(w)+','+str(h)+'}');bad=0;classes=[0,0,0];samples=0
 for y in range(-2,h+2):
  for x in range(-2,w+2):
   px=x+.5;py=y+.5
   if 22<px<w-22 and 22<py<h-22:continue
   # Exclude antialiased cut lines, but retain both whole inside border rows.
   if min(abs(px+py-18),abs(w-px+py-18),abs(px+h-py-18),abs(w-px+h-py-18))/math.sqrt(2)<1.1:continue
   rgba=list(pixels[rect['y']+y][(rect['x']+x)*4:(rect['x']+x)*4+4]);alpha=rgba[3];kind=0 if alpha==0 else 2 if alpha==255 else 1
   inside=0<=px<=w and 0<=py<=h and min(px+py,w-px+py,px+h-py,w-px+h-py)>=18
   geometric=0 if not inside else 2 if min(px,py,w-px,h-py)<2 else 1
   if geometric!=kind:
    bad+=1
    if len(diagnostics)<8:diagnostics.append([v['id'],px,py,rgba,geometric,kind])
   rows.append('{'+','.join([str(index),str(px),str(py),str(kind)])+'}');classes[kind]+=1;samples+=1
 print(json.dumps({'panel':v['id'],'samples':samples,'classes':classes,'mask_mismatches':bad}))
assert not diagnostics,diagnostics
(out/'hud_frame_fixture.h').write_text('/* Verified prepared Chrome PNG frame masks, excluding antialiased cut lines; alpha classes0=outside1=fill2=opaque border. */\nstatic const PanelCase panel_cases[]={'+','.join(sizes)+'};\nstatic const FramePixel frame_pixels[]={\n'+',\n'.join(rows)+'\n};\n')
receipt={'version':json.loads((p/'game/package.json').read_text())['version'],'source':{'path':str(p/'game/src/style.css'),'sha256':sha(p/'game/src/style.css')},'package':str(pack),'manifest_sha256':sha(pack/'manifest.json'),'verified_files':[{'path':str(pack/file),'sha256':sha(pack/file)} for file in files],'rules':rules,'reused_png_frames':2,'reused_mask_pixels':len(rows),'sample_region':'Panel boundary band22px and2px outside; omit1.1px around four slanted cut lines to exclude anti-alias uncertainty. Keep whole axis-aligned inside border rows. Classes from decoded PNG alpha0/255/intermediate, not C geometry.','scope':'CRC-checked prepared Chrome PNG geometry mask classes, current source rules unchanged. Native GPU/color blending/AA/glow and compact/DPR rounding unmeasured; synthetic compact tests use nominal/source-border model only.'}
(out/'hud-frame-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps({'mask_pixels':len(rows),'frames':2}))

from pathlib import Path
import json,hashlib,struct,zlib,math,sys
p=Path(r'C:\Users\User\Projects\neon_c');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3');out=Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
manifest=json.loads((pack/'manifest.json').read_text(encoding='utf-8'));assert sha(pack/'manifest.json')=='e8eca0c29f6f392ca1a81ef602663de6438847db12306594011bb4677dfd0443';assets=json.loads((pack/'assets.json').read_text(encoding='utf-8'));records=[next(v for v in assets if v['id']==name and v['scale']==1) for name in ['pause/buttons/resume/default','pause/buttons/resume/disabled','pause/buttons/restart/default','pause/buttons/restart/disabled']]
files=['assets.json','source/style.css']+[v['chrome'] for v in records]
for file in files:assert sha(pack/file)==next(a['sha256'] for a in manifest['assets'] if a['file']==file),file
css=(p/'game/src/style.css').read_text(encoding='utf-8');old=(pack/'source/style.css').read_text(encoding='utf-8');rules=[]
for selector in ['button {','button.primary {','button:disabled {']:
 a=css.index("\nbutton {")+1 if selector=="button {" else css.index(selector);rule=css[a:css.index('}',a)+1];assert rule in old;rules.append(rule)
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

rows=[];colors=[];sizes=[];checks=[]
for index,v in enumerate(records):
 w,h,pixels=decode(pack/v['chrome']);r=v['pixelContentRect'];width=int(r['width']);height=int(r['height']);opacity=float(v['observed']['styles']['opacity']);primary='primary' in v['observed']['classes'];sizes.append('{'+','.join(map(str,[width,height,int(primary),opacity]))+'}');bad=0
 for y in range(-2,height+2):
  for x in range(-2,width+2):
   px=x+.5;py=y+.5
   if min(abs(px+py-14),abs(width-px+height-py-14))/math.sqrt(2)<1.1:continue
   rgba=list(pixels[int(r['y'])+y][(int(r['x'])+x)*4:(int(r['x'])+x)*4+4]);alpha=rgba[3];kind=0 if alpha==0 else 2 if abs(alpha-255*opacity)<.5 else 1
   inside=0<=px<=width and 0<=py<=height and min(px+py,width-px+height-py)>=14;expected=0 if not inside else 2 if min(px,py,width-px,height-py)<2 else 1
   if kind!=expected:bad+=1
   # Boundary band retains all inside border rows plus clear outside. Interiors are sampled for color separately.
   if min(px,py,width-px,height-py)<=18:rows.append('{'+','.join(map(str,[index,px,py,kind]))+'}')
   if expected==1 and (py<12 or py>height-12) and x%13==4 and y%7==3:colors.append('{'+','.join(map(str,[index,px,py]+rgba))+'}')
 checks.append(dict(id=v['id'],mask_mismatches=bad));assert bad==0,checks
(out/'button_surface_fixture.h').write_text('/* Prepared checksum-verified Chrome PNG masks and RGBA samples. No fresh browser run. */\nstatic const ButtonSource button_sources[]={'+','.join(sizes)+'};\nstatic const ButtonPixel button_pixels[]={\n'+',\n'.join(rows)+'\n};\nstatic const ButtonColor button_colors[]={\n'+',\n'.join(colors)+'\n};\n')
receipt=dict(version=json.loads((p/'game/package.json').read_text(encoding='utf-8'))['version'],source=dict(path=str(p/'game/src/style.css'),sha256=sha(p/'game/src/style.css')),package=str(pack),manifest_sha256=sha(pack/'manifest.json'),verified_files=[dict(path=str(pack/f),sha256=sha(pack/f)) for f in files],rules=rules,checks=checks,mask_pixels=len(rows),color_samples=len(colors),scope='Reused four default/disabled primary/normal browser chrome PNGs, CRC checked. Boundary mask band18px/outside2px excludes1.1px around two diagonal cuts. Interior RGBA samples use only top/bottom12px padding bands, outside the centered25px text line plus6px guard. Chrome layers still contain glyphs; naming is not proof of no text. No fresh engine/GPU raster, hover/shadow or compact/DPR verification.')
(out/'button-surface-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(dict(mask_pixels=len(rows),color_samples=len(colors),checks=checks)))

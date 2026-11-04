from pathlib import Path
import json,hashlib,struct,zlib,math
p=Path(r'C:\Users\User\Projects\neon_c');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3');out=Path(__file__).parent/'map-background-fixture';out.mkdir(exist_ok=True)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
assert sha(pack/'manifest.json')=='e8eca0c29f6f392ca1a81ef602663de6438847db12306594011bb4677dfd0443';m=json.loads((pack/'manifest.json').read_text());a=json.loads((pack/'assets.json').read_text());v=next(v for v in a if v['id']=='hud/minimap-frame' and v['scale']==1)
files=['assets.json','source/style.css',v['chrome']]
for file in files:assert sha(pack/file)==next(e['sha256'] for e in m['assets'] if e['file']==file),file
css=(p/'game/src/style.css').read_text(encoding='utf-8');prior=(pack/'source/style.css').read_text(encoding='utf-8');rules=[]
for prefix in ['* {','#minimap {']:
 start=css.index(prefix);rule=css[start:css.index('}',start)+1];assert rule in prior;rules.append(rule)
assert 'radial-gradient(rgba(10, 20, 70, 0.8), rgba(3, 5, 22, 0.85))' in rules[1];assert v['observed']['rect']['width']==240 and v['observed']['children'][0]['rect']['width']==236
assert 'padding-box border-box' in v['observed']['styles']['background']
rshapes=p/'win_app/third_party/raylib-6.2-source/raylib-02566453880e06bec97f5b8160fa6fe1e98397ca/src/rshapes.c';rs=rshapes.read_text();start=rs.index('void DrawCircleGradient(');fan=rs[start:rs.index('// Draw a piece',start)];assert 'i < 360; i += 10' in fan and 'rlColor4ub(inner.r' in fan and 'rlColor4ub(outer.r' in fan
# Independent standard-library PNG decoder:8-bit RGBA, CRC and five row filters.
data=(pack/v['chrome']).read_bytes();at=8;packed=b''
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
rect=v['pixelContentRect'];cx=rect['x']+rect['width']/2;cy=rect['y']+rect['height']/2;pixels=[]
for y in range(55,231,8):
 for x in range(97,273,8):
  dx=x+.5-cx;dy=y+.5-cy
  if math.hypot(dx,dy)>88:continue
  rgba=list(rows[y][4*x:4*x+4]);pixels.append('{'+','.join([repr(dx),repr(dy),'{'+','.join(map(str,rgba))+'}'])+'}')
assert len(pixels)==379
(out/'map_background_fixture.h').write_text('/* Reused verified Chrome static-frame interior pixels, not native GPU samples. */\nstatic const PixelCase pixel_cases[]={\n'+',\n'.join(pixels)+'\n};\n')
receipt={'version':json.loads((p/'game/package.json').read_text())['version'],'sources':[{'path':str(p/'game/src/style.css'),'sha256':sha(p/'game/src/style.css')},{'path':str(rshapes),'sha256':sha(rshapes)}],'package':str(pack),'manifest_sha256':sha(pack/'manifest.json'),'verified_files':[{'path':str(pack/file),'sha256':sha(pack/file)} for file in files],'rules':rules,'source_fan':fan,'reused_pixel_samples':len(pixels),'sample_region':'PNG pixel centers within radius88 of minimap center, gridstep8; interior excludes frame/text-edge/shadow-dominant pixels. No runtime/distance culling change.','gradient_origin':v['observed']['styles']['background'],'scope':'CRC-checked standard-library decoding of known-manifest prepared static Chrome PNG; current relevant CSS unchanged. CPU raylib36-fan interpolation model, not native framebuffer/pixel, border/glow, whole-disk or performance acceptance.'}
(out/'map-background-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps({'pixels':len(pixels),'image':[w,h],'center':[cx,cy]}))

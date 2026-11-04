from pathlib import Path
import json,hashlib,struct,zlib,math
p=Path(r'C:\Users\User\Projects\neon_c');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3');out=Path(__file__).parent/'arrow-art-fixture';out.mkdir(exist_ok=True)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
assert sha(pack/'manifest.json')=='e8eca0c29f6f392ca1a81ef602663de6438847db12306594011bb4677dfd0443';m=json.loads((pack/'manifest.json').read_text());assets=json.loads((pack/'assets.json').read_text());v=next(v for v in assets if v['id']=='hud/markers/edge-arrow' and v['scale']==1)
files=['assets.json','source/style.css',v['reference']]
for file in files:assert sha(pack/file)==next(e['sha256'] for e in m['assets'] if e['file']==file),file
css=(p/'game/src/style.css').read_text(encoding='utf-8');prior=(pack/'source/style.css').read_text(encoding='utf-8');rules=[]
for prefix in [':root {','html, body {','#edge-arrow {']:
 start=css.index(prefix);rule=css[start:css.index('}',start)+1];assert rule in prior;rules.append(rule)
assert v['observed']['styles']['transformOrigin']=='21px 30px';assert v['pixelContentRect']=={'x':64,'y':64,'width':42,'height':60};assert v['expectedPixels']=={'width':170,'height':188}
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

w,h,rows=decode(pack/v['reference']);sums=[0,0,0,0];clear=0;ink=0;bounds=[w,h,-1,-1]
for y,row in enumerate(rows):
 for x in range(w):
  rgba=list(row[x*4:x*4+4])
  for k in range(4):sums[k]+=rgba[k]
  if rgba[3]:ink+=1;bounds=[min(bounds[0],x),min(bounds[1],y),max(bounds[2],x),max(bounds[3],y)]
  else:clear+=1
assert w*h*4==127840 and all(rows[0][i]==0 for i in range(3,w*4,4));assert bounds[0]>0 and bounds[1]>0 and bounds[2]<w-1 and bounds[3]<h-1
(out/'arrow_pixels_fixture.h').write_text('/* Independent CRC-checked stdlib decoding of verified original browser PNG. */\nstatic const unsigned long long expected_sums[4]={'+','.join(str(v)+'ull' for v in sums)+'};\nstatic const int expected_ink='+str(ink)+',expected_clear='+str(clear)+';\n')
receipt={'version':json.loads((p/'game/package.json').read_text())['version'],'source':{'path':str(p/'game/src/style.css'),'sha256':sha(p/'game/src/style.css')},'package':str(pack),'manifest_sha256':sha(pack/'manifest.json'),'verified_files':[{'path':str(pack/file),'sha256':sha(pack/file)} for file in files],'rules':rules,'asset':str(pack/v['reference']),'asset_sha256':sha(pack/v['reference']),'bytes':(pack/v['reference']).stat().st_size,'dimensions':[w,h],'rgba_bytes':127840,'element_box':[42,60],'padding':[64,64],'transform_origin':[21,30],'rgba_sums':sums,'ink_pixels':ink,'clear_pixels':clear,'alpha_bounds':bounds,'scope':'Existing source-colored glyph plus shadow PNG copied byte-for-byte, not generated; independent CPU PNG decode/geometry metadata. Native upload/pixels/filtering/rotation/visual acceptance untested.'}
(out/'arrow-pixels-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps({'sums':sums,'ink':ink,'clear':clear,'bounds':bounds,'bytes':receipt['bytes']}))

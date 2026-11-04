from pathlib import Path
import json,hashlib,struct,zlib,collections,sys
p=Path(r'C:\Users\User\Projects\neon_c');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v2');out=Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
m=json.loads((pack/'manifest.json').read_text(encoding='utf-8'));assets=json.loads((pack/'assets.json').read_text(encoding='utf-8'));records=[next(v for v in assets if v['id']=='menu/native-widget-references/'+name and v['scale']==1) for name in ['sensitivity/min','sensitivity/mid','sensitivity/max','invert/off','invert/on']];files=['assets.json','source/style.css']+[v['reference'] for v in records]
for name in files:assert sha(pack/name)==next(a['sha256'] for a in m['assets'] if a['file']==name)
cssfile=p/'game/src/style.css';css=cssfile.read_text(encoding='utf-8');prior=(pack/'source/style.css').read_text(encoding='utf-8');rules=[]
for selector in ['.settings {','.settings input[type=range] {','.settings input[type=checkbox] {','.settings label {']:
 a=css.index('\n'+selector)+1;assert a>0;rule=css[a:css.index('}',a)+1];assert rule in prior;rules.append(rule)
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

widgets=[];observations=[];histograms=[]
for index,v in enumerate(records):
 fw,fh,pixels=decode(pack/v['reference']);r=v['pixelContentRect'];hist=collections.Counter();accent=[]
 for y in range(int(r['y']),int(r['y']+r['height'])+1):
  for x in range(int(r['x']),int(r['x']+r['width'])+1):
   rgba=tuple(pixels[y][x*4:x*4+4]);hist[rgba]+=1
   if index<3 and rgba[0]>240 and rgba[1]<70 and rgba[2]>190 and rgba[3]>200 and abs(y+.5-r['y']-r['height']/2)>4:accent.append(x+.5-r['x'])
 center=(min(accent)+max(accent))/2 if accent else 0;form=v['observed']['form'];value=float(form['value']) if index<3 else 0;widgets.append('{'+','.join(map(str,[int(index>=3),r['width'],r['height'],value,int(form['checked']),center]))+'}');observations.append(dict(id=v['id'],rect=v['observed']['rect'],form=form,accent_raster_center=center));histograms.append(hist)
assert (59,59,59,255) in histograms[0] and (133,133,133,255) in histograms[0] and (255,43,214,255) in histograms[1] and (255,255,255,255) in histograms[3] and (118,118,118,255) in histograms[3] and (255,255,255,255) in histograms[4]
colors=['{'+str(i)+','+str(value)+'u}' for i,value in enumerate([0x3b3b3bff,0x858585ff,0xff2bd6ff,0xffffffff,0x767676ff,0xffffffff])]
range_rect=records[0]['observed']['rect'];check_rect=records[3]['observed']['rect'];assert check_rect['x']-range_rect['x']==116 and check_rect['y']-range_rect['y']==2.5
(out/'mouse_widgets_fixture.h').write_text('/* Existing Windows Chromium native-widget geometry/color evidence, not portable bitmap skins or native raster acceptance. */\nstatic const WidgetSource widget_sources[]={'+','.join(widgets)+'};\nstatic const WidgetColor widget_colors[]={'+','.join(colors)+'};\n')
receipt=dict(version=json.loads((p/'game/package.json').read_text(encoding='utf-8'))['version'],source=dict(path=str(cssfile),sha256=sha(cssfile)),package=str(pack),manifest_sha256=sha(pack/'manifest.json'),verified_files=[dict(path=str(pack/f),sha256=sha(pack/f)) for f in files],rules=rules,observations=observations,rgba_histograms=[dict(id=records[i]['id'],dominant_colors=hist.most_common(8)) for i,hist in enumerate(histograms)],layout=dict(slider_width=96,slider_height=16,checkbox_width=13,checkbox_height=13,checkbox_from_slider_x=116,checkbox_from_slider_y=2.5),scope='Five existing Chromium Windows native-widget PNGs CRC-decoded; default colors present and range thumb center estimated from accent pixels outside rail, with subpixel paint snapping tolerance. Native analytic widget approximates platform look while retaining pointer operation; keyboard/OS accessibility, true theme/raster/AA, browser UA drag/click internals not qualified. No new image asset or graphics run.')
(out/'mouse-widgets-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(dict(widgets=5,range_centers=[v['accent_raster_center'] for v in observations[:3]],layout=receipt['layout'])))

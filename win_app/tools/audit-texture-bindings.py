"""Independent no-render GLB conversion / runtime PNG binding and CRC audit."""
from pathlib import Path
import hashlib,struct,json,zlib,sys
root=Path(sys.argv[1]);out=Path(sys.argv[2]);assert not out.exists()
sha=lambda b:hashlib.sha256(b).hexdigest()
def glb(data):
 assert data[:4]==b'glTF' and struct.unpack_from('<I',data,4)[0]==2 and struct.unpack_from('<I',data,8)[0]==len(data)
 n,kind=struct.unpack_from('<II',data,12);assert kind==0x4e4f534a
 meta=json.loads(data[20:20+n]);size,kind=struct.unpack_from('<II',data,20+n);assert kind==0x004e4942
 return meta,data[28+n:28+n+size]
def png_audit(data):
 assert data[:8]==b'\x89PNG\r\n\x1a\n';at=8;chunks=[];encoded=[];info=None
 while at<len(data):
  n=struct.unpack_from('>I',data,at)[0];kind=data[at+4:at+8];payload=data[at+8:at+8+n];crc=struct.unpack_from('>I',data,at+8+n)[0];assert len(payload)==n and zlib.crc32(kind+payload)&0xffffffff==crc
  chunks.append(kind.decode('ascii'));at+=12+n
  if kind==b'IHDR':info=struct.unpack('>IIBBBBB',payload)
  if kind==b'IDAT':encoded.append(payload)
  if kind==b'IEND':break
 assert at==len(data) and info is not None
 width,height,bits,color,compression,filtering,interlace=info;assert bits==8 and color in (2,6) and (compression,filtering,interlace)==(0,0,0) and width<=4096 and height<=4096
 channels=4 if color==6 else 3;stride=width*channels;raw=zlib.decompress(b''.join(encoded));assert len(raw)==height*(stride+1)
 filters=[raw[y*(stride+1)] for y in range(height)];assert all(f<=4 for f in filters)
 # Validate the entire decoded stream without allocating a GPU/image object.
 previous=bytearray(stride);pixel_hash=hashlib.sha256();alpha_min=255;alpha_max=0;rgb_max=0
 for y,f in enumerate(filters):
  start=y*(stride+1)+1;current=bytearray(raw[start:start+stride])
  if f:
   for i in range(stride):
    a=current[i-channels] if i>=channels else 0;b=previous[i];c=previous[i-channels] if i>=channels else 0
    if f==1:prediction=a
    elif f==2:prediction=b
    elif f==3:prediction=(a+b)//2
    else:
     p=a+b-c;pa=abs(p-a);pb=abs(p-b);pc=abs(p-c);prediction=a if pa<=pb and pa<=pc else b if pb<=pc else c
    current[i]=(current[i]+prediction)&255
  pixel_hash.update(current)
  if channels==4:
   alpha=current[3::4];alpha_min=min(alpha_min,min(alpha));alpha_max=max(alpha_max,max(alpha));rgb_max=max(rgb_max,max(current[0::4]),max(current[1::4]),max(current[2::4]))
  else:alpha_min=alpha_max=255;rgb_max=max(rgb_max,max(current))
  previous=current
 mips=0;w,h=width,height
 while True:
  mips+=w*h*4
  if w==h==1:break
  w=max(1,w//2);h=max(1,h//2)
 return dict(width=width,height=height,channels=channels,chunks=chunks,all_chunk_crcs_valid=True,all_scanline_filters_valid=True,decoded_sha256=pixel_hash.hexdigest(),alpha_range=[alpha_min,alpha_max],rgb_max=rgb_max,rgba_base_bytes=width*height*4,rgba_full_mips_bytes=mips)
results=[]
for name in ['cyan-tree','control-tower','city-crown']:
 conversion=json.loads((root/f'reference/{name}-conversion.json').read_text());geometry=json.loads((root/f'reference/{name}-geometry.json').read_text())
 source=(root/f'reference/source-assets/{name}.glb').read_bytes();converted=(root/conversion['output']).read_bytes()
 assert sha(source)==conversion['source_sha256'] and sha(converted)==conversion['output_sha256']
 assert sha((root/conversion['source']).resolve().read_bytes())==sha(source)
 a,ab=glb(source);b,bb=glb(converted);assert bb[:len(ab)]==ab
 for key in ['meshes','accessors','nodes','scenes','scene','materials','samplers']:assert a.get(key)==b.get(key),(name,key)
 material=b['materials'][b['meshes'][0]['primitives'][0].get('material',0)];texture=b['textures'][material['pbrMetallicRoughness']['baseColorTexture']['index']];image=b['images'][texture['source']];assert image['mimeType']=='image/png'
 view=b['bufferViews'][image['bufferView']];embedded=bb[view.get('byteOffset',0):view.get('byteOffset',0)+view['byteLength']]
 native=(root/geometry['texture']['path']).read_bytes();assert native==embedded and sha(native)==geometry['texture']['sha256']
 result=png_audit(native);result.update(name=name,source_glb_sha256=sha(source),source_original_bin_preserved=True,geometry_nodes_materials_samplers_preserved=True,native_png_equals_correct_converted_base_color_image=True,native_png_sha256=sha(native),source_base_color_factor=material['pbrMetallicRoughness'].get('baseColorFactor',[1,1,1,1]),source_double_sided=material.get('doubleSided',False),source_alpha_mode=material.get('alphaMode','OPAQUE'),source_sampler=b['samplers'][texture.get('sampler',0)])
 results.append(result)
out.write_text(json.dumps(dict(scope='Independent headless binary, texture binding and full PNG CRC/filter validation. No renderer, raster capture, decoder installation, new assets or WebP pixel-equivalence claim.',textures=results,failures=0),indent=2)+'\n')
print(json.dumps({'textures':len(results),'all_bindings_and_crc_valid':True,'textures_summary':[{k:x[k] for k in ['name','width','height','alpha_range','rgba_full_mips_bytes']} for x in results]}))

from pathlib import Path
import json,struct,hashlib,re
r=Path(__file__).resolve().parents[1]
if not (r/'assets').exists():r=Path(r'C:\Users\User\Projects\neon_c\win_app')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
path=r/'assets/models/roadster.glb';raw=path.read_bytes();assert raw[:4]==b'glTF';doc=json.loads(raw[20:20+struct.unpack_from('<I',raw,12)[0]])
car_attributes=0;car_indices=0;primitives=[]
for mesh in doc['meshes']:
 for p in mesh['primitives']:
  attrs=p['attributes'];n=doc['accessors'][attrs['POSITION']]['count'];cost=0
  for semantic,bytes_per_vertex in [('POSITION',12),('NORMAL',12),('TEXCOORD_0',8),('TEXCOORD_1',8),('TANGENT',16),('COLOR_0',4),('JOINTS_0',4),('WEIGHTS_0',16)]:
   if semantic in attrs:cost+=n*bytes_per_vertex
  indices=doc['accessors'][p['indices']]['count'] if 'indices' in p else 0
  car_attributes+=cost;car_indices+=indices*2;primitives.append({'vertices':n,'index_entries':indices,'attribute_bytes':cost,'semantics':list(attrs)})
course=list((r/'assets/course').glob('*.rrmesh'));course_vertices=sum(struct.unpack_from('<I',p.read_bytes(),4)[0] for p in course)
logpath=r/'reference/graphics-20261102/facade-windows-normal-original.log';log=logpath.read_text();textures=[]
for match in re.finditer(r'TEXTURE: \[ID (\d+)\] Texture loaded successfully \((\d+)x(\d+) \| ([^|]+) \| (\d+) mipmaps\)',log):
 i,w,h,fmt,levels=match.groups();i=int(i);w=int(w);h=int(h);levels=11 if i in [5,6,7] else 10 if i==4 else int(levels);bpp={'R8G8B8A8':4,'GRAY_ALPHA':2,'GRAYSCALE':1}[fmt.strip()];size=sum(max(1,w>>m)*max(1,h>>m)*bpp for m in range(levels));textures.append({'id':i,'width':w,'height':h,'format':fmt.strip(),'levels_from_source_load_path':levels,'logical_bytes':size})
assert len(textures)==11 and course_vertices==123888 and not doc.get('skins')
gpu_images=sum(t['logical_bytes'] for t in textures)+800*600*4
out={'scope':'Logical resource accounting from completed normal trial log and unchanged runtime GLB/RRC1 source; reused evidence, no graphics run. Not actual allocation or total application memory.','source_hashes':{'car':sha(path),'completed_log':sha(logpath)},'car_primitives':primitives,'car_attribute_bytes':car_attributes,'car_uint16_index_bytes':car_indices,'course_mesh_count':len(course),'course_vertices':course_vertices,'course_attribute_bytes':course_vertices*36,'textures':textures,'texture_and_depth_logical_bytes':gpu_images,'other_mesh_nominal_bytes':car_attributes+car_indices+course_vertices*36,'known_gpu_without_city_buildings':gpu_images+car_attributes+car_indices+course_vertices*36,'known_cpu_mesh_arrays_without_city_buildings':car_attributes+car_indices+course_vertices*36,'current_city_attribute_bound_bytes_per_copy':141312*36,'known_gpu_with_current_city_bound':gpu_images+car_attributes+car_indices+course_vertices*36+141312*36,'exclusions':['driver layout/padding/compression,shader/program memory','default swapchain,MSAA/window system storage','raylib default dynamic render batches,cube template,mesh/object/map/bone structures','core tracks,corridor,physics,other CPU state and transient loader allocations','experimental source fixture/hash analysis workspace (not app linked)','full source props/crowns/material parity costs not yet ported']}
p=Path(__file__).resolve().parents[1]/'reference/city-static-resource-budget.json';p.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps({k:out[k] for k in ['car_attribute_bytes','car_uint16_index_bytes','course_attribute_bytes','texture_and_depth_logical_bytes','known_gpu_without_city_buildings','known_cpu_mesh_arrays_without_city_buildings','known_gpu_with_current_city_bound']},indent=2))

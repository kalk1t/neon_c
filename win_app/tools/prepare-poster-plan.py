from pathlib import Path
import json,hashlib,re,struct,sys
if len(sys.argv)!=3:raise SystemExit('Usage: prepare-poster-plan.py WIN_APP_ROOT STAGING_DIRECTORY')
r=Path(sys.argv[1]);s=Path(sys.argv[2])
h=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
source=json.loads((s/'poster-reference.json').read_text());audit=json.loads((r/'reference/city-mesh-parity-audit.json').read_text());by_catalog={x['catalog_id']:x for x in audit['rows']}
canonical=(r/'src/core/rr_canonical_city_data.h').read_text().split('static const RRCanonicalBuilding rr_canonical_buildings[]={')[1].split('};')[0].strip().splitlines()
by_mesh={int(re.findall(r'-?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?',line)[19]):i for i,line in enumerate(canonical)}
catalog=[]
for line in (r/'src/experimental/generated_city_catalog.h').read_text().split('static const RRCatalogHero')[0].splitlines():
 m=re.search(r'},(\d+),(\d+),(-?\d+),(true|false),(\d+)},?$',line)
 if m:catalog.append(tuple(map(int,[m[1],m[2],m[3],m[5]])))
parts=[]
for line in (r/'src/experimental/generated_city_repacking.h').read_text().splitlines():
 m=re.search(r'},(\d+),(\d+),(\d+),(\d+)},?$',line)
 if m:parts.append(tuple(map(int,m.groups())))
raw=(r/'reference/city-repacking-geometry.bin').read_bytes();cursor=0;at=0;boards={}
for id,(_,vertices,mesh,poster_verts) in enumerate(catalog):
 end=at+vertices
 while cursor<len(parts) and parts[cursor][0]<end:
  first,count,material,batch=parts[cursor];assert first==at
  if material==4:assert count==6 and id not in boards;boards[id]=raw[first*32:(first+count)*32]
  at+=count;cursor+=1
 assert at==end
selected=[]
def fnv(b):
 x=2166136261
 for c in b:x=((x^c)*16777619)&0xffffffff
 return x
for p in source['posters']:
 captured=b''.join(struct.pack('<8f',*row) for row in p['rows']);assert captured==boards[p['catalog_id']]
 if p['catalog_id'] in by_catalog:
  binding=by_catalog[p['catalog_id']];assert binding['omitted_board_vertices']==6
  selected.append(dict(catalog_id=p['catalog_id'],mesh_id=binding['mesh_id'],canonical_index=by_mesh[binding['mesh_id']],rows=[list(struct.unpack_from('<8f',captured,i*32)) for i in range(6)],source_fnv=fnv(captured),source_sha256=hashlib.sha256(captured).hexdigest()))
assert len(boards)==349 and len(selected)==97
def f(x):
 t=format(x,'.9g');return t+('f' if '.' in t or 'e' in t.lower() else '.0f')
header='/* Exact source Float32 positions/normals/UVs. Test-only; no runtime asset writes. */\nstatic const RRPoster poster_entries[]={\n'
for p in selected:header+='{'+f"{p['canonical_index']},{p['mesh_id']},{p['catalog_id']},"+'{'+','.join('{'+','.join(f(x) for x in row)+'}' for row in p['rows'])+'}},\n'
header+='};\n'
expect='/* Independent source-VM capture hashes and Canvas glyph coordinates. */\nstatic const unsigned int poster_expected_hashes[]={'+','.join(str(p['source_fnv'])+'u' for p in selected)+'};\nstatic const double poster_expected_glyphs[4][7][3][2]={\n'
for q in range(4):expect+='{'+','.join('{'+','.join('{'+','.join(format(x,'.17g') for x in point)+'}' for point in glyph)+'}' for glyph in source['glyphs'][q*7:q*7+7])+'},\n'
expect+='};\n'
expect+='static const double poster_expected_colliders[97][8]={\n'+','.join('{'+','.join(format(x,'.17g') for x in [*p['collider']['center'],*p['collider']['half'],p['collider']['cosine'],p['collider']['sine']])+'}' for p in source['posters'] if p['catalog_id'] in by_catalog)+'\n};\n'
spec=source['material'];truth=lambda v:'true' if v else 'false'
flags=[spec['flip_y'],spec['color_space']=='srgb',spec['side']==2,spec['type']=='MeshBasicMaterial',spec['transparent'],spec['depth_test'],spec['depth_write'],spec['mipmaps'],spec['wrap_s']==spec['wrap_t']==1001,spec['premultiply_alpha'],spec['fog'],spec['tone_mapped']]
assert spec['min_filter']==1008 and spec['mag_filter']==1006
expect+='static const RRPosterMaterial poster_expected_material={'+f"{spec['width']},{spec['height']},{spec['anisotropy']},"+','.join(map(truth,flags))+f",{spec['color'][0]},{spec['opacity']}"+'};\n'
for name,data in [('src/experimental/generated_posters.h',header),('tests/generated_poster_reference.h',expect)]:p=s/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(data)
sizes={x['mesh_id']:x['native_vertices'] for x in audit['rows']};postermesh={x['mesh_id'] for x in selected}
rank=sorted((n+(12 if id in postermesh else 0),id) for id,n in sizes.items())
budget=dict(current_top64_vertices=sum(sorted(sizes.values())[-64:]),merged_double_sided_top64_vertices=sum(n for n,_ in rank[-64:]),merged_max_vertices=rank[-1][0],vertex_bound=141312,charged_vertex_bytes=36,added_double_sided_vertices=1164,added_double_sided_all_mesh_bytes=41904,atlas_width=512,atlas_height=1024,atlas_rgba_base_bytes=2097152,atlas_rgba_mips_bytes=2796204,atlas_cpu_decode_bytes=2097152,separate_draw_worst=128,aggregate_draw_worst=65,draw_bound=64,merged_draw_worst=64,merged_uploads_per_frame=2,geometry_available=True,verified_bitmap_available=False,runtime_integration=False)
(s/'poster-inventory.json').write_text(json.dumps(dict(sources=source['sources'],source_posters=349,canonical_posters=97,outside_tile_posters=252,source_material=source['material'],source_canvas_commands=len(source['commands']),budget=budget,entries=selected),indent=2)+'\n')
print(json.dumps(budget))

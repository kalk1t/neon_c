"""Headless canonical mesh/source comparison; never rewrites an asset.

Usage: python tools/audit-city-mesh-parity.py REPORT.json
Requires existing exact-source repacking fixture and catalog, not a game launch.
"""
from pathlib import Path
from collections import Counter
import hashlib,json,re,struct,sys
root=Path(__file__).resolve().parents[1]
# Staging use explicitly receives project; installed tool infers its parent.
if len(sys.argv)==3:root=Path(sys.argv[2])
output=Path(sys.argv[1]);assert not output.exists(),output
sha=lambda b:hashlib.sha256(b).hexdigest()
repack=json.loads((root/'reference/city-repacking.json').read_text())
catalog=json.loads((root/'reference/city-catalog.json').read_text())
manifest=json.loads((root/'reference/canonical-city-core.json').read_text())
for item in repack['sources']:
 assert sha((root.parent/'game'/item['path']).read_bytes())==item['sha256'],item['path']
raw=(root/'reference/city-repacking-geometry.bin').read_bytes()
assert sha(raw)==repack['geometry_sha256']
records=[]
for line in (root/'src/experimental/generated_city_catalog.h').read_text().split('static const RRCatalogHero')[0].splitlines():
 m=re.search(r'},(\d+),(\d+),(-?\d+),(true|false),(\d+)},?$',line)
 if m:records.append(dict(kind=int(m[1]),vertices=int(m[2]),mesh_id=int(m[3]),complete=m[4]=='true',poster_vertices=int(m[5])))
parts=[]
for line in (root/'src/experimental/generated_city_repacking.h').read_text().splitlines():
 m=re.search(r'},(\d+),(\d+),(\d+),(\d+)},?$',line)
 if m:parts.append(tuple(map(int,m.groups())))
assert len(records)==2741 and len(parts)==38352
palette=[bytes.fromhex(c) for c in ['0d0b24ff','14e4ffff','ff26d4ff','ff26d4ff']]
assets={int(re.search(r'b(\d+)\.rrmesh',a['path'])[1]):a for a in manifest['assets']}
rows=[];at=0;cursor=0;matched_vertices=0;missing_vertices=0;omitted_planes=0;checked=Counter()
for index,record in enumerate(records):
 end=at+record['vertices'];expected=bytearray();posters=0;by_material=Counter()
 while cursor<len(parts) and parts[cursor][0]<end:
  first,count,material,batch=parts[cursor];assert first==at and first+count<=end
  if material==4:posters+=count
  elif record['mesh_id']>=0:
   by_material[material]+=count
   for vertex in range(first,first+count):expected+=raw[vertex*32:(vertex+1)*32]+palette[material]
  at+=count;cursor+=1
 assert at==end and posters==record['poster_vertices']
 if record['mesh_id']>=0:
  asset=assets[record['mesh_id']];data=(root/asset['path']).read_bytes()
  assert sha(data)==asset['sha256'] and data[:4]==b'RRC1' and len(data)==asset['bytes']
  vertices=struct.unpack_from('<I',data,4)[0];assert vertices==asset['vertices'] and len(data)==8+vertices*36
  assert data[8:]==expected,('source attributes or base palette differ',index,asset['path'])
  assert vertices+posters==record['vertices']
  assert record['complete']==(posters==0)
  matched_vertices+=vertices;missing_vertices+=posters;omitted_planes+=posters//6;checked.update(by_material)
  rows.append(dict(catalog_id=index,mesh_id=record['mesh_id'],path=asset['path'],native_vertices=vertices,source_vertices=record['vertices'],omitted_board_vertices=posters,omitted_board_planes=posters//6,attributes_and_base_palette_byte_exact=True))
assert at==1695222 and cursor==len(parts) and len(rows)==413 and len({x['mesh_id'] for x in rows})==413
assert matched_vertices==manifest['total_vertices']
for item in repack['sources']:assert sha((root.parent/'game'/item['path']).read_bytes())==item['sha256']
result=dict(scope='Independent headless byte audit. Full position/normal/UV and captured base RGBA palette; excludes shader/PBR/lighting, poster raster and graphics. Existing fixtures, no new source export or assets.',source_version=json.loads((root.parent/'game/package.json').read_text())['version'],sources=repack['sources'],source_fixture_sha256=sha(raw),canonical_assets_checked=len(rows),matched_vertices=matched_vertices,matched_by_material=dict(checked),records_with_omitted_boards=sum(x['omitted_board_vertices']>0 for x in rows),omitted_board_vertices=missing_vertices,omitted_board_planes=omitted_planes,unrepresented_grid_towers=catalog['unrepresented_grid_towers'],rows=rows,failures=0)
output.write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({k:v for k,v in result.items() if k not in ['rows','sources']}))

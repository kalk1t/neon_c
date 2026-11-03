from pathlib import Path
import sys,json,re,math
r=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).resolve().parents[1]
numbers=lambda line:[float(n) for n in re.findall(r'-?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?',line)]
text=(r/'src/experimental/generated_city_catalog.h').read_text().split('static const RRCatalogEntry catalog_entries[]={',1)[1].split('};',1)[0]
rows=[numbers(line) for line in text.strip().splitlines()];assert len(rows)==2741
text=(r/'tests/generated_city_catalog_reference.h').read_text().split('static const CatalogViewFixture catalog_view_fixtures[]={',1)[1].split('};',1)[0]
views=[numbers(line) for line in text.strip().splitlines()];assert len(views)==108
results=[]
for index,v in enumerate(views):
 visible=[]
 for ordinal,e in enumerate(rows):
  center=[(e[i]+e[i+3])*.5 for i in range(3)];radius=math.sqrt(sum((e[i+3]-center[i])**2 for i in range(3)))
  if all(sum(v[3+plane*4+k]*center[k] for k in range(3))+v[6+plane*4]>=-radius for plane in range(6)):visible.append(ordinal)
 assert len(visible)==int(v[27]),index
 vertices=sum(int(rows[i][23]) for i in visible)
 results.append({'fixture':index,'road':index//12+1,'step':index%12,'visible_structures':len(visible),'vertices':vertices,'attribute_bytes_rgba':vertices*36,'over_64_records':len(visible)>64,'over_141312_vertices':vertices>141312})
out={'scope':'Offline geometry-only working-set analysis of 108 finer-grain per-structure sphere views, far1000; matches every captured Three.Frustum visible count. No runtime cap change, GPU measurement or browser merged-batch frame-cost claim. Crowns/props/traffic/material overhead excluded.','max_visible_records':max(x['visible_structures'] for x in results),'max_visible_vertices':max(x['vertices'] for x in results),'max_attribute_bytes_rgba':max(x['attribute_bytes_rgba'] for x in results),'views_over_64_records':sum(x['over_64_records'] for x in results),'views_over_141312_vertices':sum(x['over_141312_vertices'] for x in results),'views':results}
p=Path(__file__).resolve().parents[1]/'reference/city-catalog-budget.json';p.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps({k:v for k,v in out.items() if k!='views'},indent=2))

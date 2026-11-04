from pathlib import Path
import math,json,hashlib
o=Path(__file__).parent/'clip-fixture';data=json.loads((o/'clip-marker-inputs.json').read_text());rows=[]
def cross(a,b):return a[0]*b[1]-a[1]*b[0]
def area(triangle,r):
 result=0
 for a,b in zip(triangle,triangle[1:]+triangle[:1]):
  d=[b[k]-a[k] for k in range(2)];aa=sum(x*x for x in d);bb=2*sum(a[k]*d[k] for k in range(2));cc=sum(x*x for x in a)-r*r;cuts=[0.,1.];disc=bb*bb-4*aa*cc
  if aa and disc>0:
   for t in [(-bb-math.sqrt(disc))/(2*aa),(-bb+math.sqrt(disc))/(2*aa)]:
    if 0<t<1:cuts.append(t)
  cuts.sort()
  for u,v in zip(cuts,cuts[1:]):
   p=[a[k]+u*d[k] for k in range(2)];q=[a[k]+v*d[k] for k in range(2)];mid=[a[k]+(u+v)*.5*d[k] for k in range(2)]
   result+=(cross(p,q) if sum(x*x for x in mid)<=r*r else r*r*math.atan2(cross(p,q),sum(p[k]*q[k] for k in range(2))))*.5
 return abs(result)
for item in data['rows']:
 r=item['radius'];t=item['triangle'];exact=area(t,r);perimeter=sum(math.dist(a,b) for a,b in zip(t,t[1:]+t[:1]));delta=r*(1-math.cos(math.pi/128));bound=min(math.pi*r*r-64*r*r*math.sin(math.pi/64),delta*perimeter)
 rows.append('{'+str(r)+',{'+','.join('{'+','.join(map(repr,p))+'}' for p in t)+'},'+repr(exact)+','+repr(bound)+','+str(int(item['marker']))+'}')
(o/'circle_clip_fixture.h').write_text('/* Exact disk/triangle intersection from segment roots + oriented sector integrals, not polygon clipping. */\nstatic const ClipCase clip_cases[]={\n'+',\n'.join(rows)+'\n};\n')
source=Path(data['source']);assert hashlib.sha256(source.read_bytes()).hexdigest()==data['sha256']
css=source.parent/'style.css';csslines='\n'.join(x for x in css.read_text(encoding='utf-8').splitlines() if x.startswith(('#minimap {','#minimap canvas {')))
receipt={'source':data['source'],'source_sha256':data['sha256'],'race_drawMap_method_sha256':data['method_sha256'],'css_source':str(css),'css_sha256':hashlib.sha256(css.read_bytes()).hexdigest(),'selected_css':csslines,'cases':len(rows),'source_marker_triangles':sum(i['marker'] for i in data['rows']),'reference':'Independent analytic circle/triangle intersection via segment roots and oriented sector integrals; no polygon clipping in oracle. Bounds from inscribed circle deficit and radial gap times triangle perimeter.','radial_gap_at_118_css_pixels':118*(1-math.cos(math.pi/128)),'scope':'Actual current HUD.drawMap arrow commands with inert Canvas context; mathematically checked clipping, not a browser raster/mask or native GPU test.'}
(o/'circle-clip-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps({'cases':len(rows),'radial_gap_at_118_css_pixels':receipt['radial_gap_at_118_css_pixels']}))

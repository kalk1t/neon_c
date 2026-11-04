from pathlib import Path
import math,json
o=Path(__file__).parent/'map-marks-fixture';o.mkdir(exist_ok=True);rows=[]
def lens(R,r,d):
    if d>=R+r:return 0.
    if d<=abs(R-r):return math.pi*min(R,r)**2
    a=math.acos(max(-1,min(1,(d*d+r*r-R*R)/(2*d*r))))
    b=math.acos(max(-1,min(1,(d*d+R*R-r*r)/(2*d*R))))
    return r*r*a+R*R*b-.5*math.sqrt(max(0,(-d+r+R)*(d+r-R)*(d-r+R)*(d+r+R)))
for R in [94.4,118,236]:
    size=R*2;scale=size/440
    for radius in [7*scale,11*scale]:
      stroke=3*scale
      for outline in [0,1]:
        outer=radius+outline*stroke*.5;inner=max(0,radius-stroke*.5) if outline else 0
        for distance in [0,R*.5,R-outer,R,R+outer,R+outer*.5,R*1.1]:
          for angle in [0,.3,1.2,2.9]:
            x=distance*math.cos(angle);y=distance*math.sin(angle)
            exact=lens(R,outer,distance)-(lens(R,inner,distance) if inner else 0)
            deficit=lambda r,n:math.pi*r*r-n*.5*r*r*math.sin(2*math.pi/n)
            mask_bound=min(deficit(R,128),2*math.pi*outer*R*(1-math.cos(math.pi/128)))
            bound=deficit(outer,64)+deficit(inner,64)+(2 if outline else 1)*mask_bound
            rows.append('{'+','.join(map(repr,[R,x,y,radius,stroke]))+','+str(outline)+','+repr(exact)+','+repr(bound)+'}')
def polygon_disk_area(points,R):
    result=0
    def cross(a,b):return a[0]*b[1]-a[1]*b[0]
    for a,b in zip(points,points[1:]+points[:1]):
        d=[b[k]-a[k] for k in range(2)];aa=sum(v*v for v in d);bb=2*sum(a[k]*d[k] for k in range(2));cc=sum(v*v for v in a)-R*R;cuts=[0.,1.];discriminant=bb*bb-4*aa*cc
        if aa and discriminant>0:
            for t in [(-bb-math.sqrt(discriminant))/(2*aa),(-bb+math.sqrt(discriminant))/(2*aa)]:
                if 0<t<1:cuts.append(t)
        cuts.sort()
        for u,v in zip(cuts,cuts[1:]):
            p=[a[k]+u*d[k] for k in range(2)];q=[a[k]+v*d[k] for k in range(2)];mid=[a[k]+(u+v)*.5*d[k] for k in range(2)]
            result+=(cross(p,q) if sum(z*z for z in mid)<=R*R else R*R*math.atan2(cross(p,q),sum(p[k]*q[k] for k in range(2))))*.5
    return abs(result)
rects=[]
for R in [94.4,118,236]:
    dx=12*(R*2)/440;dy=3*(R*2)/440
    for distance in [0,R-dx,R,R+dx*.5,R+dx]:
        for angle in [0,.3,1.2,2.9]:
            x=distance*math.cos(angle);y=distance*math.sin(angle);points=[[x-dx,y-dy],[x+dx,y-dy],[x+dx,y+dy],[x-dx,y+dy]]
            exact=polygon_disk_area(points,R);bound=min(math.pi*R*R-64*R*R*math.sin(math.pi/64),4*(dx+dy)*R*(1-math.cos(math.pi/128)))
            rects.append('{'+','.join(map(repr,[R,x,y,exact,bound]))+'}')
(o/'minimap_marks_geometry.h').write_text('/* Independent exact disk lens / rectangle-sector integrals; no polygon clipping in oracle. */\nstatic const GeometryCase geometry_cases[]={\n'+',\n'.join(rows)+'\n};\nstatic const RectCase rect_cases[]={\n'+',\n'.join(rects)+'\n};\n')
receipt={'circle_annulus_cases':len(rows),'rectangle_cases':len(rects),'reference':'Exact circle/circle lens formula, outer-minus-inner for annuli; exact rectangle/disk via segment roots and oriented sector integrals. Explicit polygon/mask area-deficit bounds; no triangle clipping in reference.','mark_segments':64,'mask_segments':128,'max_outline_radius_at_desktop':12.5*236/440,'mark_radial_deficit_at_desktop':(12.5*236/440)*(1-math.cos(math.pi/64)),'mask_radial_deficit_at_desktop':118*(1-math.cos(math.pi/128))}
(o/'minimap-marks-geometry.json').write_text(json.dumps(receipt,indent=2)+'\n');print(receipt)

"""Pure-Python starter mesh construction shared by Blender and offline checks.

Vertices use canonical game axes. Faces are outward-wound quads. UVs are per
face-loop, so the final seam can use total length rather than wrap to zero.
"""
import math
from route_tools import add, mul, frame


def deck_mesh(route, width=18, drop=3, thickness=1.2, spacing=8, uv_repeat_m=12):
    samples = route.samples(spacing)
    vertices, faces, uvs, materials = [], [], [], []
    for sample in samples:
        center = sample["position_game"][:]
        center[1] -= drop
        right, _, _ = frame(sample["forward_game"])
        left, right_point = add(center, mul(right, -width/2)), add(center, mul(right, width/2))
        vertices.extend([left, right_point, add(left, [0, -thickness, 0]), add(right_point, [0, -thickness, 0])])
    for i in range(len(samples)):
        j = (i+1) % len(samples)
        a, b = i*4, j*4
        v0 = samples[i]["distance_m"]/uv_repeat_m
        v1 = (samples[j]["distance_m"] if j else route.total_length)/uv_repeat_m
        faces.extend([[a,b,b+1,a+1], [a+2,a+3,b+3,b+2], [a,a+2,b+2,b], [a+1,b+1,b+3,a+3]])
        uvs.extend([[(0,v0),(0,v1),(1,v1),(1,v0)], [(0,v0),(1,v0),(1,v1),(0,v1)],
                    [(0,v0),(thickness/width,v0),(thickness/width,v1),(0,v1)],
                    [(0,v0),(0,v1),(thickness/width,v1),(thickness/width,v0)]])
        materials.extend([0,1,1,1])
    return vertices, faces, uvs, materials


def edge_strip_mesh(route, lateral_offset, width=.20, drop=2.96, spacing=8):
    samples = route.samples(spacing)
    vertices, faces, uvs = [], [], []
    for sample in samples:
        right, _, _ = frame(sample["forward_game"])
        center = add(sample["position_game"], [0,-drop,0])
        vertices += [add(center,mul(right,lateral_offset-width/2)), add(center,mul(right,lateral_offset+width/2))]
    for i in range(len(samples)):
        j=(i+1)%len(samples)
        faces.append([i*2,j*2,j*2+1,i*2+1])
        a=samples[i]["distance_m"]/12
        b=(samples[j]["distance_m"] if j else route.total_length)/12
        uvs.append([(0,a),(0,b),(1,b),(1,a)])
    return vertices, faces, uvs, [0]*len(faces)


def ring_mesh(center, forward, inner_radius=11, outer_radius=13, depth=.8, segments=48):
    right, up, tangent=frame(forward)
    vertices, faces, uvs, materials=[],[],[],[]
    for i in range(segments):
        angle=math.tau*i/segments
        radial=add(mul(right,math.cos(angle)),mul(up,math.sin(angle)))
        for radius, axial in [(inner_radius,depth/2),(outer_radius,depth/2),(inner_radius,-depth/2),(outer_radius,-depth/2)]:
            vertices.append(add(center,add(mul(radial,radius),mul(tangent,axial))))
    for i in range(segments):
        a,b=i*4,((i+1)%segments)*4
        faces.extend([[a,a+1,b+1,b], [a+2,b+2,b+3,a+3], [a+1,a+3,b+3,b+1], [a,b,b+2,a+2]])
        u0,u1=i/segments,(i+1)/segments
        uvs.extend([[(u0,0),(u0,1),(u1,1),(u1,0)]]*4)
        materials.extend([0,0,1,0])
    return vertices,faces,uvs,materials


def box_mesh(center, size):
    x,y,z=[v/2 for v in size]
    vertices=[[center[0]+a,center[1]+b,center[2]+c] for a,b,c in [(-x,-y,-z),(x,-y,-z),(x,y,-z),(-x,y,-z),(-x,-y,z),(x,-y,z),(x,y,z),(-x,y,z)]]
    faces=[[0,3,2,1],[4,5,6,7],[0,4,7,3],[1,2,6,5],[0,1,5,4],[3,7,6,2]]
    return vertices,faces,[[(0,0),(0,1),(1,1),(1,0)] for _ in faces],[0]*len(faces)


def start_platform_mesh(route, center_distance, width=24, run_length=48, drop=3.08, thickness=1.4):
    """Open swept platform follows local slope, with closed ends and no flat ledge."""
    count=13
    vertices,faces,uvs,materials=[],[],[],[]
    for i in range(count):
        p,t=route.at_distance(center_distance-run_length/2+run_length*i/(count-1))
        right,_,_=frame(t)
        center=add(p,[0,-drop,0])
        left,rt=add(center,mul(right,-width/2)),add(center,mul(right,width/2))
        vertices.extend([left,rt,add(left,[0,-thickness,0]),add(rt,[0,-thickness,0])])
    for i in range(count-1):
        a,b=i*4,(i+1)*4
        faces.extend([[a,b,b+1,a+1],[a+2,a+3,b+3,b+2],[a,a+2,b+2,b],[a+1,b+1,b+3,a+3]])
        v0,v1=i/(count-1)*4,(i+1)/(count-1)*4
        uvs.extend([[(0,v0),(0,v1),(1,v1),(1,v0)],[(0,v0),(1,v0),(1,v1),(0,v1)],[(0,v0),(1,v0),(1,v1),(0,v1)],[(0,v0),(0,v1),(1,v1),(1,v0)]])
        materials.extend([0,1,1,1])
    a=(count-1)*4
    faces.extend([[0,1,3,2],[a,a+2,a+3,a+1]])
    uvs.extend([[(0,0),(1,0),(1,1),(0,1)]]*2)
    materials.extend([1,1])
    return vertices,faces,uvs,materials


def frustum_mesh(center_base, height, bottom_radius, top_radius, segments=10):
    """Low-poly upright frustum for replaceable trees, rocks, and machinery."""
    vertices,faces,uvs=[],[],[]
    for y,r in [(0,bottom_radius),(height,top_radius)]:
        vertices += [[center_base[0]+r*math.cos(math.tau*i/segments),center_base[1]+y,center_base[2]+r*math.sin(math.tau*i/segments)] for i in range(segments)]
    for i in range(segments):
        j=(i+1)%segments
        faces.append([i,segments+i,segments+j,j])
        uvs.append([(i/segments,0),(i/segments,1),((i+1)/segments,1),((i+1)/segments,0)])
    faces.extend([list(range(segments)),list(range(segments*2-1,segments-1,-1))])
    for face in faces[-2:]:
        uvs.append([(.5+.5*math.cos(math.tau*(i%segments)/segments),.5+.5*math.sin(math.tau*(i%segments)/segments)) for i in face])
    return vertices,faces,uvs,[0]*len(faces)

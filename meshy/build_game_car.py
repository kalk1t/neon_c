# Builds the game car file from a raw Meshy .glb in one go:
#   - orientation/scale: nose towards glTF -Z (Blender +Y), 5.6 m long, centred on the origin
#   - two levels of detail sharing one texture set: "hi" (player, chase camera close-ups) and "lo" (rivals)
#   - the four thruster pods split off the body as separate objects with their pivot at the pod centre, so the game
#     can tilt them (names: <lod>_body, <lod>_pod_<FL|FR|RL|RR>)
# Usage: blender -b --python build_game_car.py -- <raw.glb> <out.glb> <hi_tris> <lo_tris> <texture_px> [pod_x pod_y [pod_x_rear pod_y_rear]]
# pod_x: faces whose centre is further than this from the centre line (in metres) belong to a pod; run once without
# it to print the width profile and pick the gap between body and pods.
import sys, math
import bpy, bmesh
from mathutils import Vector, Matrix

argv = sys.argv[sys.argv.index('--') + 1:]
src, dst, hi_tris, lo_tris, tex_px = argv[0], argv[1], int(argv[2]), int(argv[3]), int(argv[4])
pod_x = float(argv[5]) if len(argv) > 5 else None
pod_y = float(argv[6]) if len(argv) > 6 else 0.0  # pods sit further than this from the middle along the length
# optional separate limits for the rear pods (car02: its rear wing reaches further out than the front body)
pod_x_rear = float(argv[7]) if len(argv) > 7 else pod_x
pod_y_rear = float(argv[8]) if len(argv) > 8 else pod_y
LENGTH = 5.6

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
bpy.context.view_layer.objects.active = meshes[0]
for o in meshes:
    o.select_set(True)
if len(meshes) > 1:
    bpy.ops.object.join()
car = bpy.context.view_layer.objects.active
for o in list(bpy.context.scene.objects):
    if o is not car:
        bpy.data.objects.remove(o)
car.data.transform(car.matrix_world)
car.matrix_world = Matrix.Identity(4)
car.data.transform(Matrix.Rotation(math.radians(-90), 4, 'Z'))  # Meshy nose -X -> +Y

co = [v.co for v in car.data.vertices]
lo = Vector((min(c.x for c in co), min(c.y for c in co), min(c.z for c in co)))
hi = Vector((max(c.x for c in co), max(c.y for c in co), max(c.z for c in co)))
if hi.y - lo.y < hi.x - lo.x:  # model came out sideways: turn it so its long axis runs along Y
    car.data.transform(Matrix.Rotation(math.radians(90), 4, 'Z'))
    co = [v.co for v in car.data.vertices]
    lo = Vector((min(c.x for c in co), min(c.y for c in co), min(c.z for c in co)))
    hi = Vector((max(c.x for c in co), max(c.y for c in co), max(c.z for c in co)))
s = LENGTH / (hi.y - lo.y)
car.data.transform(Matrix.Scale(s, 4) @ Matrix.Translation(-(lo + hi) / 2))
size = (hi - lo) * s
print(f'SIZE w={size.x:.2f} l={size.y:.2f} h={size.z:.2f} TRIS_RAW {sum(len(p.vertices) - 2 for p in car.data.polygons)}')

# width profile: share of faces per 10 cm band of |x| (the body/pod gap shows up as a dip)
bands = {}
for p in car.data.polygons:
    b = int(abs(p.center.x) * 10)
    bands[b] = bands.get(b, 0) + 1
print('PROFILE', ' '.join(f'{b / 10:.1f}:{bands.get(b, 0)}' for b in range(0, 24)))

for img in bpy.data.images:
    if img.size[0] > tex_px:
        img.scale(tex_px, tex_px)


def decimated_copy(name, tris):
    o = car.copy()
    o.data = car.data.copy()
    o.name = name
    bpy.context.scene.collection.objects.link(o)
    n = sum(len(p.vertices) - 2 for p in o.data.polygons)
    if tris < n:
        d = o.modifiers.new('decimate', 'DECIMATE')
        d.ratio = tris / n
        bpy.context.view_layer.objects.active = o
        bpy.ops.object.modifier_apply(modifier=d.name)
    # crisp panel edges: smooth shading only within 35 degrees, so reflections don't wobble across the facets
    for poly in o.data.polygons:
        poly.use_smooth = True
    o.data.set_sharp_from_angle(angle=math.radians(35))
    return o


def in_pod(f, sx, sy):
    c = f.calc_center_median()
    if sy < 0:
        return c.x * sx > pod_x_rear and c.y * sy > pod_y_rear
    return c.x * sx > pod_x and c.y * sy > pod_y


def split_pods(o):
    """Moves pod faces into four objects whose origin sits at each pod's centre."""
    if pod_x is None:
        return
    for key, sx, sy in (('FL', -1, 1), ('FR', 1, 1), ('RL', -1, -1), ('RR', 1, -1)):
        bm = bmesh.new()
        bm.from_mesh(o.data)
        keep = [f for f in bm.faces if in_pod(f, sx, sy)]
        if not keep:
            bm.free()
            continue
        pod_mesh = bpy.data.meshes.new(f'{o.name}_pod_{key}')
        pbm = bmesh.new()
        pbm.from_mesh(o.data)
        drop = [f for f in pbm.faces if not (in_pod(f, sx, sy))]
        bmesh.ops.delete(pbm, geom=drop, context='FACES')
        bmesh.ops.delete(pbm, geom=[v for v in pbm.verts if not v.link_faces], context='VERTS')
        pbm.to_mesh(pod_mesh)
        pbm.free()
        bmesh.ops.delete(bm, geom=keep, context='FACES')
        bmesh.ops.delete(bm, geom=[v for v in bm.verts if not v.link_faces], context='VERTS')
        bm.to_mesh(o.data)
        bm.free()
        for m in o.data.materials:
            pod_mesh.materials.append(m)
        pts = [v.co for v in pod_mesh.vertices]
        c = (Vector(map(min, zip(*pts))) + Vector(map(max, zip(*pts)))) / 2  # bounding-box centre
        pod_mesh.transform(Matrix.Translation(-c))
        pod = bpy.data.objects.new(f'{o.name}_pod_{key}', pod_mesh)
        pod.location = c
        bpy.context.scene.collection.objects.link(pod)
        print(f'POD {pod.name} centre=({c.x:.2f},{c.y:.2f},{c.z:.2f}) faces={len(pod_mesh.polygons)}')
    o.data.update()


for name, tris in (('hi', hi_tris), ('lo', lo_tris)):
    o = decimated_copy(name, tris)
    split_pods(o)
    o.name = f'{name}_body'
bpy.data.objects.remove(car)
for o in bpy.context.scene.objects:
    print('OBJECT', o.name, sum(len(p.vertices) - 2 for p in o.data.polygons))

bpy.ops.export_scene.gltf(filepath=dst, export_format='GLB', export_image_format='WEBP', export_image_quality=90)
print('EXPORTED', dst)

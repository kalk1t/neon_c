"""Review: a model (left) and its collision boxes (right, red) side by side, two views.
  blender -b --factory-startup --python colview.py -- <model.glb> <boxes.json> <out.png>
"""
import bpy, sys, os, json, math
from mathutils import Vector

glb, boxes_f, out = sys.argv[sys.argv.index('--') + 1:][:3]
bpy.ops.wm.read_factory_settings(use_empty=True)
S = bpy.context.scene
bpy.ops.import_scene.gltf(filepath=glb)
model = [o for o in S.objects if o.type == 'MESH']
lo = Vector((1e9, 1e9, 1e9)); hi = -lo
for o in model:
    for c in o.bound_box:
        w = o.matrix_world @ Vector(c)
        lo = Vector(map(min, lo, w)); hi = Vector(map(max, hi, w))
size = hi - lo
shift = size.x * 1.3 + 10
boxes = json.load(open(boxes_f))
verts, faces = [], []
for (x0, x1, y0, y1, z0, z1) in boxes:
    b = len(verts)
    x0 += shift; x1 += shift
    verts += [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0), (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)]
    faces += [(b, b + 3, b + 2, b + 1), (b + 4, b + 5, b + 6, b + 7), (b, b + 1, b + 5, b + 4), (b + 1, b + 2, b + 6, b + 5),
              (b + 2, b + 3, b + 7, b + 6), (b + 3, b, b + 4, b + 7)]
me = bpy.data.meshes.new('col'); me.from_pydata(verts, [], faces); me.update()
m = bpy.data.materials.new('red'); m.use_nodes = True
m.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value = (0.8, 0.12, 0.08, 1)
me.materials.append(m)
S.collection.objects.link(bpy.data.objects.new('col', me))
S.render.engine = 'BLENDER_EEVEE' if 'BLENDER_EEVEE' in [e.identifier for e in bpy.types.RenderSettings.bl_rna.properties['engine'].enum_items] else 'CYCLES'
if S.render.engine == 'CYCLES':
    S.cycles.samples = 16
w = bpy.data.worlds.new('w'); w.use_nodes = True
w.node_tree.nodes['Background'].inputs['Color'].default_value = (0.6, 0.65, 0.7, 1)
S.world = w
sun = bpy.data.objects.new('sun', bpy.data.lights.new('sun', 'SUN')); sun.data.energy = 3
sun.rotation_euler = (math.radians(50), 0, math.radians(30)); S.collection.objects.link(sun)
cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam')); S.collection.objects.link(cam); S.camera = cam
cam.data.type = 'ORTHO'
mid = Vector(((lo.x + hi.x + shift) / 2, (lo.y + hi.y) / 2, (lo.z + hi.z) / 2))
span = max(size.x * 2.4 + 10, size.z * 1.1)
cam.data.ortho_scale = span
S.render.resolution_x, S.render.resolution_y = 1400, int(1400 * min(1.6, max(0.5, size.z * 1.1 / span)))
base, ext = os.path.splitext(out)
for k, (az, el) in enumerate([(-35, 25), (-90 - 35, 60)]):
    d = Vector((math.sin(math.radians(az)) * math.cos(math.radians(el)), -math.cos(math.radians(az)) * math.cos(math.radians(el)), math.sin(math.radians(el))))
    cam.location = mid + d * 2000
    cam.rotation_euler = (-d).to_track_quat('-Z', 'Y').to_euler()
    cam.data.clip_end = 5000
    S.render.filepath = f'{base}_{k}{ext}'
    bpy.ops.render.render(write_still=True)

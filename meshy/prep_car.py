# Prepares a Meshy car model for the browser game.
# Usage: blender -b --python prep_car.py -- <in.glb> <out.glb> <target_triangles> [texture_px]
# Reports the model's size/orientation, reduces the triangle count, shrinks textures and exports a compact .glb.
import sys
import bpy
from mathutils import Vector

argv = sys.argv[sys.argv.index('--') + 1:]
src, dst, target = argv[0], argv[1], int(argv[2])
tex_px = int(argv[3]) if len(argv) > 3 else 1024

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']


def tris(objs):
    deps = bpy.context.evaluated_depsgraph_get()
    n = 0
    for o in objs:
        m = o.evaluated_get(deps).to_mesh()
        m.calc_loop_triangles()
        n += len(m.loop_triangles)
        o.evaluated_get(deps).to_mesh_clear()
    return n


lo = Vector((1e9, 1e9, 1e9)); hi = Vector((-1e9, -1e9, -1e9))
for o in meshes:
    for c in o.bound_box:
        w = o.matrix_world @ Vector(c)
        lo = Vector(map(min, lo, w)); hi = Vector(map(max, hi, w))
before = tris(meshes)
print(f'MESHES {len(meshes)} TRIS {before} BBOX_MIN {tuple(round(v, 3) for v in lo)} BBOX_MAX {tuple(round(v, 3) for v in hi)}')

ratio = min(1.0, target / max(before, 1))
for o in meshes:
    if ratio < 1:
        d = o.modifiers.new('decimate', 'DECIMATE')
        d.ratio = ratio
        d.use_collapse_triangulate = True
    bpy.context.view_layer.objects.active = o
    for m in list(o.modifiers):
        bpy.ops.object.modifier_apply(modifier=m.name)
print(f'TRIS_AFTER {tris(meshes)}')

for img in bpy.data.images:
    if img.size[0] > tex_px:
        img.scale(tex_px, tex_px)
    print(f'IMAGE {img.name} {img.size[0]}x{img.size[1]}')

bpy.ops.export_scene.gltf(filepath=dst, export_format='GLB', export_image_format='WEBP', export_apply=True)
print('EXPORTED', dst)

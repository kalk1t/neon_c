# Blender: Meshy GLB -> game prop. Decimate to a triangle budget, textures to 1024 WebP, centred on x/z with the base at y=0,
# scaled so the longest side is 1 unit (the game scales it). Usage: blender -b --python optimize.py -- in.glb out.glb tris
import bpy, sys
a = sys.argv[sys.argv.index('--') + 1:]
src, dst, budget = a[0], a[1], int(a[2])
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
meshes = [o for o in bpy.data.objects if o.type == 'MESH']
bpy.ops.object.select_all(action='DESELECT')
for o in meshes:
    o.select_set(True)
bpy.context.view_layer.objects.active = meshes[0]
if len(meshes) > 1:
    bpy.ops.object.join()
ob = bpy.context.view_layer.objects.active
bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
ob.data.calc_loop_triangles()
n = len(ob.data.loop_triangles)
if n > budget * 3:  # loose leaf cards: weld them first so the decimator can collapse them
    bpy.ops.object.mode_set(mode='EDIT'); bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.remove_doubles(threshold=0.004); bpy.ops.object.mode_set(mode='OBJECT')
for _ in range(4):  # some meshes need more than one pass
    ob.data.calc_loop_triangles()
    n = len(ob.data.loop_triangles)
    if n <= budget * 1.15:
        break
    d = ob.modifiers.new('dec', 'DECIMATE')
    d.ratio = max(0.02, budget / n)
    d.use_collapse_triangulate = True
    bpy.ops.object.modifier_apply(modifier='dec')
# normalise: base at 0, centred, longest side 1
import mathutils
bb = [ob.matrix_world @ mathutils.Vector(c) for c in ob.bound_box]
mn = mathutils.Vector((min(v.x for v in bb), min(v.y for v in bb), min(v.z for v in bb)))
mx = mathutils.Vector((max(v.x for v in bb), max(v.y for v in bb), max(v.z for v in bb)))
size = max(mx - mn)
for v in ob.data.vertices:
    v.co = (v.co - mathutils.Vector(((mn.x + mx.x) / 2, (mn.y + mx.y) / 2, mn.z))) / size
for im in bpy.data.images:
    if im.size[0] > 1024:
        im.scale(1024, 1024)
ob.data.calc_loop_triangles()
print('OUT', dst, 'tris', len(ob.data.loop_triangles), 'dims', [round(x / size, 3) for x in (mx - mn)])
bpy.ops.export_scene.gltf(filepath=dst, export_format='GLB', export_image_format='WEBP', export_image_quality=82, use_selection=False)

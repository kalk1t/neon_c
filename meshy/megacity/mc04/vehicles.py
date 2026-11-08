"""v0.3.6 city vehicles from the Meshy models (mc04/run.sh): decimated, sized, front = glTF +z (Blender -Y),
standing on the ground at the origin, texture 512 px. The white paint takes each car's tint in the game
(instance colour x texture); the taxi and the bus keep theirs.
  blender -b --factory-startup --python vehicles.py -- <mc04 dir> <out dir> [review]
"""
import bpy, sys, os, math, json
from mathutils import Vector, Matrix

args = sys.argv[sys.argv.index('--') + 1:]
SRC, OUT = args[0], args[1]
REVIEW = 'review' in args
# name: (length or size m, the axis the model's long side lies on, front sign along it, target triangles, which size)
SPEC = {
    'sedan': (4.7, 'x', None, 2400, 'len'), 'hatch': (4.0, 'x', None, 2400, 'len'), 'van': (5.2, 'x', None, 1600, 'len'),
    'taxi': (4.7, 'x', None, 1600, 'len'), 'bus': (12.0, 'x', None, 2400, 'len'),
}
WELD = {'sedan': 0.003, 'hatch': 0.003, 'bus': 0.003}
SYM = set()
FRONT = json.load(open(os.path.join(SRC, 'front.json'))) if os.path.exists(os.path.join(SRC, 'front.json')) else {}
os.makedirs(OUT, exist_ok=True)
report = {}
for name, (size, axis, _, target, how) in SPEC.items():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=os.path.join(SRC, name, 'model.glb'))
    obs = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    bpy.ops.object.select_all(action='DESELECT')
    for o in obs: o.select_set(True)
    bpy.context.view_layer.objects.active = obs[0]
    if len(obs) > 1: bpy.ops.object.join()
    ob = bpy.context.view_layer.objects.active
    bpy.ops.object.parent_clear(type='CLEAR_KEEP_TRANSFORM')
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    n0 = sum(len(p.vertices) - 2 for p in ob.data.polygons)
    # Meshy meshes are triangle soup (every triangle its own corners): weld them first, or the decimation shatters
    bpy.ops.object.mode_set(mode='EDIT'); bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.remove_doubles(threshold=WELD.get(name, 0.0005)); bpy.ops.mesh.delete_loose(); bpy.ops.object.mode_set(mode='OBJECT')
    # gently, in steps (one big collapse folds thin panels over each other)
    for _ in range(12):
        n = sum(len(p.vertices) - 2 for p in ob.data.polygons)
        if n <= target * 1.05: break
        m = ob.modifiers.new('dec', 'DECIMATE'); m.ratio = max(target / n, 0.35); m.use_collapse_triangulate = True
        m.use_symmetry = name in SYM; m.symmetry_axis = 'Z'
        bpy.ops.object.modifier_apply(modifier='dec')
    # front: Blender -Y (glTF +z). front.json says where the model's front is: '+x', '-x', '+y', '-y'
    f = FRONT.get(name, '+x')
    rot = {'+x': -math.pi / 2, '-x': math.pi / 2, '+y': math.pi, '-y': 0.0}[f]
    ob.data.transform(Matrix.Rotation(rot, 4, 'Z'))
    vs = [v.co for v in ob.data.vertices]
    lo = Vector((min(v.x for v in vs), min(v.y for v in vs), min(v.z for v in vs)))
    hi = Vector((max(v.x for v in vs), max(v.y for v in vs), max(v.z for v in vs)))
    ext = hi - lo
    k = size / (ext.z if how == 'height' else max(ext.x, ext.y))
    ob.data.transform(Matrix.Translation(Vector((-(lo.x + hi.x) / 2, -(lo.y + hi.y) / 2, -lo.z))))
    ob.data.transform(Matrix.Scale(k, 4))
    for img in bpy.data.images:
        if img.size[0] > 512: img.scale(512, 512)
    n1 = sum(len(p.vertices) - 2 for p in ob.data.polygons)
    ext = ext * k
    report[name] = {'tris_in': n0, 'tris': n1, 'size_m': [round(ext.x, 2), round(ext.y, 2), round(ext.z, 2)], 'front': f}
    ob.name = f'VEH-{name}'
    bpy.ops.export_scene.gltf(filepath=os.path.join(OUT, f'VEH-{name}.glb'), use_selection=False, export_format='GLB',
                              export_image_format='JPEG', export_jpeg_quality=85, export_apply=True)
    if REVIEW:
        S = bpy.context.scene
        S.render.engine = 'BLENDER_EEVEE' if 'BLENDER_EEVEE' in [e.identifier for e in bpy.types.RenderSettings.bl_rna.properties['engine'].enum_items] else 'CYCLES'
        w = bpy.data.worlds.new('w'); w.use_nodes = True; w.node_tree.nodes['Background'].inputs['Color'].default_value = (0.6, 0.65, 0.7, 1); S.world = w
        sun = bpy.data.objects.new('sun', bpy.data.lights.new('sun', 'SUN')); sun.data.energy = 3; sun.rotation_euler = (0.8, 0, 0.5); S.collection.objects.link(sun)
        # a red arrow along the front (-Y)
        bpy.ops.mesh.primitive_cone_add(radius1=size * 0.06, depth=size * 0.2, location=(0, -max(ext.x, ext.y) * 0.75, ext.z * 0.5), rotation=(math.pi / 2, 0, 0))
        ar = bpy.context.active_object; mt = bpy.data.materials.new('r'); mt.diffuse_color = (1, 0, 0, 1); ar.data.materials.append(mt)
        cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam')); S.collection.objects.link(cam); S.camera = cam
        cam.data.type = 'ORTHO'; cam.data.ortho_scale = max(ext) * 1.9
        S.render.resolution_x, S.render.resolution_y = 700, 500
        d = Vector((1, -1.2, 0.8)).normalized()
        cam.location = Vector((0, 0, ext.z / 2)) + d * 100; cam.rotation_euler = (-d).to_track_quat('-Z', 'Y').to_euler(); cam.data.clip_end = 500
        S.render.filepath = os.path.join(OUT, f'review-{name}.png')
        bpy.ops.render.render(write_still=True)
json.dump(report, open(os.path.join(OUT, 'VEHICLES.json'), 'w'), indent=1)
print(json.dumps(report))

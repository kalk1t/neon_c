# Renders a .glb from the six axis directions plus a 3/4 view (Blender coordinates), to check orientation.
# Usage: blender -b --python views.py -- <in.glb> <out_prefix>
import sys, math
import bpy
from mathutils import Vector

argv = sys.argv[sys.argv.index('--') + 1:]
src, out = argv[0], argv[1]
scale = float(argv[2]) if len(argv) > 2 else 2.4
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
sc = bpy.context.scene
sc.render.engine = 'BLENDER_EEVEE'
sc.render.resolution_x = sc.render.resolution_y = 512
w = bpy.data.worlds.new('w'); sc.world = w
w.use_nodes = True
w.node_tree.nodes['Background'].inputs[0].default_value = (0.35, 0.35, 0.4, 1)
w.node_tree.nodes['Background'].inputs[1].default_value = 1.5
sun = bpy.data.objects.new('sun', bpy.data.lights.new('sun', 'SUN'))
sun.data.energy = 4; sun.rotation_euler = (0.6, 0.3, 0.8); sc.collection.objects.link(sun)
cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam'))
cam.data.type = 'ORTHO'; cam.data.ortho_scale = scale
sc.collection.objects.link(cam); sc.camera = cam
views = {'px': (1, 0, 0), 'nx': (-1, 0, 0), 'py': (0, 1, 0), 'ny': (0, -1, 0), 'pz': (0, 0, 1), 'q34': (1, -1, 0.7)}
for name, d in views.items():
    v = Vector(d).normalized()
    cam.location = v * scale * 2
    up = Vector((0, 1, 0)) if name == 'pz' else Vector((0, 0, 1))
    cam.rotation_euler = (-v).to_track_quat('-Z', 'Y').to_euler() if name != 'pz' else (0, 0, 0)
    sc.render.filepath = f'{out}_{name}.png'
    bpy.ops.render.render(write_still=True)
print('DONE')

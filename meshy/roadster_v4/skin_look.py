# Renders a car GLB with another paint texture (a shop skin's base PNG) from four sides, workbench, nose to +Y.
# Usage: blender -b -P skin_look.py -- <car.glb> <base.png> <out prefix>
import sys, bpy, numpy as np
from mathutils import Vector
glb, png, pre = sys.argv[sys.argv.index('--') + 1:][:3]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=glb)
img = bpy.data.images.load(png)
for m in bpy.data.materials:
    if not m.use_nodes: continue
    if m.name.startswith('wheel_') or not any(n.type == 'BSDF_PRINCIPLED' for n in m.node_tree.nodes): continue
    t = m.node_tree.nodes.new('ShaderNodeTexImage'); t.image = img
    m.node_tree.nodes.active = t  # workbench shows the active image node
ms = [o for o in bpy.context.scene.objects if o.type == 'MESH']
for o in ms: o.hide_render = o.name.startswith('lo_')
pts = np.array([list(o.matrix_world @ v.co) for o in ms if not o.hide_render for v in o.data.vertices])
lo, hi = pts.min(0), pts.max(0)
sc = bpy.context.scene
sc.render.engine = 'BLENDER_WORKBENCH'; sc.display.shading.light = 'STUDIO'; sc.display.shading.color_type = 'TEXTURE'
sc.display.shading.show_cavity = True; sc.render.resolution_x, sc.render.resolution_y = 700, 440
cam = bpy.data.objects.new('c', bpy.data.cameras.new('c')); cam.data.lens = 50; sc.collection.objects.link(cam); sc.camera = cam
c = Vector(((lo + hi) / 2).tolist()); L = float(max(hi - lo))
for name, d in (('q34', (-1, 0.8, 0.4)), ('side', (-1, 0, 0.05)), ('rear', (0.6, -1, 0.45)), ('top', (0, 0.01, 1))):
    d = Vector(d).normalized(); cam.location = c + d * L * 1.7
    cam.rotation_euler = d.to_track_quat('Z', 'Y').to_euler()
    sc.render.filepath = f'{pre}_{name}.png'; bpy.ops.render.render(write_still=True)

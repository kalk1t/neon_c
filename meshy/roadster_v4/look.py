# Renders any GLB from four sides (textured, workbench) after normalising its length to 5.6 m, nose to -Y.
# Usage: blender -b --python look.py -- <model.glb> <out prefix>
import sys, math, bpy, numpy as np
from mathutils import Vector, Matrix
a = sys.argv[sys.argv.index('--') + 1:]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=a[0])
ms = [o for o in bpy.context.scene.objects if o.type == 'MESH']
pts = np.array([list(o.matrix_world @ v.co) for o in ms for v in o.data.vertices])
lo, hi = pts.min(0), pts.max(0); print('size', (hi - lo).round(3), 'tris', sum(len(o.data.polygons) for o in ms))
sc = bpy.context.scene
sc.render.engine = 'BLENDER_WORKBENCH'; sc.display.shading.light = 'STUDIO'; sc.display.shading.color_type = 'TEXTURE'
sc.display.shading.show_cavity = True; sc.render.resolution_x, sc.render.resolution_y = 900, 560
cam = bpy.data.objects.new('c', bpy.data.cameras.new('c')); cam.data.lens = 50; sc.collection.objects.link(cam); sc.camera = cam
c = Vector(((lo + hi) / 2).tolist()); L = float(max(hi - lo))
for name, d in (('q34', (-0.8, -1, 0.35)), ('side', (0, -1, 0.02)), ('front', (-1, -0.3, 0.15)), ('rear', (1, -0.3, 0.2)), ('top', (0, 0.01, 1))):
    d = Vector(d).normalized(); cam.location = c + d * L * 1.9
    cam.rotation_euler = d.to_track_quat('Z', 'Y').to_euler() if name != 'top' else (0, 0, 0)
    sc.render.filepath = f'{a[1]}_{name}.png'; bpy.ops.render.render(write_still=True)

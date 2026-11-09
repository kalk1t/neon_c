# Orthographic left-side render of the normalised model with circles drawn at given wheel guesses.
# Usage: blender -b --python sideortho.py -- model.glb out.png  y,z,r  y,z,r ...
import sys, math, bpy, numpy as np
from mathutils import Matrix, Vector
a = sys.argv[sys.argv.index('--') + 1:]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=a[0])
o = [o for o in bpy.context.scene.objects if o.type == 'MESH'][0]
o.data.transform(o.matrix_world); o.matrix_world = Matrix.Identity(4)
o.data.transform(Matrix.Rotation(math.radians(-90), 4, 'Z'))
co = np.empty(len(o.data.vertices) * 3); o.data.vertices.foreach_get('co', co); co = co.reshape(-1, 3)
lo, hi = co.min(0), co.max(0); s = 5.6 / (hi[1] - lo[1])
o.data.transform(Matrix.Scale(s, 4) @ Matrix.Translation(-Vector((lo + hi) / 2)))
for g in a[2:]:
    y, z, r = map(float, g.split(','))
    bpy.ops.mesh.primitive_torus_add(major_radius=r, minor_radius=0.008, location=(-1.8, y, z), rotation=(0, math.pi / 2, 0))
    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.02, location=(-1.8, y, z))
sc = bpy.context.scene
sc.render.engine = 'BLENDER_WORKBENCH'; sc.display.shading.light = 'FLAT'; sc.display.shading.color_type = 'TEXTURE'
sc.render.resolution_x, sc.render.resolution_y = 1600, 600
cam = bpy.data.objects.new('c', bpy.data.cameras.new('c')); cam.data.type = 'ORTHO'; cam.data.ortho_scale = 6
cam.location = (-10, 0, 0); cam.rotation_euler = Vector((-1, 0, 0)).to_track_quat('Z', 'Y').to_euler()
sc.collection.objects.link(cam); sc.camera = cam
sc.render.filepath = a[1]; bpy.ops.render.render(write_still=True)

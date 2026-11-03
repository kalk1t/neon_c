# Bakes orientation/scale for the game: nose towards glTF -Z (Blender +Y), length 5.6 m, centred on the origin.
# Usage: blender -b --python finalize_car.py -- <in.glb> <out.glb>
import sys, math
import bpy
from mathutils import Vector, Matrix

argv = sys.argv[sys.argv.index('--') + 1:]
src, dst = argv[0], argv[1]
LENGTH = 5.6

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
for o in meshes:
    o.data.transform(o.matrix_world)
    o.matrix_world = Matrix.Identity(4)
    o.data.transform(Matrix.Rotation(math.radians(-90), 4, 'Z'))  # Meshy nose -X -> +Y

lo = Vector((1e9,) * 3); hi = Vector((-1e9,) * 3)
for o in meshes:
    for v in o.data.vertices:
        lo = Vector(map(min, lo, v.co)); hi = Vector(map(max, hi, v.co))
s = LENGTH / (hi.y - lo.y)
c = (lo + hi) / 2
for o in meshes:
    o.data.transform(Matrix.Scale(s, 4) @ Matrix.Translation(-c))
    o.data.update()
    o.name = 'car'
size = (hi - lo) * s
print(f'SIZE w={size.x:.2f} l={size.y:.2f} h={size.z:.2f}')

for mat in bpy.data.materials:
    print('MATERIAL', mat.name)
bpy.ops.export_scene.gltf(filepath=dst, export_format='GLB', export_image_format='WEBP', export_image_quality=88)
print('EXPORTED', dst)

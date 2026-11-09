# Orthographic front / rear / left renders of a game car GLB (glTF space: nose -Z) with a 10 cm grid; prints px scale.
# Usage: blender -b --python ortho_ends.py -- car.glb outprefix
import sys, math, bpy
from mathutils import Vector
a = sys.argv[sys.argv.index('--') + 1:]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=a[0])
for o in bpy.data.objects:
    if o.type == 'MESH': o.hide_render = o.name.startswith('lo_')
sc = bpy.context.scene
sc.render.engine = 'BLENDER_WORKBENCH'; sc.display.shading.light = 'STUDIO'; sc.display.shading.color_type = 'TEXTURE'
sc.render.resolution_x = sc.render.resolution_y = 1000
cam = bpy.data.objects.new('c', bpy.data.cameras.new('c')); cam.data.type = 'ORTHO'; cam.data.ortho_scale = 4
sc.collection.objects.link(cam); sc.camera = cam
# grid lines (thin boxes) every 0.1 m in the view plane, bold every 0.5 m
def grid(plane):
    for i in range(-20, 21):
        w = 0.006 if i % 5 else 0.014
        for along in (0, 1):
            if plane == 'xz':
                loc = (i * 0.1, 0, 0) if along == 0 else (0, 0, i * 0.1); size = (w, 0.001, 4) if along == 0 else (4, 0.001, w)
                for yy in (4, -4):
                    bpy.ops.mesh.primitive_cube_add(location=(loc[0], yy, loc[2])); o = bpy.context.object; o.scale = (size[0] / 2, size[1] / 2, size[2] / 2); o.name = 'grid'
            else:
                loc = (0, i * 0.1, 0) if along == 0 else (0, 0, i * 0.1); size = (0.001, w, 4) if along == 0 else (0.001, 6, w)
                bpy.ops.mesh.primitive_cube_add(location=(-4, loc[1], loc[2])); o = bpy.context.object; o.scale = (size[0] / 2, size[1] / 2, size[2] / 2); o.name = 'grid'
grid('xz'); grid('yz')
for name, loc, ortho in (('front', (0, 10, 0), 4), ('rear', (0, -10, 0), 4), ('left', (-10, 0, 0), 6)):
    cam.location = loc; cam.data.ortho_scale = ortho
    d = (Vector((0, 0, 0)) - Vector(loc)).normalized(); cam.rotation_euler = (-d).to_track_quat('Z', 'Y').to_euler()
    for o in bpy.data.objects:
        if o.name.startswith('grid'): o.hide_render = (name == 'left') == (abs(o.location.x) < 3.9 or True and abs(o.location.y) > 3.9)
    sc.render.filepath = f'{a[1]}_{name}.png'; bpy.ops.render.render(write_still=True)

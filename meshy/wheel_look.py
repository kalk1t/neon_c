# v0.3.9: renders each refined car with its pod wheels (game/tools/split-wheels.mjs) in red, frames grey.
# Usage: blender -b --python wheel_look.py -- <out dir>
import os, sys
import bpy
from mathutils import Vector

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[sys.argv.index('--') + 1]
os.makedirs(OUT, exist_ok=True)
for f in ('car', 'car02', 'car03'):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=os.path.join(HERE, 'refined', f + '.glb'))
    for o in bpy.data.objects:
        if o.type != 'MESH': continue
        o.hide_render = o.name.startswith('lo_')
        o.color = (1, 0.1, 0.1, 1) if o.name.endswith('_wheel') else (0.75, 0.75, 0.75, 1)
    sc = bpy.context.scene
    sc.render.engine = 'BLENDER_WORKBENCH'
    sc.display.shading.light = 'STUDIO'
    sc.display.shading.color_type = 'OBJECT'
    sc.display.shading.show_cavity = True
    sc.render.resolution_x, sc.render.resolution_y = 900, 600
    cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam'))
    cam.data.type = 'ORTHO'; cam.data.ortho_scale = 6.5
    sc.collection.objects.link(cam); sc.camera = cam
    for name, d in (('side', Vector((-1, 0, 0))), ('q34', Vector((-1, 1, 0.5)).normalized()), ('under', Vector((-0.3, 0.2, -1)).normalized())):
        cam.location = d * 10
        cam.rotation_euler = d.to_track_quat('Z', 'Y').to_euler()
        sc.render.filepath = os.path.join(OUT, f'{f}_{name}.png')
        bpy.ops.render.render(write_still=True)
    print('DONE', f)

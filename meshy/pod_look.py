# v0.3.9: renders the front-left pod of each refined car (side, front, top) to see its wheel inside the frame.
# Usage: blender -b --python pod_look.py -- <out dir>
import os, sys, math
import bpy
from mathutils import Vector

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[sys.argv.index('--') + 1]
os.makedirs(OUT, exist_ok=True)
for f in ('car', 'car02', 'car03'):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=os.path.join(HERE, 'refined', f + '.glb'))
    pod = next(o for o in bpy.data.objects if o.name.startswith(os.environ.get('POD', 'hi_pod_FL')))
    for o in list(bpy.data.objects):
        if o is not pod and o.type == 'MESH': o.hide_render = True
    c = pod.matrix_world.translation.copy()
    sc = bpy.context.scene
    sc.render.engine = 'BLENDER_WORKBENCH'
    sc.display.shading.light = 'STUDIO'
    sc.display.shading.color_type = 'SINGLE'; sc.display.shading.single_color = (0.8, 0.8, 0.8)
    sc.display.shading.show_cavity = True
    sc.render.resolution_x = sc.render.resolution_y = 700
    cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam'))
    cam.data.type = 'ORTHO'; cam.data.ortho_scale = 2.4
    sc.collection.objects.link(cam); sc.camera = cam
    # glTF import: y up becomes z up; car nose toward -Z (game) = +Y in Blender
    for name, d in (('out', Vector((-1, 0, 0))), ('in', Vector((1, 0, 0))), ('front', Vector((0, 1, 0))), ('top', Vector((0, 0, 1)))):
        cam.location = c + d * 5
        cam.rotation_euler = d.to_track_quat('Z', 'X' if name == 'top' else 'Y').to_euler()
        sc.render.filepath = os.path.join(OUT, f'{f}_{os.environ.get("POD", "hi_pod_FL")}_{name}.png')
        bpy.ops.render.render(write_still=True)
    print('POD', f, pod.name, tuple(round(v, 2) for v in c), pod.dimensions[:])

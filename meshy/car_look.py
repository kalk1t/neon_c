# v0.3.9: renders each refined car (meshy/refined/<file>.glb) textured from a low three-quarter view and the side, to check
# the modelled wheels (build_wheels.py, game/tools/split-wheels.mjs).  Usage: blender -b --python car_look.py -- <out dir>
import os, sys, bpy
from mathutils import Vector
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[sys.argv.index('--') + 1]
for f in ('car', 'car02', 'car03'):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=os.path.join(HERE, 'refined', f + '.glb'))
    for o in bpy.data.objects:
        if o.type == 'MESH': o.hide_render = o.name.startswith('lo_')
    sc = bpy.context.scene
    sc.render.engine = 'BLENDER_WORKBENCH'
    sc.display.shading.light = 'STUDIO'; sc.display.shading.color_type = 'TEXTURE'
    sc.display.shading.show_cavity = True
    sc.render.resolution_x, sc.render.resolution_y = 900, 560
    cam = bpy.data.objects.new('c', bpy.data.cameras.new('c')); cam.data.lens = 50
    sc.collection.objects.link(cam); sc.camera = cam
    for name, d, dist in (('q34', Vector((-1, 0.8, 0.12)).normalized(), 9.5), ('side', Vector((-1, 0, 0.02)).normalized(), 10), ('wheel', Vector((-1, 0.35, 0.05)).normalized(), 3.2)):
        tgt = Vector((-1.4, 1.36, -0.5)) if name == 'wheel' else Vector((0, 0, -0.2))
        cam.location = tgt + d * dist; cam.rotation_euler = d.to_track_quat('Z', 'Y').to_euler()
        sc.render.filepath = os.path.join(OUT, f'{f}_{name}.png'); bpy.ops.render.render(write_still=True)

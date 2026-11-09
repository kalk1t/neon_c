# v0.3.9: flat front / section views of meshy/wheels/<car>_wheels.glb (the hi_L wheel), by material, to check the build.
import os, sys, bpy
from mathutils import Vector
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[sys.argv.index('--') + 1]
for car in ('roadster', 'viper', 'titan'):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=os.path.join(HERE, 'wheels', f'{car}_wheels.glb'))
    for o in bpy.data.objects: o.hide_render = o.name != 'hi_L'
    sc = bpy.context.scene
    sc.render.engine = 'BLENDER_WORKBENCH'
    sc.display.shading.light = 'STUDIO'; sc.display.shading.color_type = 'MATERIAL'
    sc.render.resolution_x = sc.render.resolution_y = 500
    cam = bpy.data.objects.new('c', bpy.data.cameras.new('c')); cam.data.type = 'ORTHO'; cam.data.ortho_scale = 1.1
    sc.collection.objects.link(cam); sc.camera = cam
    for name, d in (('face', Vector((-1, 0, 0))), ('side', Vector((0, 1, 0)))):
        cam.location = d * 3; cam.rotation_euler = d.to_track_quat('Z', 'Y').to_euler()
        sc.render.filepath = os.path.join(OUT, f'{car}_{name}.png'); bpy.ops.render.render(write_still=True)

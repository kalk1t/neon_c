# Renders hi_body with the cockpit glass triangles (game glass.json) in cyan: top, front, driver-eye views.
# Usage: blender -b -P glass_look.py -- <plain car glb> <glass.json> <out prefix>
import sys, json, bpy
from mathutils import Vector
glb, gj, pre = sys.argv[sys.argv.index('--') + 1:][:3]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=glb)
o = bpy.data.objects['hi_body']
tri = set(json.load(open(gj))['triangles'])
me = o.data
print('faces', len(me.polygons))
a = bpy.data.materials.new('a'); a.diffuse_color = (0.5, 0.5, 0.55, 1)
b = bpy.data.materials.new('b'); b.diffuse_color = (0, 1, 1, 1)
me.materials.clear(); me.materials.append(a); me.materials.append(b)
for p in me.polygons: p.material_index = 1 if p.index in tri else 0
sc = bpy.context.scene
sc.render.engine = 'BLENDER_WORKBENCH'; sc.display.shading.color_type = 'MATERIAL'; sc.display.shading.light = 'STUDIO'
sc.render.resolution_x, sc.render.resolution_y = 640, 400
cam = bpy.data.objects.new('c', bpy.data.cameras.new('c')); sc.collection.objects.link(cam); sc.camera = cam
# Blender: x right, y = -glTF z (nose +y), z up
for name, loc, tgt, lens in (('top', (0, 0.3, 9), (0, 0.3, 0), 40), ('front', (0, 8, 1.6), (0, 0, 0.3), 45),
                             ('eye', (-0.36, -0.6, 0.52), (-0.36, 3, 0.35), 18)):
    cam.location = Vector(loc); cam.data.lens = lens
    cam.rotation_euler = (Vector(tgt) - Vector(loc)).to_track_quat('-Z', 'Y').to_euler()
    sc.render.filepath = f'{pre}_{name}.png'; bpy.ops.render.render(write_still=True)

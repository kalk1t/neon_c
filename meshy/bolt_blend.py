# Saves a game bolt (.glb) as a .blend to look at (v0.2.19): textures packed in, glowing in its own colours on a dark
# background, a camera along its side, the viewport in material preview, and the bolt shown 8 m long as in the game.
# Usage: blender -b --python bolt_blend.py -- <bolt.glb> <out.blend>
import sys, math
import bpy

argv = sys.argv[sys.argv.index('--') + 1:]
src, dst = argv[0], argv[1]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
objs = [o for o in bpy.context.scene.objects if o.type == 'MESH']
for o in objs:
    o.scale = (8, 8, 8)  # the game draws it about 8 m long (1.5 car lengths)
    for m in o.data.materials:
        if not m or not m.use_nodes:
            continue
        b = next((n for n in m.node_tree.nodes if n.type == 'BSDF_PRINCIPLED'), None)
        if b:
            b.inputs['Emission Strength'].default_value = 1.5
bpy.ops.file.pack_all()

sc = bpy.context.scene
w = bpy.data.worlds.new('dark'); sc.world = w
w.use_nodes = True
w.node_tree.nodes['Background'].inputs['Color'].default_value = (0.02, 0.015, 0.03, 1)
sc.render.engine = 'BLENDER_EEVEE_NEXT' if 'BLENDER_EEVEE_NEXT' in [e.identifier for e in bpy.types.RenderSettings.bl_rna.properties['engine'].enum_items] else 'BLENDER_EEVEE'
try:
    sc.eevee.use_bloom = True
except Exception:
    pass

cam = bpy.data.objects.new('Camera', bpy.data.cameras.new('Camera'))
sc.collection.objects.link(cam); sc.camera = cam
cam.location = (12, -4, 2)
cam.rotation_euler = (math.radians(82), 0, math.radians(72))
light = bpy.data.objects.new('Light', bpy.data.lights.new('Light', 'SUN'))
light.data.energy = 2
sc.collection.objects.link(light)

# open with material preview so the textures show in the viewport
for scr in bpy.data.screens:
    for a in scr.areas:
        if a.type == 'VIEW_3D':
            for s in a.spaces:
                if s.type == 'VIEW_3D':
                    s.shading.type = 'MATERIAL'
bpy.ops.wm.save_as_mainfile(filepath=dst, compress=True)
print('wrote', dst)

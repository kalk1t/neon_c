# v0.3.9 ROADSTER pilot: bakes the new body's object-space position and normal into its own UV atlas (float16 .npy, 2048, rows top-down),
# so paint_skins.py can paint the shop skins in 3D (stripes, camo, gradients) straight into the texture.
# Usage: blender -b -P bake_pos.py -- <car glb> <out dir>
import os, sys, bpy
glb, out = sys.argv[sys.argv.index('--') + 1:][:2]
os.makedirs(out, exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=glb)
body = next(o for o in bpy.data.objects if o.type == 'MESH' and o.name == 'hi_body')
for o in list(bpy.data.objects):
    if o is not body: bpy.data.objects.remove(o, do_unlink=True)
body.parent = None
bpy.context.view_layer.objects.active = body; body.select_set(True)
bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
m = bpy.data.materials.new('p'); m.use_nodes = True; nt = m.node_tree; nt.nodes.clear()
geo = nt.nodes.new('ShaderNodeNewGeometry'); em = nt.nodes.new('ShaderNodeEmission'); o = nt.nodes.new('ShaderNodeOutputMaterial')
nt.links.new(em.outputs['Emission'], o.inputs['Surface'])
tex = nt.nodes.new('ShaderNodeTexImage'); nt.nodes.active = tex
body.data.materials.clear(); body.data.materials.append(m)
sc = bpy.context.scene; sc.render.engine = 'CYCLES'; sc.cycles.samples = 1; sc.render.bake.margin = 0
for name, sock in (('pos', 'Position'), ('nrm', 'Normal')):
    nt.links.new(geo.outputs[sock], em.inputs['Color'])
    img = bpy.data.images.new(name, 2048, 2048, alpha=True, float_buffer=True)
    img.colorspace_settings.name = 'Non-Color'; img.generated_color = (0, 0, 0, 0)
    tex.image = img
    bpy.ops.object.bake(type='EMIT')
    import numpy as np
    a = np.empty(2048 * 2048 * 4, np.float32); img.pixels.foreach_get(a)
    np.save(os.path.join(out, name + '.npy'), a.reshape(2048, 2048, 4)[::-1].astype(np.float16))  # rows top-down, as in a PNG
    print('baked', name)

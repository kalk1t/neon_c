# Builds a game bolt from a raw Meshy .glb (v0.2.19): one mesh, head towards glTF -Z (Blender +Y), 1 m long (the game
# scales it), centred on the origin, decimated to <tris>, its texture scaled to <tex_px>, and its colour also used as
# emission so the bolt glows in its own colours.
# The head is found as the narrower end along the longest axis (the trail end of the art is wider and wispier); pass
# 'flip' to turn it round if that guess is wrong.
# fan=N (v0.2.19): Meshy makes a light-trail picture into a flat cut-out, a thin sliver seen from behind (the chase
# camera's view). N copies of it turned evenly round the bolt's length (like an arrow's feathers) look the same from the
# side and full from behind. <tris> is per copy.
# Usage: blender -b --python build_game_bolt.py -- <raw.glb> <out.glb> <tris> <tex_px> [flip] [fan=N]
import sys
import bpy, bmesh
from mathutils import Vector, Matrix

argv = sys.argv[sys.argv.index('--') + 1:]
src, dst, tris, tex_px = argv[0], argv[1], int(argv[2]), int(argv[3])
flip = 'flip' in argv[4:]
fan = next((int(a[4:]) for a in argv[4:] if a.startswith('fan=')), 1)

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
bpy.ops.object.select_all(action='DESELECT')
for o in meshes:
    o.select_set(True)
bpy.context.view_layer.objects.active = meshes[0]
if len(meshes) > 1:
    bpy.ops.object.join()
obj = bpy.context.view_layer.objects.active
# bake every parent / object transform into the vertices
obj.parent = None
bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)

def verts():
    return [obj.matrix_world @ v.co for v in obj.data.vertices]

vs = verts()
lo = Vector((min(v[i] for v in vs) for i in range(3)))
hi = Vector((max(v[i] for v in vs) for i in range(3)))
size = hi - lo
axis = max(range(3), key=lambda i: size[i])  # the bolt's length
print('raw size', tuple(round(x, 3) for x in size), 'long axis', 'XYZ'[axis])

# which end is the head: the narrower cross-section within 15% of each end
def spread(sel):
    if not sel:
        return 0
    o = [i for i in range(3) if i != axis]
    return sum((max(v[i] for v in sel) - min(v[i] for v in sel)) for i in o)
L = size[axis]
lo_end = [v for v in vs if v[axis] < lo[axis] + 0.15 * L]
hi_end = [v for v in vs if v[axis] > hi[axis] - 0.15 * L]
head_hi = spread(hi_end) < spread(lo_end)
if flip:
    head_hi = not head_hi
print('head at', 'max' if head_hi else 'min', 'end; spreads lo/hi', round(spread(lo_end), 3), round(spread(hi_end), 3))

# rotate so the long axis runs along Blender +Y with the head at +Y (glTF -Z, the game's forward)
src_dir = Vector((0, 0, 0)); src_dir[axis] = 1 if head_hi else -1
rot = src_dir.rotation_difference(Vector((0, 1, 0))).to_matrix().to_4x4()
centre = (lo + hi) / 2
obj.data.transform(rot @ Matrix.Translation(-centre))
obj.data.transform(Matrix.Scale(1 / L, 4))
obj.data.update()

# decimate
dec = obj.modifiers.new('dec', 'DECIMATE')
dec.ratio = min(1.0, tris / max(1, len(obj.data.polygons)))
bpy.ops.object.modifier_apply(modifier='dec')

# fan: copies turned round the length axis (Blender Y), joined into one mesh
if fan > 1:
    import math
    copies = []
    for i in range(1, fan):
        c = obj.copy(); c.data = obj.data.copy()
        c.data.transform(Matrix.Rotation(math.pi * i / fan, 4, 'Y'))
        bpy.context.scene.collection.objects.link(c)
        copies.append(c)
    bpy.ops.object.select_all(action='DESELECT')
    for o in [obj, *copies]:
        o.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.join()
print('triangles', len(obj.data.polygons), 'fan', fan)

# textures: scale down; base colour doubles as emission (glow)
for mat in obj.data.materials:
    if not mat or not mat.use_nodes:
        continue
    nt = mat.node_tree
    bsdf = next((n for n in nt.nodes if n.type == 'BSDF_PRINCIPLED'), None)
    for n in nt.nodes:
        if n.type == 'TEX_IMAGE' and n.image:
            if n.image.size[0] > tex_px:
                n.image.scale(tex_px, tex_px)
            n.image.pack()
    if bsdf:
        base = bsdf.inputs['Base Color']
        if base.is_linked:
            nt.links.new(base.links[0].from_socket, bsdf.inputs['Emission Color'])
        bsdf.inputs['Emission Strength'].default_value = 1.0

bpy.ops.export_scene.gltf(filepath=dst, export_format='GLB', use_selection=False, export_image_format='WEBP')
print('wrote', dst)

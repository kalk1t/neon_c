"""Living Megacity MC01: facade tiles MAT-01 / MAT-02 / MAT-03 (Blender 5.2, headless).

  blender -b --factory-startup --python facade_tiles.py -- <outdir> [res=512] [family ...]

Each facade family is modelled as real geometry for ONE BAY x ONE FLOOR at true scale (metres), seamless at its
edges, then baked with Cycles onto a flat tile (selected-to-active, the tile plane at y = 0 facing +Y = outwards):

  <fam>_basecolor.png  sRGB colour (the glass's own colour where it is glass)
  <fam>_normal.png     tangent-space normal, OpenGL convention (+Y up), as glTF / three.js expect
  <fam>_orm.png        R = ambient occlusion, G = roughness, B = metalness, A = window mask (1 = glass a room can
                       light up behind; used by the game's occupancy shader)

The game maps UVs in tile units (u = metres / bay width, v = metres / floor height from the building's base), so a
building of any size repeats whole bays and floors and windows are never stretched.
Also writes <fam>_tile.blend (editable source) and <fam>_review.png (neutral-light render of a 4 x 3 tile wall).
"""
import bpy, bmesh, sys, os, json, math
from mathutils import Vector

A = sys.argv[sys.argv.index('--') + 1:]
OUT = os.path.abspath(A[0])  # image saves resolve relative paths against the drive root otherwise
RES = int(A[1]) if len(A) > 1 else 512
WANT = A[2:]
os.makedirs(OUT, exist_ok=True)

# ---------------------------------------------------------------- materials: value-driven so each map can be baked
# from an emission copy (colour, roughness, metal, mask); normal + AO are baked from the real shading.
PAL = {
    # office curtain wall (MAT-01): blue-grey vision glass, darker spandrel glass, clear-anodised aluminium
    'glass_office':   dict(col=(0.055, 0.075, 0.11), rough=0.06, metal=0.0, mask=1.0),
    'spandrel':       dict(col=(0.10, 0.12, 0.15), rough=0.28, metal=0.55, mask=0.0),
    'alu':            dict(col=(0.56, 0.59, 0.63), rough=0.32, metal=1.0, mask=0.0),
    'alu_dark':       dict(col=(0.20, 0.22, 0.25), rough=0.40, metal=0.9, mask=0.0),
    # residential (MAT-02): pale precast concrete, darker joints, warm-grey frames, clear glass
    'concrete_pale':  dict(col=(0.60, 0.58, 0.54), rough=0.86, metal=0.0, mask=0.0),
    'concrete_band':  dict(col=(0.50, 0.49, 0.46), rough=0.82, metal=0.0, mask=0.0),
    'joint':          dict(col=(0.22, 0.21, 0.20), rough=0.9, metal=0.0, mask=0.0),
    'frame_res':      dict(col=(0.28, 0.27, 0.26), rough=0.45, metal=0.7, mask=0.0),
    'glass_res':      dict(col=(0.045, 0.05, 0.055), rough=0.07, metal=0.0, mask=1.0),
    # transport (MAT-03): structural concrete, galvanised steel ribs, dark steel frames, service glass
    'concrete_struct': dict(col=(0.47, 0.47, 0.45), rough=0.84, metal=0.0, mask=0.0),
    'steel_rib':      dict(col=(0.34, 0.36, 0.38), rough=0.42, metal=0.95, mask=0.0),
    'steel_dark':     dict(col=(0.12, 0.13, 0.14), rough=0.5, metal=0.85, mask=0.0),
    'glass_service':  dict(col=(0.05, 0.06, 0.065), rough=0.1, metal=0.0, mask=1.0),
    # office stone (MAT-01B): limestone grid with deep punched windows, bronze frames and spandrel panels
    'limestone':      dict(col=(0.58, 0.54, 0.47), rough=0.78, metal=0.0, mask=0.0),
    'limestone_dark': dict(col=(0.46, 0.43, 0.38), rough=0.82, metal=0.0, mask=0.0),
    'bronze':         dict(col=(0.24, 0.16, 0.09), rough=0.38, metal=0.9, mask=0.0),
    'glass_stone':    dict(col=(0.05, 0.06, 0.075), rough=0.06, metal=0.0, mask=1.0),
}
MATS = {}


def pbr(name):
    if name in MATS:
        return MATS[name]
    p = PAL[name]
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    b = m.node_tree.nodes['Principled BSDF']
    b.inputs['Base Color'].default_value = (*p['col'], 1)
    b.inputs['Roughness'].default_value = p['rough']
    b.inputs['Metallic'].default_value = p['metal']
    MATS[name] = m
    return m


def set_emission_mode(mode):
    """mode None = real PBR; else every high-poly material emits the chosen value (col / rough / metal / mask)."""
    for name, m in MATS.items():
        nt = m.node_tree
        out = nt.nodes['Material Output']
        bsdf = nt.nodes['Principled BSDF']
        em = nt.nodes.get('BakeEmit') or nt.nodes.new('ShaderNodeEmission')
        em.name = 'BakeEmit'
        for l in list(out.inputs['Surface'].links):
            nt.links.remove(l)
        if mode is None:
            nt.links.new(bsdf.outputs['BSDF'], out.inputs['Surface'])
            continue
        p = PAL[name]
        v = p['col'] if mode == 'col' else (p[mode],) * 3
        em.inputs['Color'].default_value = (*v, 1)
        em.inputs['Strength'].default_value = 1.0
        nt.links.new(em.outputs['Emission'], out.inputs['Surface'])


# ---------------------------------------------------------------- geometry helpers (x across the tile, z up the
# floor, y = depth: + out of the wall, - into it). Everything lands in one high-poly object per family.
PARTS = []


def box(x0, x1, z0, z1, y0, y1, mat):
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for v in bm.verts:
        v.co = Vector(((x0 + x1) / 2 + v.co.x * (x1 - x0), (y0 + y1) / 2 + v.co.y * (y1 - y0), (z0 + z1) / 2 + v.co.z * (z1 - z0)))
    me = bpy.data.meshes.new('part')
    bm.to_mesh(me)
    bm.free()
    me.materials.append(pbr(mat))
    o = bpy.data.objects.new('part', me)
    bpy.context.scene.collection.objects.link(o)
    bev = o.modifiers.new('bev', 'BEVEL')
    bev.width = min(0.012, (x1 - x0) / 4, (z1 - z0) / 4, (y1 - y0) / 4)
    bev.segments = 2
    PARTS.append(o)
    return o


def wrap_box(x0, x1, z0, z1, y0, y1, mat, W, H):
    """A box that may cross the tile edge: drawn again shifted by the tile size so the bake is seamless."""
    for dx in (-W, 0, W):
        for dz in (-H, 0, H):
            if x1 + dx <= 0 or x0 + dx >= W or z1 + dz <= 0 or z0 + dz >= H:
                continue
            box(x0 + dx, x1 + dx, z0 + dz, z1 + dz, y0, y1, mat)


# ---------------------------------------------------------------- the three families
def office():
    """MAT-01: 3.0 m bay (two 1.5 m panels) x 4.0 m office floor. Vision glass 0.9-3.75 m, opaque spandrel across
    the slab (3.75 -> 0.9 of the next floor), narrow aluminium mullions, a transom at 3.1 m."""
    W, H = 3.0, 4.0
    box(0, W, 0, H, -0.16, -0.12, 'glass_office')                         # glass plane (recessed)
    wrap_box(0, W, -0.25, 0.9, -0.10, -0.02, 'spandrel', W, H)            # spandrel: head (3.75) -> sill (0.9) via the wrap
    for x in (0.0, 1.5):                                                  # vertical mullions (the one at 0 wraps to 3)
        wrap_box(x - 0.045, x + 0.045, 0, H, -0.12, 0.09, 'alu', W, H)
    for z in (0.9, 3.75):                                                 # sill + head transoms
        wrap_box(0, W, z - 0.04, z + 0.04, -0.12, 0.06, 'alu', W, H)
    wrap_box(0, W, 3.08, 3.12, -0.12, 0.02, 'alu_dark', W, H)             # transom
    wrap_box(0, W, -0.06, 0.06, -0.12, 0.12, 'alu', W, H)                 # slab-edge cap (floor line)
    return W, H


def residential():
    """MAT-02: 3.4 m bay x 3.4 m floor. Pale precast panel with a punched window (2.0 x 1.6 m), recessed frame,
    projecting sill, a slab band at the floor line and a vertical panel joint at the bay edge."""
    W, H = 3.4, 3.4
    ox0, ox1, oz0, oz1 = 0.7, 2.7, 0.95, 2.6
    # panel around the opening
    box(0, ox0, 0, H, -0.25, 0.0, 'concrete_pale')
    box(ox1, W, 0, H, -0.25, 0.0, 'concrete_pale')
    box(ox0, ox1, 0, oz0, -0.25, 0.0, 'concrete_pale')
    box(ox0, ox1, oz1, H, -0.25, 0.0, 'concrete_pale')
    # reveal (opening depth) and frame
    box(ox0, ox1, oz0, oz1, -0.24, -0.20, 'glass_res')
    for x0, x1 in ((ox0, ox0 + 0.07), (ox1 - 0.07, ox1), ((ox0 + ox1) / 2 - 0.03, (ox0 + ox1) / 2 + 0.03)):
        box(x0, x1, oz0, oz1, -0.21, -0.14, 'frame_res')
    for z0, z1 in ((oz0, oz0 + 0.07), (oz1 - 0.07, oz1)):
        box(ox0, ox1, z0, z1, -0.21, -0.14, 'frame_res')
    box(ox0 - 0.08, ox1 + 0.08, oz0 - 0.07, oz0, -0.05, 0.07, 'concrete_band')   # sill
    wrap_box(0, W, -0.13, 0.13, -0.02, 0.06, 'concrete_band', W, H)                # slab band (floor line)
    wrap_box(-0.012, 0.012, 0, H, -0.3, 0.002, 'joint', W, H)             # panel joint
    return W, H


def transport():
    """MAT-03: 6.0 m structural bay x 4.5 m floor. Concrete columns at the bay edges, a ribbed steel service panel
    below 1.2 m, a large steel-framed opening, and a concrete beam band across the slab."""
    W, H = 6.0, 4.5
    wrap_box(-0.4, 0.4, 0, H, -0.1, 0.3, 'concrete_struct', W, H)          # column
    wrap_box(0, W, 3.95, 4.5001, -0.1, 0.16, 'concrete_struct', W, H)               # beam band
    box(0.4, W - 0.4, 0, 1.2, -0.2, -0.16, 'steel_dark')                            # service panel back
    n = int((W - 0.8) / 0.2)
    for k in range(n):                                                              # vertical ribs
        x = 0.4 + (k + 0.5) * (W - 0.8) / n
        box(x - 0.05, x + 0.05, 0.02, 1.18, -0.17, -0.07, 'steel_rib')
    box(0.4, W - 0.4, 1.2, 1.28, -0.2, -0.04, 'steel_dark')                        # rail
    box(0.4, W - 0.4, 1.28, 3.95, -0.24, -0.2, 'glass_service')                     # opening glass
    for k in range(1, 4):                                                           # frame grid
        x = 0.4 + k * (W - 0.8) / 4
        box(x - 0.05, x + 0.05, 1.28, 3.95, -0.2, -0.08, 'steel_dark')
    box(0.4, W - 0.4, 2.62, 2.7, -0.2, -0.08, 'steel_dark')
    return W, H


def office_stone():
    """MAT-01B: the same 3.0 m bay x 4.0 m office floor as MAT-01, as a limestone grid (prewar / Rockefeller-era
    office towers): two tall punched windows per bay (1.0 x 2.6 m) in 0.3 m deep reveals, bronze frames with a transom,
    a bronze spandrel panel under each window, and a projecting fluted pier on the bay line."""
    W, H = 3.0, 4.0
    wins = ((0.25, 1.25), (1.75, 2.75))
    z0, z1 = 0.85, 3.45
    xs = [0.0] + [v for w in wins for v in w] + [W]
    for a, b in zip(xs[0::2], xs[1::2]):                                  # stone between the windows (full height)
        box(a, b, 0, H, -0.3, 0.0, 'limestone')
    for a, b in wins:
        box(a, b, 0, z0, -0.3, 0.0, 'limestone')                          # under the window
        box(a, b, z1, H, -0.3, 0.0, 'limestone')                          # over the window
        box(a, b, z0, z1, -0.34, -0.3, 'glass_stone')                     # glass deep in the reveal
        for x0, x1 in ((a, a + 0.06), (b - 0.06, b), ((a + b) / 2 - 0.025, (a + b) / 2 + 0.025)):
            box(x0, x1, z0, z1, -0.31, -0.24, 'bronze')
        for zz in (z0, 2.75, z1 - 0.06):
            box(a, b, zz, zz + 0.06, -0.31, -0.24, 'bronze')
        box(a + 0.1, b - 0.1, 0.2, 0.7, -0.06, -0.02, 'bronze')            # spandrel panel (recessed)
        box(a - 0.06, b + 0.06, z0 - 0.08, z0, -0.02, 0.06, 'limestone_dark')   # sill
    wrap_box(-0.2, 0.2, 0, H, -0.02, 0.1, 'limestone', W, H)              # pier on the bay line
    for k in (-1, 0, 1):                                                  # its flutes
        wrap_box(k * 0.1 - 0.02, k * 0.1 + 0.02, 0, H, 0.1, 0.115, 'limestone_dark', W, H)
    wrap_box(0, W, -0.08, 0.08, -0.02, 0.08, 'limestone_dark', W, H)      # string course at the floor line
    return W, H


FAMILIES = {'office': office, 'residential': residential, 'transport': transport, 'office_stone': office_stone}


# ---------------------------------------------------------------- bake
def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    PARTS.clear()
    MATS.clear()
    s = bpy.context.scene
    s.render.engine = 'CYCLES'
    s.cycles.device = 'CPU'
    s.cycles.samples = 16
    s.render.bake.margin = 0  # the tile repeats: no padding bleeding across the seam


def bake_family(fam):
    reset()
    W, H = FAMILIES[fam]()
    # join the parts (modifiers applied) into the high-poly
    bpy.ops.object.select_all(action='DESELECT')
    for o in PARTS:
        o.select_set(True)
    bpy.context.view_layer.objects.active = PARTS[0]
    bpy.ops.object.convert(target='MESH')
    bpy.ops.object.join()
    hi = bpy.context.view_layer.objects.active
    hi.name = f'{fam}_tile_high'
    # the low tile: a plane x [0, W], z [0, H] at y = 0 facing +Y, UV = the unit square
    me = bpy.data.meshes.new('tile')
    bm = bmesh.new()
    vs = [bm.verts.new(c) for c in ((0, 0, 0), (W, 0, 0), (W, 0, H), (0, 0, H))]
    f = bm.faces.new(vs)
    f.normal_update()
    if f.normal.y < 0:
        f.normal_flip()
    uv = bm.loops.layers.uv.new('UVMap')
    for l in f.loops:
        l[uv].uv = (l.vert.co.x / W, l.vert.co.z / H)
    bm.to_mesh(me)
    bm.free()
    lo = bpy.data.objects.new(f'{fam}_tile', me)
    bpy.context.scene.collection.objects.link(lo)
    mat = bpy.data.materials.new('bake_target')
    mat.use_nodes = True
    me.materials.append(mat)
    imgs = {}

    def target(key, color=False):
        img = bpy.data.images.new(f'{fam}_{key}', RES, RES, alpha=False, float_buffer=False)
        img.colorspace_settings.name = 'sRGB' if color else 'Non-Color'
        node = mat.node_tree.nodes.new('ShaderNodeTexImage')
        node.image = img
        for n in mat.node_tree.nodes:
            n.select = False
        node.select = True
        mat.node_tree.nodes.active = node
        imgs[key] = img
        return img

    def do_bake(kind, **kw):
        bpy.ops.object.select_all(action='DESELECT')
        hi.select_set(True)
        lo.select_set(True)
        bpy.context.view_layer.objects.active = lo
        bpy.ops.object.bake(type=kind, use_selected_to_active=True, cage_extrusion=0.6, max_ray_distance=1.2,
                            margin=2, margin_type='EXTEND', use_clear=True, **kw)  # 2 px: no black unbaked seam

    s = bpy.context.scene
    set_emission_mode(None)
    target('normal')
    s.cycles.samples = 4
    do_bake('NORMAL', normal_space='TANGENT')
    target('ao')
    s.cycles.samples = 64
    if s.world is None:
        s.world = bpy.data.worlds.new('bake')
    s.world.light_settings.distance = 0.35  # crevice-scale only: the game supplies the large-scale shading
    do_bake('AO')
    s.cycles.samples = 1
    for key, mode, color in (('basecolor', 'col', True), ('rough', 'rough', False), ('metal', 'metal', False), ('mask', 'mask', False)):
        set_emission_mode(mode)
        target(key, color)
        do_bake('EMIT')
    set_emission_mode(None)

    # pack ORM + mask (R = AO, G = roughness, B = metal, A = window mask) and save
    import numpy as np
    def px(img):
        a = np.empty(RES * RES * 4, dtype=np.float32)
        img.pixels.foreach_get(a)
        return a.reshape(-1, 4)
    ao, ro, me_, mk = px(imgs['ao']), px(imgs['rough']), px(imgs['metal']), px(imgs['mask'])
    orm = bpy.data.images.new(f'{fam}_orm', RES, RES, alpha=True)
    orm.colorspace_settings.name = 'Non-Color'
    packed = np.stack([ao[:, 0], ro[:, 0], me_[:, 0], mk[:, 0]], 1).ravel()
    orm.pixels.foreach_set(packed)
    for key, img in (('basecolor', imgs['basecolor']), ('normal', imgs['normal']), ('orm', orm)):
        img.filepath_raw = os.path.join(OUT, f'{fam}_{key}.png')
        img.file_format = 'PNG'
        img.save()
    # neutral review: the baked tile as the game draws it, 4 x 3
    review(fam, W, H, imgs, orm)
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT, f'{fam}_tile.blend'))
    stats = dict(family=fam, bay_m=W, floor_m=H, res=RES, tris_high=sum(len(p.vertices) - 2 for p in hi.data.polygons),
                 mask_cover=round(float(mk[:, 0].mean()), 3), metal_mean=round(float(me_[:, 0].mean()), 3),
                 rough_mean=round(float(ro[:, 0].mean()), 3))
    print('TILE', json.dumps(stats))
    return stats


def review(fam, W, H, imgs, orm):
    """What the game draws: the baked low tile repeated 4 x 3 (one plane, UVs 0..4 x 0..3) with base colour, normal
    and ORM, under a neutral sky and a soft key light. Window cells glow where the mask says glass (half of them),
    approximating the occupancy shader."""
    s = bpy.context.scene
    me = bpy.data.meshes.new('wall')
    bm = bmesh.new()
    vs = [bm.verts.new(c) for c in ((0, 0, 0), (4 * W, 0, 0), (4 * W, 0, 3 * H), (0, 0, 3 * H))]
    f = bm.faces.new(vs)
    f.normal_update()
    if f.normal.y < 0:
        f.normal_flip()
    uv = bm.loops.layers.uv.new('UVMap')
    for l in f.loops:
        l[uv].uv = (l.vert.co.x / W, l.vert.co.z / H)
    bm.to_mesh(me)
    bm.free()
    wall = bpy.data.objects.new('wall', me)
    s.collection.objects.link(wall)
    m = bpy.data.materials.new('review')
    m.use_nodes = True
    nt = m.node_tree
    b = nt.nodes['Principled BSDF']
    def tex(img):
        n = nt.nodes.new('ShaderNodeTexImage')
        n.image = img
        n.interpolation = 'Cubic'
        return n
    bc, nm, om = tex(imgs['basecolor']), tex(imgs['normal']), tex(orm)
    sep = nt.nodes.new('ShaderNodeSeparateColor')
    nt.links.new(om.outputs['Color'], sep.inputs['Color'])
    mul = nt.nodes.new('ShaderNodeMix')
    mul.data_type, mul.blend_type = 'RGBA', 'MULTIPLY'
    mul.inputs['Factor'].default_value = 1.0
    nt.links.new(bc.outputs['Color'], mul.inputs['A'])
    nt.links.new(sep.outputs['Red'], mul.inputs['B'])
    nt.links.new(mul.outputs['Result'], b.inputs['Base Color'])
    nt.links.new(sep.outputs['Green'], b.inputs['Roughness'])
    nt.links.new(sep.outputs['Blue'], b.inputs['Metallic'])
    nmap = nt.nodes.new('ShaderNodeNormalMap')
    nt.links.new(nm.outputs['Color'], nmap.inputs['Color'])
    nt.links.new(nmap.outputs['Normal'], b.inputs['Normal'])
    # lit windows: mask x a per-cell on/off (checker of whole cells), warm light
    uvn = nt.nodes.new('ShaderNodeUVMap')
    chk = nt.nodes.new('ShaderNodeTexChecker')
    chk.inputs['Scale'].default_value = 1.0
    chk.inputs['Color1'].default_value = (1, 1, 1, 1)
    chk.inputs['Color2'].default_value = (0, 0, 0, 1)
    vmap = nt.nodes.new('ShaderNodeVectorMath')
    vmap.operation = 'MULTIPLY'
    vmap.inputs[1].default_value = (0.5, 0.5, 1)
    nt.links.new(uvn.outputs['UV'], vmap.inputs[0])
    nt.links.new(vmap.outputs['Vector'], chk.inputs['Vector'])
    lit = nt.nodes.new('ShaderNodeMath')
    lit.operation = 'MULTIPLY'
    nt.links.new(sep.outputs['Alpha'] if 'Alpha' in sep.outputs else om.outputs['Alpha'], lit.inputs[0])
    nt.links.new(chk.outputs['Fac'], lit.inputs[1])
    b.inputs['Emission Color'].default_value = (1.0, 0.72, 0.45, 1)
    nt.links.new(lit.outputs['Value'], b.inputs['Emission Strength'])
    me.materials.append(m)
    world = bpy.data.worlds.get('neutral') or bpy.data.worlds.new('neutral')
    world.use_nodes = True
    world.node_tree.nodes['Background'].inputs['Color'].default_value = (0.45, 0.47, 0.5, 1)
    world.node_tree.nodes['Background'].inputs['Strength'].default_value = 0.6
    s.world = world
    sun = bpy.data.objects.new('key', bpy.data.lights.new('key', 'SUN'))
    sun.data.energy = 3.0
    sun.data.angle = math.radians(8)
    sun.rotation_euler = (math.radians(60), 0, math.radians(35))
    s.collection.objects.link(sun)
    cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam'))
    s.collection.objects.link(cam)
    cx, cz = 2 * W, 1.5 * H
    cam.location = (cx + 0.35 * W * 4, 2.6 * H + 2.0 * W, cz + 0.3 * H)
    cam.rotation_euler = (Vector((cx, 0, cz)) - cam.location).to_track_quat('-Z', 'Y').to_euler()
    cam.data.lens = 35
    s.camera = cam
    s.render.resolution_x, s.render.resolution_y = 960, 640
    s.cycles.samples = 48
    s.render.filepath = os.path.join(OUT, f'{fam}_review.png')
    hide = [o for o in s.collection.objects if o.type == 'MESH' and o is not wall]
    for o in hide:
        o.hide_render = True
    bpy.ops.render.render(write_still=True)
    for o in hide:
        o.hide_render = False
    for o in (wall, sun, cam):
        bpy.data.objects.remove(o)


results = [bake_family(f) for f in (WANT or FAMILIES)]
with open(os.path.join(OUT, 'tiles.json'), 'w') as fh:
    json.dump(results, fh, indent=1)

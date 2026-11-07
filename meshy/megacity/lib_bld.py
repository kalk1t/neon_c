"""Living Megacity: shared building blocks for the BLD-xx builders (Blender 5.2, headless).

Conventions (as BLD-01, bld01_office.py): metres, origin = bottom-centre of the ground floor, Blender Z up, front = -Y
(glTF: Y up, front +Z). Facade boxes carry UVs in tile units (u along the face from its left corner as seen from
outside, v = (z - z_base) / floor), z_base = the first residential / office floor for EVERY stage, so the game's
floor index (interiors.js, facades.js uReal) runs through all setbacks. Facade materials keep the contract names
(MAT-01 / MAT-02 / MAT-03), the game swaps its own material in; all other materials ship as PBR factors.
"""
import bpy, bmesh, os, json, math
from mathutils import Vector

S = None
M = {}


def start():
    global S
    bpy.ops.wm.read_factory_settings(use_empty=True)
    S = bpy.context.scene
    M.clear()
    return S


def mat(name, col, rough, metal, emit=None, emit_k=0.0, alpha=None):
    if name in M:
        return M[name]
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    b = m.node_tree.nodes['Principled BSDF']
    b.inputs['Base Color'].default_value = (*col, 1)
    b.inputs['Roughness'].default_value = rough
    b.inputs['Metallic'].default_value = metal
    if emit:
        b.inputs['Emission Color'].default_value = (*emit, 1)
        b.inputs['Emission Strength'].default_value = emit_k
    if alpha is not None:
        b.inputs['Alpha'].default_value = alpha
        m.surface_render_method = 'BLENDED'     # glTF alphaMode BLEND
    M[name] = m
    return m


def common_mats():
    for a in [
        ('MAT-01', (0.1, 0.12, 0.15), 0.2, 0.5), ('MAT-02', (0.6, 0.58, 0.54), 0.85, 0.0), ('MAT-03', (0.47, 0.47, 0.45), 0.84, 0.0),
        ('concrete_struct', (0.50, 0.49, 0.47), 0.82, 0.0), ('concrete_pale', (0.62, 0.6, 0.56), 0.85, 0.0),
        ('louvre_dark', (0.11, 0.12, 0.13), 0.45, 0.85), ('roof_metal', (0.42, 0.44, 0.46), 0.4, 0.9),
        ('steel_dark', (0.12, 0.13, 0.14), 0.5, 0.85), ('steel_rail', (0.3, 0.32, 0.34), 0.35, 0.9),
        ('wood_deck', (0.36, 0.25, 0.16), 0.7, 0.0), ('soil', (0.12, 0.09, 0.06), 0.95, 0.0),
        ('planter', (0.2, 0.2, 0.2), 0.6, 0.0), ('tank_wood', (0.33, 0.24, 0.17), 0.8, 0.0),
        ('fin_alu', (0.55, 0.58, 0.62), 0.32, 0.9), ('hazard', (0.85, 0.65, 0.1), 0.5, 0.0),
        ('MAT-01B', (0.58, 0.54, 0.47), 0.78, 0.0), ('limestone', (0.58, 0.54, 0.47), 0.78, 0.0),
        ('limestone_dark', (0.46, 0.43, 0.38), 0.82, 0.0), ('bronze', (0.24, 0.16, 0.09), 0.38, 0.9),
        ('MAT-05', (0.6, 0.36, 0.29), 0.35, 0.0), ('granite', (0.6, 0.36, 0.29), 0.35, 0.0),
        ('granite_dark', (0.4, 0.22, 0.17), 0.4, 0.0), ('pool_tile', (0.75, 0.78, 0.8), 0.4, 0.0),
        ('MAT-07', (0.78, 0.77, 0.74), 0.6, 0.0), ('concrete_white', (0.78, 0.77, 0.74), 0.6, 0.0),
    ]:
        mat(*a)
    mat('leaf', (0.06, 0.2, 0.06), 0.7, 0.0, (0.3, 0.6, 0.2), 0.04)
    mat('lamp_warm', (1.0, 0.85, 0.6), 0.5, 0.0, (1.0, 0.75, 0.45), 2.5)
    mat('lamp_sodium', (1.0, 0.7, 0.3), 0.5, 0.0, (1.0, 0.55, 0.15), 3.0)
    mat('lamp_cool', (0.85, 0.92, 1.0), 0.5, 0.0, (0.85, 0.92, 1.0), 2.5)
    mat('lamp_dim', (0.75, 0.82, 0.9), 0.5, 0.0, (0.75, 0.85, 1.0), 0.6)
    mat('uplight', (1.0, 0.85, 0.65), 0.5, 0.0, (1.0, 0.8, 0.55), 1.6)
    mat('lantern', (1.0, 0.85, 0.6), 0.4, 0.0, (1.0, 0.78, 0.5), 0.7)
    mat('red_light', (1.0, 0.1, 0.05), 0.5, 0.0, (1.0, 0.08, 0.04), 5.0)
    mat('string_light', (1.0, 0.8, 0.5), 0.5, 0.0, (1.0, 0.75, 0.4), 4.0)
    mat('rail_glass', (0.6, 0.7, 0.72), 0.05, 0.0, alpha=0.25)
    mat('lobby_glass', (0.05, 0.055, 0.06), 0.05, 0.0, alpha=0.22)
    mat('lobby_stone', (0.62, 0.58, 0.52), 0.3, 0.0, (1.0, 0.8, 0.6), 0.06)
    mat('lobby_ceiling', (0.75, 0.74, 0.72), 0.8, 0.0, (1.0, 0.85, 0.7), 0.1)
    mat('opening_dark', (0.02, 0.02, 0.025), 0.9, 0.0)
    mat('pool_water', (0.1, 0.45, 0.55), 0.05, 0.0, (0.2, 0.75, 0.95), 0.9)
    mat('bar_glow', (1.0, 0.8, 0.6), 0.5, 0.0, (1.0, 0.7, 0.4), 1.1)


def block(name, x0, x1, y0, y1, z0, z1, m, uv_tile=None, coll=None):
    """An axis-aligned box; uv_tile = (bay, floor, z_base) gives its side faces facade UVs in tile units."""
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for v in bm.verts:
        v.co = Vector(((x0 + x1) / 2 + v.co.x * (x1 - x0), (y0 + y1) / 2 + v.co.y * (y1 - y0), (z0 + z1) / 2 + v.co.z * (z1 - z0)))
    uv = bm.loops.layers.uv.new('UVMap')
    for f in bm.faces:
        n = f.normal
        for l in f.loops:
            c = l.vert.co
            if uv_tile is None or abs(n.z) > 0.5:
                l[uv].uv = (0.02, 0.02)
                continue
            bay, flo, zb = uv_tile
            if abs(n.y) > 0.5:
                u = (c.x - x0) if n.y < 0 else (x1 - c.x)
            else:
                u = (y1 - c.y) if n.x < 0 else (c.y - y0)
            l[uv].uv = (u / bay, (c.z - zb) / flo)
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    me.materials.append(M[m])
    o = bpy.data.objects.new(name, me)
    (coll or S.collection).objects.link(o)
    return o


def rblock(name, hx, hy, cy, r, z0, z1, m, uv_tile=None, seg=8, caps=True, coll=None):
    """A rectangle with rounded corners (centre x = 0, y = cy; half sizes hx, hy; corner radius r), extruded z0..z1.
    Side UVs in tile units run along the outline by arc length from the left end of the front (-y) straight, counter-
    clockwise seen from above (= left to right seen from outside), so bays continue round the curves: choose r so a
    quarter arc (r * pi / 2) is whole bays. Sides are smooth-shaded (tagged for join), caps flat."""
    pts = [(-hx + r, cy - hy)]
    for (ox, oy, a0) in ((hx - r, cy - hy + r, -90), (hx - r, cy + hy - r, 0), (-hx + r, cy + hy - r, 90), (-hx + r, cy - hy + r, 180)):
        for k in range(seg + 1):
            a = math.radians(a0 + 90 * k / seg)
            pts.append((ox + r * math.cos(a), oy + r * math.sin(a)))
    pts = pts[:-1]                                             # the last point is the first one
    us = [0.0]
    for i in range(len(pts)):
        a, b = pts[i], pts[(i + 1) % len(pts)]
        us.append(us[-1] + math.hypot(b[0] - a[0], b[1] - a[1]))
    bm = bmesh.new()
    uv = bm.loops.layers.uv.new('UVMap')
    sm = bm.faces.layers.int.new('smooth')
    lo = [bm.verts.new((x, y, z0)) for x, y in pts]
    hi = [bm.verts.new((x, y, z1)) for x, y in pts]
    n = len(pts)
    for i in range(n):
        j = (i + 1) % n
        if math.hypot(pts[j][0] - pts[i][0], pts[j][1] - pts[i][1]) < 1e-6:
            continue
        f = bm.faces.new((lo[i], lo[j], hi[j], hi[i]))
        f[sm] = 1
        for l, (u, z) in zip(f.loops, ((us[i], z0), (us[i + 1], z0), (us[i + 1], z1), (us[i], z1))):
            if uv_tile:
                bay, flo, zb = uv_tile
                l[uv].uv = (u / bay, (z - zb) / flo)
            else:
                l[uv].uv = (0.02, 0.02)
    if caps:
        for ring_, flip in ((hi, False), (lo, True)):
            f = bm.faces.new(list(reversed(ring_)) if flip else ring_)
            for l in f.loops:
                l[uv].uv = (0.02, 0.02)
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    me.materials.append(M[m])
    o = bpy.data.objects.new(name, me)
    (coll or S.collection).objects.link(o)
    return o


def cyl(name, x, y, z0, z1, r, m, n=12, coll=None):
    bm = bmesh.new()
    bmesh.ops.create_cone(bm, cap_ends=True, segments=n, radius1=r, radius2=r, depth=z1 - z0)
    for v in bm.verts:
        v.co = Vector((v.co.x + x, v.co.y + y, v.co.z + (z0 + z1) / 2))
    uv = bm.loops.layers.uv.new('UVMap')
    for f in bm.faces:
        for l in f.loops:
            l[uv].uv = (0.02, 0.02)
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    me.materials.append(M[m])
    o = bpy.data.objects.new(name, me)
    (coll or S.collection).objects.link(o)
    return o


def ring(parts, name, x0, x1, y0, y1, z0, z1, t, m, c, sides='nsew'):
    """Four thin walls on a rectangle's edges (parapets, railings). sides: which of n(+y) s(-y) e(+x) w(-x)."""
    if 's' in sides: parts.append(block(name, x0, x1, y0, y0 + t, z0, z1, m, coll=c))
    if 'n' in sides: parts.append(block(name, x0, x1, y1 - t, y1, z0, z1, m, coll=c))
    if 'w' in sides: parts.append(block(name, x0, x0 + t, y0, y1, z0, z1, m, coll=c))
    if 'e' in sides: parts.append(block(name, x1 - t, x1, y0, y1, z0, z1, m, coll=c))


def tree(parts, x, y, z, h, c):
    parts.append(block('pl', x - 0.5, x + 0.5, y - 0.5, y + 0.5, z, z + 0.7, 'planter', coll=c))
    parts.append(cyl('trunk', x, y, z + 0.7, z + 0.7 + h * 0.45, 0.08, 'tank_wood', 6, coll=c))
    parts.append(cyl('crown', x, y, z + 0.7 + h * 0.35, z + 0.7 + h, h * 0.32, 'leaf', 8, coll=c))


def join(parts, name):
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:
        o.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    o = bpy.context.view_layer.objects.active
    o.name = o.data.name = name
    bm = bmesh.new()
    bm.from_mesh(o.data)
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-4)
    bm.to_mesh(o.data)
    bm.free()
    sm = o.data.attributes.get('smooth')                     # rblock sides stay smooth, the rest is flat
    flags = [0] * len(o.data.polygons)
    if sm:
        sm.data.foreach_get('value', flags)
    for p, f in zip(o.data.polygons, flags):
        p.use_smooth = bool(f)
    if sm:
        o.data.attributes.remove(sm)
    return o


def socket(name, loc, coll):
    e = bpy.data.objects.new(name, None)
    e.empty_display_size = 1.5
    e.location = loc
    coll.objects.link(e)
    return e


def build(asset, out, lods, results, sockets, review):
    """lods: {'near': fn(coll) -> parts, 'mid': ..., 'far': ...}; sockets: fn(lod, coll) -> [empties];
    review: dict(tiles=dir, tile=fam, mat='MAT-02', cam=(x, y, z), look=(x, y, z)). Writes <asset>_<lod>.glb,
    <asset>.json, <asset>.blend, <asset>_review.png."""
    os.makedirs(out, exist_ok=True)
    done = {}
    results['lods'] = {}
    for lod, fn in lods.items():
        c = bpy.data.collections.new(f'{asset}_{lod}')
        S.collection.children.link(c)
        obj = join(fn(c), f'{asset}_{lod}')
        socks = sockets(lod, c)
        done[lod] = (c, obj)
        tris = sum(len(p.vertices) - 2 for p in obj.data.polygons)
        bb = [obj.matrix_world @ Vector(v) for v in obj.bound_box]
        results['lods'][lod] = {'file': f'{asset}_{lod}.glb', 'triangles': tris,
                                'materials': sorted({s.material.name for s in obj.material_slots}),
                                'bounds_blender_m': [[round(min(v[i] for v in bb), 3) for i in range(3)],
                                                     [round(max(v[i] for v in bb), 3) for i in range(3)]],
                                'sockets': {s.name.split('.')[0]: [round(v, 3) for v in s.location] for s in socks}}
    for lod, (c, obj) in done.items():
        for l2, (c2, o2) in done.items():
            c2.hide_viewport = c2.hide_render = (l2 != lod)
        bpy.ops.object.select_all(action='DESELECT')
        for o in c.objects:
            o.select_set(True)
        bpy.ops.export_scene.gltf(filepath=os.path.join(out, f'{asset}_{lod}.glb'), export_format='GLB', use_selection=True,
                                  export_yup=True, export_apply=True, export_image_format='NONE', export_animations=False,
                                  export_extras=False)
    # review render: near form, the baked facade tile on the facade material, neutral light
    for l2, (c2, o2) in done.items():
        c2.hide_viewport = c2.hide_render = (l2 != 'near')
    nt = M[review['mat']].node_tree
    b = nt.nodes['Principled BSDF']
    def img(file, srgb):
        n = nt.nodes.new('ShaderNodeTexImage')
        n.image = bpy.data.images.load(os.path.join(review['tiles'], file))
        n.image.colorspace_settings.name = 'sRGB' if srgb else 'Non-Color'
        return n
    fam = review['tile']
    bc, nm, orm = img(f'{fam}_basecolor.png', True), img(f'{fam}_normal.png', False), img(f'{fam}_orm.png', False)
    nt.links.new(bc.outputs['Color'], b.inputs['Base Color'])
    sep = nt.nodes.new('ShaderNodeSeparateColor')
    nt.links.new(orm.outputs['Color'], sep.inputs['Color'])
    nt.links.new(sep.outputs['Green'], b.inputs['Roughness'])
    nt.links.new(sep.outputs['Blue'], b.inputs['Metallic'])
    nmap = nt.nodes.new('ShaderNodeNormalMap')
    nt.links.new(nm.outputs['Color'], nmap.inputs['Color'])
    nt.links.new(nmap.outputs['Normal'], b.inputs['Normal'])
    S.render.engine = 'CYCLES'
    S.cycles.samples = 48
    w = bpy.data.worlds.new('neutral')
    w.use_nodes = True
    w.node_tree.nodes['Background'].inputs['Color'].default_value = (0.5, 0.53, 0.58, 1)
    w.node_tree.nodes['Background'].inputs['Strength'].default_value = 0.7
    S.world = w
    sun = bpy.data.objects.new('key', bpy.data.lights.new('key', 'SUN'))
    sun.data.energy = 3.2
    sun.rotation_euler = (math.radians(55), 0, math.radians(-35))
    S.collection.objects.link(sun)
    cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam'))
    S.collection.objects.link(cam)
    cam.location = review['cam']
    cam.rotation_euler = (Vector(review['look']) - cam.location).to_track_quat('-Z', 'Y').to_euler()
    cam.data.lens = 28
    cam.data.clip_end = 20000.0                          # BLD-12 is 1.2 km tall
    S.camera = cam
    S.render.resolution_x, S.render.resolution_y = 900, 1200
    S.render.filepath = os.path.join(out, f'{asset}_review.png')
    bpy.ops.render.render(write_still=True)
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(out, f'{asset}.blend'))
    with open(os.path.join(out, f'{asset}.json'), 'w') as fh:
        json.dump(results, fh, indent=1)
    print(asset, json.dumps({k: v['triangles'] for k, v in results['lods'].items()}))

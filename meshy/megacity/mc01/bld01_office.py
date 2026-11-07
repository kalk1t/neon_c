"""Living Megacity MC01: BLD-01 Business office slab, benchmark build (Blender 5.2, headless).

  blender -b --factory-startup --python bld01_office.py -- <tiles dir> <outdir> [bays_x=6] [bays_y=5] [floors=18]

Builds the office slab from the MAT-01 contract (3.0 m bay, 4.0 m floor) in three distance forms and exports each:
  near: glazed double-height lobby set back behind concrete columns, entrance canopy, transfer band, curtain-wall
        shaft (UVs in tile units: whole bays and floors), a louvred mechanical floor mid-shaft, parapet, mechanical
        penthouse with louvre fins, HVAC units and the roof access bulkhead;
  mid:  the same masses as plain boxes (shaft, band, lobby, parapet, penthouse);
  far:  shaft box + penthouse box.
Origin: bottom-centre of the lobby floor. Blender Z up / -Y front -> glTF Y up / +Z front. Sockets are exported as
empties: SOCKET_roof, SOCKET_base, SOCKET_bridge_W / _E (a skybridge floor on each short side).
Materials keep the contract names (MAT-01 is swapped for the game's facade material; the others ship as PBR
factors): MAT-01, lobby_glass, concrete_struct, louvre_dark, roof_metal. No textures are embedded.
Writes BLD-01_<lod>.glb, BLD-01.json (bounds, triangles, materials, sockets), BLD-01.blend, BLD-01_review.png.
"""
import bpy, bmesh, sys, os, json, math
from mathutils import Vector

A = sys.argv[sys.argv.index('--') + 1:]
TILES, OUT = os.path.abspath(A[0]), os.path.abspath(A[1])
BX = int(A[2]) if len(A) > 2 else 6
BY = int(A[3]) if len(A) > 3 else 5
FLOORS = int(A[4]) if len(A) > 4 else 18
BAY, FLOOR = 3.0, 4.0
W, D = BX * BAY, BY * BAY                 # shaft footprint (x across the front, y depth)
LOBBY_H, BAND_H = 9.0, 1.5                # double-height lobby, transfer band
SHAFT0 = LOBBY_H + BAND_H                 # first office floor
MECH = FLOORS // 2                        # the mechanical floor (index from the bottom)
TOP = SHAFT0 + FLOORS * FLOOR             # roof slab
os.makedirs(OUT, exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
S = bpy.context.scene

# ---------------------------------------------------------------- materials
def mat(name, col, rough, metal, emit=None, emit_k=0.0):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    b = m.node_tree.nodes['Principled BSDF']
    b.inputs['Base Color'].default_value = (*col, 1)
    b.inputs['Roughness'].default_value = rough
    b.inputs['Metallic'].default_value = metal
    if emit:
        b.inputs['Emission Color'].default_value = (*emit, 1)
        b.inputs['Emission Strength'].default_value = emit_k
    return m

M = {
    'MAT-01': mat('MAT-01', (0.1, 0.12, 0.15), 0.2, 0.5),
    'lobby_glass': mat('lobby_glass', (0.05, 0.055, 0.06), 0.05, 0.0),   # see-through (alpha, below), the lobby is modelled inside
    # the lobby inside: softly self-lit (the game has no interior lamps), downlights small and bright
    'lobby_stone': mat('lobby_stone', (0.62, 0.58, 0.52), 0.3, 0.0, (1.0, 0.8, 0.6), 0.06),
    'lobby_ceiling': mat('lobby_ceiling', (0.75, 0.74, 0.72), 0.8, 0.0, (1.0, 0.85, 0.7), 0.1),
    'core_stone': mat('core_stone', (0.42, 0.38, 0.33), 0.35, 0.0, (1.0, 0.8, 0.6), 0.07),
    'lift_door': mat('lift_door', (0.75, 0.72, 0.65), 0.25, 1.0, (1.0, 0.85, 0.7), 0.05),
    'downlight': mat('downlight', (1.0, 0.9, 0.75), 0.5, 0.0, (1.0, 0.85, 0.65), 3.0),
    'desk_wood': mat('desk_wood', (0.35, 0.22, 0.12), 0.45, 0.0, (1.0, 0.7, 0.4), 0.05),
    'planter': mat('planter', (0.15, 0.15, 0.15), 0.6, 0.0),
    'leaf': mat('leaf', (0.06, 0.2, 0.06), 0.7, 0.0, (0.3, 0.6, 0.2), 0.04),
    'concrete_struct': mat('concrete_struct', (0.50, 0.49, 0.47), 0.82, 0.0),
    'louvre_dark': mat('louvre_dark', (0.11, 0.12, 0.13), 0.45, 0.85),
    'roof_metal': mat('roof_metal', (0.42, 0.44, 0.46), 0.4, 0.9),
    'fin_alu': mat('fin_alu', (0.55, 0.58, 0.62), 0.32, 0.9),
}
_g = M['lobby_glass']
_g.node_tree.nodes['Principled BSDF'].inputs['Alpha'].default_value = 0.22
_g.surface_render_method = 'BLENDED'   # glTF alphaMode BLEND
# review only: the baked MAT-01 tile on the shaft (stripped before export: the game supplies the material)
def textured_mat01():
    nt = M['MAT-01'].node_tree
    b = nt.nodes['Principled BSDF']
    def img(file, srgb):
        n = nt.nodes.new('ShaderNodeTexImage')
        n.image = bpy.data.images.load(os.path.join(TILES, file))
        n.image.colorspace_settings.name = 'sRGB' if srgb else 'Non-Color'
        return n
    bc, nm, orm = img('office_basecolor.png', True), img('office_normal.png', False), img('office_orm.png', False)
    nt.links.new(bc.outputs['Color'], b.inputs['Base Color'])
    sep = nt.nodes.new('ShaderNodeSeparateColor')
    nt.links.new(orm.outputs['Color'], sep.inputs['Color'])
    nt.links.new(sep.outputs['Green'], b.inputs['Roughness'])
    nt.links.new(sep.outputs['Blue'], b.inputs['Metallic'])
    nmap = nt.nodes.new('ShaderNodeNormalMap')
    nt.links.new(nm.outputs['Color'], nmap.inputs['Color'])
    nt.links.new(nmap.outputs['Normal'], b.inputs['Normal'])
    return [bc, nm, orm, sep, nmap]

# ---------------------------------------------------------------- geometry
def block(name, x0, x1, y0, y1, z0, z1, m, uv_tile=None, coll=None):
    """An axis-aligned box. uv_tile = (bay, floor, z_base): side faces get UVs in tile units (u along the face from
    its left corner, v = (z - z_base) / floor); top / bottom faces get a dead UV (the tile's corner)."""
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
            # u runs left to right as seen from outside the face
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


def lod_near(c):
    hx, hy = W / 2, D / 2
    parts = []
    # lobby: glass box set back 1.5 m, concrete columns on the shaft line every 2 bays, soffit
    parts.append(block('lobby', -hx + 1.5, hx - 1.5, -hy + 1.5, hy - 1.5, 0, LOBBY_H, 'lobby_glass', coll=c))
    for i in range(0, BX + 1, 2):
        for y in (-hy + 0.35, hy - 0.35):
            x = -hx + i * BAY
            x = max(-hx + 0.35, min(hx - 0.35, x))
            parts.append(block('col', x - 0.35, x + 0.35, y - 0.35, y + 0.35, 0, LOBBY_H, 'concrete_struct', coll=c))
    for j in range(0, BY + 1, 2):
        for x in (-hx + 0.35, hx - 0.35):
            y = max(-hy + 0.35, min(hy - 0.35, -hy + j * BAY))
            parts.append(block('col', x - 0.35, x + 0.35, y - 0.35, y + 0.35, 0, LOBBY_H, 'concrete_struct', coll=c))
    # lobby glazing: aluminium mullions every 1.5 m and a mezzanine transom at 4.5 m on all four glass walls
    gx0, gx1, gy0, gy1 = -hx + 1.5, hx - 1.5, -hy + 1.5, hy - 1.5
    for (a, b, y, axis) in ((gx0, gx1, gy0, 'x'), (gx0, gx1, gy1, 'x'), (gy0, gy1, gx0, 'y'), (gy0, gy1, gx1, 'y')):
        n = int((b - a) / 1.5)
        for i in range(n + 1):
            u = a + (b - a) * i / n
            if axis == 'x':
                parts.append(block('lmull', u - 0.05, u + 0.05, y - 0.12, y + 0.12, 0, LOBBY_H, 'fin_alu', coll=c))
            else:
                parts.append(block('lmull', y - 0.12, y + 0.12, u - 0.05, u + 0.05, 0, LOBBY_H, 'fin_alu', coll=c))
        for z in (4.5, LOBBY_H - 0.3):
            if axis == 'x':
                parts.append(block('ltrans', a, b, y - 0.1, y + 0.1, z, z + 0.3, 'fin_alu', coll=c))
            else:
                parts.append(block('ltrans', y - 0.1, y + 0.1, a, b, z, z + 0.3, 'fin_alu', coll=c))
    # the lobby inside the glass: stone floor, a lift core with doors on both faces, a mezzanine at the back,
    # reception desk, planters, a ceiling with a grid of downlights
    parts.append(block('lfloor', gx0, gx1, gy0, gy1, 0, 0.05, 'lobby_stone', coll=c))
    parts.append(block('lceil', gx0, gx1, gy0, gy1, LOBBY_H - 0.25, LOBBY_H - 0.05, 'lobby_ceiling', coll=c))
    for x in [gx0 + 1.5 + 3 * i for i in range(int((gx1 - gx0 - 1.5) / 3) + 1)]:
        for y in [gy0 + 1.5 + 3 * j for j in range(int((gy1 - gy0 - 1.5) / 3) + 1)]:
            parts.append(block('dl', x - 0.25, x + 0.25, y - 0.25, y + 0.25, LOBBY_H - 0.28, LOBBY_H - 0.25, 'downlight', coll=c))
    cw, cd = min(9.0, (gx1 - gx0) * 0.4), min(5.0, (gy1 - gy0) * 0.35)
    parts.append(block('core', -cw / 2, cw / 2, -cd / 2 + 1.0, cd / 2 + 1.0, 0.05, LOBBY_H - 0.25, 'core_stone', coll=c))
    for k in (-1, 0, 1):
        x = k * cw / 3.2
        for (y0, y1) in ((-cd / 2 + 0.95, -cd / 2 + 1.0), (cd / 2 + 1.0, cd / 2 + 1.05)):
            parts.append(block('ldoor', x - 0.55, x + 0.55, y0, y1, 0.05, 2.65, 'lift_door', coll=c))
    parts.append(block('mezz', gx0, gx1, gy1 - 4.5, gy1, 4.5, 4.8, 'lobby_stone', coll=c))
    parts.append(block('mrail', gx0, gx1, gy1 - 4.55, gy1 - 4.5, 4.8, 5.85, 'fin_alu', coll=c))
    parts.append(block('reception', -2.2, 2.2, gy0 + 3.0, gy0 + 3.8, 0.05, 1.15, 'desk_wood', coll=c))
    for (x, y) in ((gx0 + 1.2, gy0 + 1.2), (gx1 - 1.2, gy0 + 1.2), (gx0 + 1.2, gy1 - 6.0), (gx1 - 1.2, gy1 - 6.0)):
        parts.append(block('pl', x - 0.5, x + 0.5, y - 0.5, y + 0.5, 0.05, 0.8, 'planter', coll=c))
        parts.append(block('tree', x - 0.7, x + 0.7, y - 0.7, y + 0.7, 0.8, 3.2, 'leaf', coll=c))
    # entrance canopy on the front (-Y), with its two thin hangers
    parts.append(block('canopy', -5, 5, -hy - 3.2, -hy + 1.5, 5.0, 5.35, 'roof_metal', coll=c))
    for x in (-4.6, 4.6):
        parts.append(block('hanger', x - 0.05, x + 0.05, -hy - 3.0, -hy - 2.9, 5.35, LOBBY_H, 'roof_metal', coll=c))
    # transfer band, projecting 0.3 m
    parts.append(block('band', -hx - 0.3, hx + 0.3, -hy - 0.3, hy + 0.3, LOBBY_H, SHAFT0, 'concrete_struct', coll=c))
    # shaft below / above the mechanical floor (MAT-01, whole bays and floors)
    zm0, zm1 = SHAFT0 + MECH * FLOOR, SHAFT0 + (MECH + 1) * FLOOR
    parts.append(block('shaft_lo', -hx, hx, -hy, hy, SHAFT0, zm0, 'MAT-01', (BAY, FLOOR, SHAFT0), c))
    parts.append(block('shaft_hi', -hx, hx, -hy, hy, zm1, TOP, 'MAT-01', (BAY, FLOOR, SHAFT0), c))
    # relief (near only): a slab edge at every floor line and a vertical fin on every bay line, so the curtain wall
    # has real depth: shadow lines and parallax against the rooms behind the glass
    for i in range(FLOORS + 1):
        if i in (MECH, MECH + 1):
            continue
        z = SHAFT0 + i * FLOOR
        for (a, b, d, e) in ((-hx - 0.25, hx + 0.25, -hy - 0.25, -hy), (-hx - 0.25, hx + 0.25, hy, hy + 0.25),
                             (-hx - 0.25, -hx, -hy, hy), (hx, hx + 0.25, -hy, hy)):
            parts.append(block('slab_edge', a, b, d, e, z - 0.15, z + 0.15, 'fin_alu', coll=c))
    for (z0, z1) in ((SHAFT0, zm0), (zm1, TOP)):
        for i in range(1, BX):
            x = -hx + i * BAY
            parts.append(block('fin', x - 0.075, x + 0.075, -hy - 0.45, -hy, z0, z1, 'fin_alu', coll=c))
            parts.append(block('fin', x - 0.075, x + 0.075, hy, hy + 0.45, z0, z1, 'fin_alu', coll=c))
        for j in range(1, BY):
            y = -hy + j * BAY
            parts.append(block('fin', -hx - 0.45, -hx, y - 0.075, y + 0.075, z0, z1, 'fin_alu', coll=c))
            parts.append(block('fin', hx, hx + 0.45, y - 0.075, y + 0.075, z0, z1, 'fin_alu', coll=c))
    # mechanical floor: recessed dark core + horizontal louvre fins all round
    parts.append(block('mech', -hx + 0.4, hx - 0.4, -hy + 0.4, hy - 0.4, zm0, zm1, 'louvre_dark', coll=c))
    for k in range(10):
        z = zm0 + 0.25 + k * 0.36
        for (a, b, d, e) in ((-hx, hx, -hy, -hy + 0.12), (-hx, hx, hy - 0.12, hy), (-hx, -hx + 0.12, -hy, hy), (hx - 0.12, hx, -hy, hy)):
            parts.append(block('fin', a, b, d, e, z, z + 0.06, 'louvre_dark', coll=c))
    for z in (zm0, zm1 - 0.25):
        parts.append(block('mech_edge', -hx - 0.05, hx + 0.05, -hy - 0.05, hy + 0.05, z, z + 0.25, 'concrete_struct', coll=c))
    # roof: parapet, penthouse with louvres, HVAC units, access bulkhead
    for (a, b, d, e) in ((-hx, hx, -hy, -hy + 0.3), (-hx, hx, hy - 0.3, hy), (-hx, -hx + 0.3, -hy, hy), (hx - 0.3, hx, -hy, hy)):
        parts.append(block('parapet', a, b, d, e, TOP, TOP + 1.2, 'concrete_struct', coll=c))
    parts.append(block('roof_slab', -hx + 0.3, hx - 0.3, -hy + 0.3, hy - 0.3, TOP - 0.2, TOP + 0.15, 'roof_metal', coll=c))
    px, py = W * 0.3, D * 0.28
    parts.append(block('penthouse', -px, px, -py + 1.0, py + 1.0, TOP, TOP + 4.5, 'louvre_dark', coll=c))
    parts.append(block('penthouse_cap', -px - 0.2, px + 0.2, -py + 0.8, py + 1.2, TOP + 4.5, TOP + 4.8, 'concrete_struct', coll=c))
    for k in range(8):
        z = TOP + 0.5 + k * 0.48
        parts.append(block('pfin', -px - 0.1, px + 0.1, -py + 0.88, -py + 0.96, z, z + 0.05, 'roof_metal', coll=c))
    for i, x in enumerate((-hx + 2.5, -hx + 5.2, hx - 3.4)):
        parts.append(block('hvac', x - 1.1, x + 1.1, -hy + 1.2, -hy + 2.8, TOP + 0.15, TOP + 1.6, 'roof_metal', coll=c))
        parts.append(block('hvac_fan', x - 0.6, x + 0.6, -hy + 1.4, -hy + 2.6, TOP + 1.6, TOP + 1.75, 'louvre_dark', coll=c))
    parts.append(block('bulkhead', hx - 4.2, hx - 1.4, hy - 3.6, hy - 1.0, TOP + 0.15, TOP + 3.0, 'concrete_struct', coll=c))
    parts.append(block('door', hx - 3.3, hx - 2.3, hy - 3.65, hy - 3.58, TOP + 0.15, TOP + 2.3, 'roof_metal', coll=c))
    return parts


def lod_mid(c):
    hx, hy = W / 2, D / 2
    zm0, zm1 = SHAFT0 + MECH * FLOOR, SHAFT0 + (MECH + 1) * FLOOR
    px, py = W * 0.3, D * 0.28
    return [
        block('lobby', -hx + 1.5, hx - 1.5, -hy + 1.5, hy - 1.5, 0, LOBBY_H, 'lobby_glass', coll=c),
        block('band', -hx - 0.3, hx + 0.3, -hy - 0.3, hy + 0.3, LOBBY_H, SHAFT0, 'concrete_struct', coll=c),
        block('shaft_lo', -hx, hx, -hy, hy, SHAFT0, zm0, 'MAT-01', (BAY, FLOOR, SHAFT0), c),
        block('mech', -hx + 0.2, hx - 0.2, -hy + 0.2, hy - 0.2, zm0, zm1, 'louvre_dark', coll=c),
        block('shaft_hi', -hx, hx, -hy, hy, zm1, TOP, 'MAT-01', (BAY, FLOOR, SHAFT0), c),
        block('parapet', -hx, hx, -hy, hy, TOP, TOP + 1.2, 'concrete_struct', coll=c),
        block('penthouse', -px, px, -py + 1.0, py + 1.0, TOP + 1.2, TOP + 4.8, 'louvre_dark', coll=c),
    ]


def lod_far(c):
    hx, hy = W / 2, D / 2
    px, py = W * 0.3, D * 0.28
    return [
        block('mass', -hx, hx, -hy, hy, 0, TOP + 1.2, 'MAT-01', (BAY, FLOOR, SHAFT0), c),
        block('penthouse', -px, px, -py + 1.0, py + 1.0, TOP + 1.2, TOP + 4.8, 'louvre_dark', coll=c),
    ]


def join(parts, name):
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:
        o.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    o = bpy.context.view_layer.objects.active
    o.name = o.data.name = name
    # merge coplanar duplicates where boxes touch; keep hard edges (architecture)
    bm = bmesh.new()
    bm.from_mesh(o.data)
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-4)
    bm.to_mesh(o.data)
    bm.free()
    for p in o.data.polygons:
        p.use_smooth = False
    return o


def socket(name, loc, coll):
    e = bpy.data.objects.new(name, None)
    e.empty_display_size = 1.5
    e.location = loc
    coll.objects.link(e)
    return e


results = {'asset_id': 'BLD-01', 'version': '1.0-mc01', 'units': 'm', 'origin': 'bottom-centre of the lobby floor',
           'bays': [BX, BY], 'bay_m': BAY, 'floor_m': FLOOR, 'floors': FLOORS, 'mechanical_floor': MECH,
           'lobby_h_m': LOBBY_H, 'shaft_base_m': SHAFT0, 'roof_m': TOP, 'lods': {}}
lods = {}
for lod, fn in (('near', lod_near), ('mid', lod_mid), ('far', lod_far)):
    c = bpy.data.collections.new(f'BLD-01_{lod}')
    S.collection.children.link(c)
    obj = join(fn(c), f'BLD-01_{lod}')
    # glTF: Blender -Y (front) -> glTF +Z: rotate so the front faces Blender -Y already maps to +Z on export (Y-up)
    lods[lod] = (c, obj)
    socks = [socket('SOCKET_base', (0, 0, 0), c), socket('SOCKET_roof', (0, 1.0, TOP + 4.8), c)]
    if lod == 'near':
        zb = SHAFT0 + 6 * FLOOR
        socks += [socket('SOCKET_bridge_W', (-W / 2, 0, zb), c), socket('SOCKET_bridge_E', (W / 2, 0, zb), c)]
    tris = sum(len(p.vertices) - 2 for p in obj.data.polygons)
    bb = [obj.matrix_world @ Vector(v) for v in obj.bound_box]
    mn = [min(v[i] for v in bb) for i in range(3)]
    mx = [max(v[i] for v in bb) for i in range(3)]
    results['lods'][lod] = {'file': f'BLD-01_{lod}.glb', 'triangles': tris,
                            'materials': sorted({s.material.name for s in obj.material_slots}),
                            'bounds_blender_m': [[round(v, 3) for v in mn], [round(v, 3) for v in mx]],
                            'sockets': {s.name.split('.')[0]: [round(v, 3) for v in s.location] for s in socks}}  # Blender suffixes .001 on repeats

# exports (one collection each, no textures: MAT-01 comes from the game)
for lod, (c, obj) in lods.items():
    for l2, (c2, o2) in lods.items():
        c2.hide_viewport = c2.hide_render = (l2 != lod)
    bpy.ops.object.select_all(action='DESELECT')
    for o in c.objects:
        o.select_set(True)
    bpy.ops.export_scene.gltf(filepath=os.path.join(OUT, f'BLD-01_{lod}.glb'), export_format='GLB', use_selection=True,
                              export_yup=True, export_apply=True, export_image_format='NONE', export_animations=False,
                              export_extras=False)

# neutral review render of the near form with the baked MAT-01 tile
for l2, (c2, o2) in lods.items():
    c2.hide_viewport = c2.hide_render = (l2 != 'near')
textured_mat01()
S.render.engine = 'CYCLES'
S.cycles.samples = 64
world = bpy.data.worlds.new('neutral')
world.use_nodes = True
world.node_tree.nodes['Background'].inputs['Color'].default_value = (0.5, 0.53, 0.58, 1)
world.node_tree.nodes['Background'].inputs['Strength'].default_value = 0.7
S.world = world
sun = bpy.data.objects.new('key', bpy.data.lights.new('key', 'SUN'))
sun.data.energy = 3.2
sun.rotation_euler = (math.radians(55), 0, math.radians(-35))
S.collection.objects.link(sun)
cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam'))
S.collection.objects.link(cam)
cam.location = (W * 2.4, -D * 4.2, TOP * 0.45)
cam.rotation_euler = (Vector((0, 0, TOP * 0.45)) - cam.location).to_track_quat('-Z', 'Y').to_euler()
cam.data.lens = 28
S.camera = cam
S.render.resolution_x, S.render.resolution_y = 900, 1200
S.render.filepath = os.path.join(OUT, 'BLD-01_review.png')
bpy.ops.render.render(write_still=True)
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT, 'BLD-01.blend'))
with open(os.path.join(OUT, 'BLD-01.json'), 'w') as fh:
    json.dump(results, fh, indent=1)
print('BLD01', json.dumps({k: v['triangles'] for k, v in results['lods'].items()}))

"""Living Megacity MC02: BLD-05 Residential terrace slab (Blender 5.2, headless).

  blender -b --factory-startup --python bld05_residential.py -- <tiles dir> <outdir>

New York point of view (NYC-REFERENCE.md): 15 Central Park West and the 1920s Central Park West setback apartments:
a pale stone-coloured slab on the MAT-02 grid (3.4 m bay x 3.4 m floor), 40 floors in three stages that step back
with planted terraces, full balcony stacks on both long faces (glass railings, a lamp under each slab, some plants),
a two-storey lobby and shop base, and a roof garden with trees, a pergola and a New York water tank.
Writes BLD-05_<lod>.glb, BLD-05.json (incl. 'stages' for the game's interiors), BLD-05.blend, BLD-05_review.png.
"""
import sys, os, math, random
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L
from lib_bld import block, cyl, ring, tree

A = sys.argv[sys.argv.index('--') + 1:]
TILES, OUT = os.path.abspath(A[0]), os.path.abspath(A[1])
BAY = FLOOR = 3.4
LOBBY_H, BAND_H = 7.0, 1.0
SHAFT0 = LOBBY_H + BAND_H
FLOORS = 40
TOP = SHAFT0 + FLOORS * FLOOR
UV = (BAY, FLOOR, SHAFT0)
# stages: bays across (x), bays deep (y), first floor, end floor (exclusive), centre offset in y (front = -y)
STAGES = [(10, 6, 0, 30, 0.0), (8, 6, 30, 36, 0.0), (6, 5, 36, 40, BAY / 2)]
BAL_D = 1.5
rnd = random.Random(5)
L.start()
L.common_mats()


def stage_box(st):
    bx, by, f0, f1, cy = st
    hx, hy = bx * BAY / 2, by * BAY / 2
    return -hx, hx, cy - hy, cy + hy, SHAFT0 + f0 * FLOOR, SHAFT0 + f1 * FLOOR


def balconies(parts, c, full):
    """Balcony stacks on the long faces: units are two bays; stacks on units 0/2/4 at the front, 1/3 at the back."""
    for st in STAGES[:2]:
        x0, x1, y0, y1, z0, z1 = stage_box(st)
        bx, by, f0, f1, cy = st
        for face, yy, sgn in (('front', y0, -1), ('back', y1, 1)):
            for u in range(bx // 2):
                if (u % 2 == 0) != (face == 'front'):
                    continue
                ux = x0 + (u * 2 + 1) * BAY
                for f in range(max(f0, 1), f1):
                    z = SHAFT0 + f * FLOOR
                    a, b = ux - 1.4, ux + 1.4
                    ya, yb = (yy - BAL_D, yy) if sgn < 0 else (yy, yy + BAL_D)
                    parts.append(block('bal_slab', a, b, ya, yb, z - 0.18, z, 'concrete_pale', coll=c))
                    if not full:
                        continue
                    yo = ya if sgn < 0 else yb                       # outer edge
                    parts.append(block('bal_glass', a, b, yo - 0.01, yo + 0.01, z, z + 1.05, 'rail_glass', coll=c))
                    for xx in (a, b - 0.02):
                        parts.append(block('bal_glass', xx, xx + 0.02, ya, yb, z, z + 1.05, 'rail_glass', coll=c))
                    parts.append(block('bal_rail', a, b, yo - 0.03, yo + 0.03, z + 1.05, z + 1.1, 'steel_rail', coll=c))
                    parts.append(block('bal_lamp', ux - 0.15, ux + 0.15, (ya + yb) / 2 - 0.15, (ya + yb) / 2 + 0.15,
                                       z + FLOOR - 0.2, z + FLOOR - 0.18, 'lamp_warm', coll=c))
                    k = rnd.random()
                    if k < 0.35:                                     # a planter, sometimes a small tree
                        px = a + 0.4 if k < 0.2 else b - 0.4
                        parts.append(block('bal_pl', px - 0.25, px + 0.25, (ya + yb) / 2 - 0.25, (ya + yb) / 2 + 0.25, z, z + 0.45, 'planter', coll=c))
                        parts.append(cyl('bal_leaf', px, (ya + yb) / 2, z + 0.45, z + (1.4 if k < 0.12 else 0.8), 0.3, 'leaf', 6, coll=c))
                    elif k < 0.55:                                   # a small table and two chairs
                        parts.append(block('bal_tab', ux - 0.3, ux + 0.3, (ya + yb) / 2 - 0.3, (ya + yb) / 2 + 0.3, z + 0.7, z + 0.74, 'steel_rail', coll=c))
                        for dx in (-0.6, 0.6):
                            parts.append(block('bal_ch', ux + dx - 0.2, ux + dx + 0.2, (ya + yb) / 2 - 0.2, (ya + yb) / 2 + 0.2, z, z + 0.45, 'wood_deck', coll=c))


def terrace(parts, c, x0, x1, y0, y1, z, edges, lamps=True):
    """A planted terrace on a setback roof: deck, planters with trees, glass railing on the open edges, lamp posts."""
    parts.append(block('deck', x0, x1, y0, y1, z, z + 0.15, 'wood_deck', coll=c))
    ring(parts, 'trail', x0, x1, y0, y1, z + 0.15, z + 1.2, 0.02, 'rail_glass', c, edges)
    ring(parts, 'trail_top', x0, x1, y0, y1, z + 1.2, z + 1.25, 0.05, 'steel_rail', c, edges)
    n = max(1, int((x1 - x0) / 4.5))
    for i in range(n):
        x = x0 + (i + 0.5) * (x1 - x0) / n
        if (y1 - y0) > 2.6:
            tree(parts, x, (y0 + y1) / 2, z + 0.15, 2.6 + rnd.random() * 1.2, c)
        if lamps:
            lx = x + (x1 - x0) / n / 2 - 0.3
            parts.append(cyl('post', lx, y0 + 0.4, z + 0.15, z + 2.4, 0.04, 'steel_dark', 6, coll=c))
            parts.append(block('post_lamp', lx - 0.12, lx + 0.12, y0 + 0.28, y0 + 0.52, z + 2.4, z + 2.55, 'lamp_warm', coll=c))


def water_tank(parts, c, x, y, z):
    """The New York rooftop water tank: a timber barrel on a steel frame, a conical cap."""
    for dx in (-1.2, 1.2):
        for dy in (-1.2, 1.2):
            parts.append(block('tleg', x + dx - 0.08, x + dx + 0.08, y + dy - 0.08, y + dy + 0.08, z, z + 3.0, 'steel_dark', coll=c))
    parts.append(block('tdeck', x - 1.6, x + 1.6, y - 1.6, y + 1.6, z + 3.0, z + 3.2, 'steel_dark', coll=c))
    parts.append(cyl('tank', x, y, z + 3.2, z + 7.4, 1.5, 'tank_wood', 16, coll=c))
    for k in range(4):
        parts.append(cyl('hoop', x, y, z + 3.6 + k * 1.0, z + 3.68 + k * 1.0, 1.53, 'steel_dark', 16, coll=c))
    import bmesh, bpy
    from mathutils import Vector
    bm = bmesh.new()
    bmesh.ops.create_cone(bm, cap_ends=True, segments=16, radius1=1.6, radius2=0.05, depth=1.2)
    for v in bm.verts:
        v.co = Vector((v.co.x + x, v.co.y + y, v.co.z + z + 8.0))
    uv = bm.loops.layers.uv.new('UVMap')
    me = bpy.data.meshes.new('cap')
    bm.to_mesh(me)
    bm.free()
    me.materials.append(L.M['roof_metal'])
    o = bpy.data.objects.new('cap', me)
    c.objects.link(o)
    parts.append(o)


def base(parts, c, full):
    hx, hy = STAGES[0][0] * BAY / 2, STAGES[0][1] * BAY / 2
    parts.append(block('lobby', -hx + 1.2, hx - 1.2, -hy + 1.2, hy - 1.2, 0, LOBBY_H, 'lobby_glass', coll=c))
    parts.append(block('band', -hx - 0.3, hx + 0.3, -hy - 0.3, hy + 0.3, LOBBY_H, SHAFT0, 'concrete_pale', coll=c))
    if not full:
        return
    for i in range(0, STAGES[0][0] + 1, 2):
        x = max(-hx + 0.3, min(hx - 0.3, -hx + i * BAY))
        for y in (-hy + 0.3, hy - 0.3):
            parts.append(block('col', x - 0.3, x + 0.3, y - 0.3, y + 0.3, 0, LOBBY_H, 'concrete_pale', coll=c))
    gx0, gx1, gy0, gy1 = -hx + 1.2, hx - 1.2, -hy + 1.2, hy - 1.2
    parts.append(block('lfloor', gx0, gx1, gy0, gy1, 0, 0.05, 'lobby_stone', coll=c))
    parts.append(block('lceil', gx0, gx1, gy0, gy1, LOBBY_H - 0.25, LOBBY_H - 0.05, 'lobby_ceiling', coll=c))
    parts.append(block('lcore', -6, 6, -2.5, 3.5, 0.05, LOBBY_H - 0.25, 'lobby_stone', coll=c))
    for x in [gx0 + 1.6 + 3.2 * i for i in range(int((gx1 - gx0 - 1.6) / 3.2) + 1)]:
        for y in (gy0 + 1.6, gy1 - 1.6):
            parts.append(block('dl', x - 0.25, x + 0.25, y - 0.25, y + 0.25, LOBBY_H - 0.28, LOBBY_H - 0.25, 'lamp_warm', coll=c))
    # shopfronts: warm canopies with a strip light on both long faces, a residential entrance canopy in the middle
    for (ya, yb) in ((-hy - 2.2, -hy + 0.3), (hy - 0.3, hy + 2.2)):
        for x0 in (-hx + 1.0, hx - 11.0):
            parts.append(block('shop_canopy', x0, x0 + 10, ya, yb, 3.4, 3.6, 'steel_dark', coll=c))
            parts.append(block('shop_light', x0 + 0.2, x0 + 9.8, (ya + yb) / 2 - 0.05, (ya + yb) / 2 + 0.05, 3.35, 3.4, 'lamp_warm', coll=c))
    parts.append(block('entry', -3.5, 3.5, -hy - 3.5, -hy + 0.3, 3.8, 4.1, 'concrete_pale', coll=c))
    for x in (-3.2, 3.2):
        parts.append(cyl('entry_post', x, -hy - 3.2, 0, 3.8, 0.1, 'steel_dark', 8, coll=c))
    parts.append(block('entry_light', -3.0, 3.0, -hy - 3.2, -hy - 0.2, 3.75, 3.8, 'lamp_warm', coll=c))


def shafts(parts, c, full):
    for st in STAGES:
        x0, x1, y0, y1, z0, z1 = stage_box(st)
        parts.append(block('shaft', x0, x1, y0, y1, z0, z1, 'MAT-02', UV, c))
        if full:   # a cornice at each stage top
            parts.append(block('cornice', x0 - 0.35, x1 + 0.35, y0 - 0.35, y1 + 0.35, z1 - 0.1, z1 + 0.45, 'concrete_pale', coll=c))


def tops(parts, c, full):
    # setback terraces on stage 1's roof (both ends) and stage 2's roof (ends + front strip)
    s1, s2, s3 = (stage_box(s) for s in STAGES)
    z = s1[5] + 0.45
    for (xa, xb, e) in ((s1[0], s2[0], 'nsw'), (s2[1], s1[1], 'nse')):
        terrace(parts, c, xa, xb, s1[2], s1[3], z, e) if full else parts.append(block('deck', xa, xb, s1[2], s1[3], z, z + 0.15, 'wood_deck', coll=c))
    z = s2[5] + 0.45
    for (xa, xb, ya, yb, e) in ((s2[0], s3[0], s2[2], s2[3], 'nsw'), (s3[1], s2[1], s2[2], s2[3], 'nse'), (s3[0], s3[1], s2[2], s3[2], 's')):
        terrace(parts, c, xa, xb, ya, yb, z, e, lamps=full) if full else parts.append(block('deck', xa, xb, ya, yb, z, z + 0.15, 'wood_deck', coll=c))
    # roof garden on stage 3: deck, trees, pergola with string lights, the water tank, a lift bulkhead
    x0, x1, y0, y1, _, zt = s3
    zt += 0.45
    parts.append(block('roof', x0, x1, y0, y1, zt - 0.3, zt + 0.15, 'wood_deck', coll=c))
    water_tank(parts, c, x1 - 3.0, y1 - 3.0, zt + 0.15)
    if not full:
        return
    ring(parts, 'rparapet', x0, x1, y0, y1, zt + 0.15, zt + 1.2, 0.02, 'rail_glass', c)
    ring(parts, 'rtop', x0, x1, y0, y1, zt + 1.2, zt + 1.25, 0.05, 'steel_rail', c)
    for (x, y) in ((x0 + 2.0, y0 + 2.0), (x0 + 6.5, y0 + 2.0), (x0 + 2.0, y1 - 2.5), (x1 - 2.2, y0 + 2.2)):
        tree(parts, x, y, zt + 0.15, 3.2 + rnd.random(), c)
    px0, px1, py0, py1 = -3.5, 3.5, y0 + 4.0, y0 + 9.0
    for x in (px0, px1):
        for y in (py0, py1):
            parts.append(block('pg_post', x - 0.08, x + 0.08, y - 0.08, y + 0.08, zt + 0.15, zt + 2.8, 'steel_dark', coll=c))
    for k in range(8):
        y = py0 + k * (py1 - py0) / 7
        parts.append(block('pg_slat', px0 - 0.3, px1 + 0.3, y - 0.04, y + 0.04, zt + 2.8, zt + 2.95, 'wood_deck', coll=c))
    for y in (py0 + 1.2, py0 + 2.6, py0 + 3.8):
        parts.append(block('string', px0, px1, y - 0.015, y + 0.015, zt + 2.65, zt + 2.68, 'string_light', coll=c))
    parts.append(block('pg_table', -1.5, 1.5, py0 + 1.8, py0 + 3.2, zt + 0.85, zt + 0.9, 'wood_deck', coll=c))
    parts.append(block('bulkhead', x0 + 0.5, x0 + 4.0, y1 - 4.5, y1 - 0.5, zt + 0.15, zt + 3.2, 'concrete_pale', coll=c))


def lod_near(c):
    parts = []
    base(parts, c, True); shafts(parts, c, True); balconies(parts, c, True); tops(parts, c, True)
    return parts


def lod_mid(c):
    parts = []
    base(parts, c, False); shafts(parts, c, False); balconies(parts, c, False); tops(parts, c, False)
    return parts


def lod_far(c):
    parts = []
    hx, hy = STAGES[0][0] * BAY / 2, STAGES[0][1] * BAY / 2
    parts.append(block('base', -hx, hx, -hy, hy, 0, SHAFT0, 'concrete_pale', coll=c))
    shafts(parts, c, False)
    s3 = stage_box(STAGES[2])
    parts.append(cyl('tank', s3[1] - 3.0, s3[3] - 3.0, s3[5] + 3.6, s3[5] + 8.0, 1.5, 'tank_wood', 8, coll=c))
    return parts


def sockets(lod, c):
    return [L.socket('SOCKET_base', (0, 0, 0), c), L.socket('SOCKET_roof', (0, 0, TOP + 0.6), c)]


results = {'asset_id': 'BLD-05', 'version': '1.0-mc02', 'units': 'm', 'origin': 'bottom-centre of the ground floor',
           'family': 'residential', 'bays': [STAGES[0][0], STAGES[0][1]], 'bay_m': BAY, 'floor_m': FLOOR, 'floors': FLOORS,
           'mechanical_floor': None, 'lobby_h_m': LOBBY_H, 'shaft_base_m': SHAFT0, 'roof_m': TOP,
           'balcony_depth_m': BAL_D,
           # interiors: per stage half sizes and centre in Blender metres (x, y), floors [f0, f1)
           'stages': [{'hx': s[0] * BAY / 2, 'hy': s[1] * BAY / 2, 'cy': s[4], 'f0': s[2], 'f1': s[3]} for s in STAGES]}
L.build('BLD-05', OUT, {'near': lod_near, 'mid': lod_mid, 'far': lod_far}, results, sockets,
        dict(tiles=TILES, tile='residential', mat='MAT-02', cam=(48, -78, 70), look=(0, 0, 80)))

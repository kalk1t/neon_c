"""Living Megacity MC02: BLD-06 Residential courtyard towers (Blender 5.2, headless).

  blender -b --factory-startup --python bld06_courtyard.py -- <tiles dir> <outdir>

New York point of view (NYC-REFERENCE.md): The San Remo (Emery Roth, 1930): one block, a shared base, two towers
rising from its ends, each topped by a lit temple. Here: a 17-storey residential base (MAT-02, 3.4 m grid) whose roof
is a garden court between the towers (lawns, paths, a fountain, trees, a pergola with lights, a glass pavilion; kept
open and readable from above), two 7 x 7 bay towers with recessed loggias in rhythm (two stacks on the front and the
back of each tower: glass rails, a slab per floor), a setback penthouse stage, and a temple crown (colonnade round a
softly lit core, stepped roof, finial). Shops and two entrance canopies at the street.
Writes BLD-06_<lod>.glb, BLD-06.json (incl. 'stages' with tower offsets 'cx' and 'loggia'), BLD-06.blend, review.
"""
import sys, os, math, random
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L
from lib_bld import block, cyl, ring, tree

A = sys.argv[sys.argv.index('--') + 1:]
TILES, OUT = os.path.abspath(A[0]), os.path.abspath(A[1])
BAY = FLOOR = 3.4
LOBBY_H, BAND_H = 6.0, 0.8
SHAFT0 = LOBBY_H + BAND_H
UV = (BAY, FLOOR, SHAFT0)
BASE = (22, 10, 0, 17)                         # bays x, bays y, floors
TOWER = (7, 7, 17, 55)
CROWN = (5, 5, 55, 59)
TX = (BASE[0] - TOWER[0]) / 2 * BAY            # tower centres at x = -TX, +TX (25.5 m)
FLOORS = CROWN[3]
TOP = SHAFT0 + FLOORS * FLOOR
LG_D = 1.6                                     # loggia depth
LG_BAYS = (1, 5)                               # loggia stacks: bays 1 and 5 of the tower front and back
rnd = random.Random(6)
L.start()
L.common_mats()
L.mat('temple_glow', (1.0, 0.85, 0.62), 0.6, 0.0, (1.0, 0.75, 0.5), 0.28)
L.mat('lawn', (0.07, 0.2, 0.06), 0.9, 0.0, (0.1, 0.3, 0.08), 0.03)


def zf(f):
    return SHAFT0 + f * FLOOR


def tower(parts, c, cx, full):
    h = TOWER[0] * BAY / 2
    z0, z1 = zf(TOWER[2]), zf(TOWER[3])
    # the shaft as solid strips between the loggia stacks (bay lines stay whole bays, so the tile stays aligned)
    edges = [-h] + [v for b in LG_BAYS for v in (-h + b * BAY, -h + (b + 1) * BAY)] + [h]
    for k, (a, b) in enumerate(zip(edges, edges[1:])):
        lg = k % 2 == 1
        y0, y1 = (-h + LG_D, h - LG_D) if lg else (-h, h)
        parts.append(block('shaft', cx + a, cx + b, y0, y1, z0, z1, 'MAT-02', UV, c))
        if not lg:
            continue
        for yy in (-h, h - LG_D):                                       # each loggia: cheeks, slabs, rails
            if full:
                parts.append(block('lg_cheek', cx + a, cx + a + 0.06, yy, yy + LG_D, z0, z1, 'concrete_pale', coll=c))
                parts.append(block('lg_cheek', cx + b - 0.06, cx + b, yy, yy + LG_D, z0, z1, 'concrete_pale', coll=c))
            for f in range(TOWER[2], TOWER[3]):
                z = zf(f)
                if full or f % 4 == 0:
                    parts.append(block('lg_slab', cx + a, cx + b, yy, yy + LG_D, z - 0.12, z + 0.08, 'concrete_pale', coll=c))
                if full:
                    yr = yy if yy < 0 else yy + LG_D - 0.05
                    parts.append(block('lg_rail', cx + a + 0.06, cx + b - 0.06, yr, yr + 0.05, z + 0.08, z + 1.05, 'rail_glass', coll=c))
                    parts.append(block('lg_cap', cx + a + 0.06, cx + b - 0.06, yr - 0.02, yr + 0.07, z + 1.05, z + 1.1, 'steel_rail', coll=c))
    # the setback penthouse stage and its terrace
    hc = CROWN[0] * BAY / 2
    parts.append(block('cornice', cx - h - 0.4, cx + h + 0.4, -h - 0.4, h + 0.4, z1, z1 + 0.5, 'concrete_pale', coll=c))
    parts.append(block('crown', cx - hc, cx + hc, -hc, hc, zf(CROWN[2]), zf(CROWN[3]), 'MAT-02', UV, c))
    if full:
        ring(parts, 't_rail', cx - h, cx + h, -h, h, z1 + 0.5, z1 + 1.6, 0.06, 'rail_glass', c)
        for (sx, sy) in ((-1, -1), (1, -1), (-1, 1), (1, 1)):
            tree(parts, cx + sx * (h - 1.4), sy * (h - 1.4), z1 + 0.5, 2.0, c)
    temple(parts, c, cx, zf(CROWN[3]), full)


def temple(parts, c, cx, z, full):
    """San Remo's crown: plinth, a ring of columns round a softly lit core, entablature, stepped roof, finial."""
    hc = CROWN[0] * BAY / 2
    parts.append(block('t_plinth', cx - hc - 0.3, cx + hc + 0.3, -hc - 0.3, hc + 0.3, z, z + 1.2, 'concrete_pale', coll=c))
    z += 1.2
    parts.append(block('t_core', cx - 4.0, cx + 4.0, -4.0, 4.0, z, z + 8.0, 'temple_glow' if full else 'concrete_pale', coll=c))
    if full:
        r = 6.6
        for k in range(16):
            s = -r + (k % 4 + 0.5) * (2 * r / 4)
            side = k // 4
            x, y = ((s, -r), (r, s), (-s, r), (-r, -s))[side]
            parts.append(cyl('t_col', cx + x, y, z, z + 8.0, 0.42, 'concrete_pale', 10, coll=c))
    parts.append(block('t_entab', cx - 7.4, cx + 7.4, -7.4, 7.4, z + 8.0, z + 9.6, 'concrete_pale', coll=c))
    parts.append(block('t_roof1', cx - 5.6, cx + 5.6, -5.6, 5.6, z + 9.6, z + 11.4, 'concrete_pale', coll=c))
    parts.append(block('t_roof2', cx - 3.6, cx + 3.6, -3.6, 3.6, z + 11.4, z + 13.2, 'concrete_pale', coll=c))
    parts.append(block('t_roof3', cx - 1.8, cx + 1.8, -1.8, 1.8, z + 13.2, z + 15.0, 'concrete_pale', coll=c))
    parts.append(cyl('finial', cx, 0, z + 15.0, z + 21.0, 0.2, 'steel_dark', 8, coll=c))
    if full:
        parts.append(block('finial_light', cx - 0.25, cx + 0.25, -0.25, 0.25, z + 21.0, z + 21.5, 'red_light', coll=c))
        ring(parts, 't_glow', cx - 7.45, cx + 7.45, -7.45, 7.45, z + 7.8, z + 8.0, 0.08, 'temple_glow', c)


def base(parts, c, full):
    hx, hy = BASE[0] * BAY / 2, BASE[1] * BAY / 2
    parts.append(block('base', -hx, hx, -hy, hy, zf(0), zf(BASE[3]), 'MAT-02', UV, c))
    parts.append(block('band', -hx - 0.2, hx + 0.2, -hy - 0.2, hy + 0.2, LOBBY_H, SHAFT0, 'concrete_pale', coll=c))
    parts.append(block('cornice', -hx - 0.5, hx + 0.5, -hy - 0.5, hy + 0.5, zf(BASE[3]) - 0.2, zf(BASE[3]) + 0.4, 'concrete_pale', coll=c))
    if not full:
        parts.append(block('street', -hx, hx, -hy, hy, 0, LOBBY_H, 'concrete_pale', coll=c))
        return
    # street floor: shop fronts on the front and back, lit inside, piers on the bay lines; two entrance canopies
    parts.append(block('core_g', -hx + 4.0, hx - 4.0, -hy + 4.0, hy - 4.0, 0, LOBBY_H, 'concrete_pale', coll=c))
    for (x0, x1) in ((-hx, -hx + 4.0), (hx - 4.0, hx)):
        parts.append(block('end_g', x0, x1, -hy, hy, 0, LOBBY_H, 'concrete_pale', coll=c))
    for yy, s in ((-hy, 1), (hy, -1)):
        y_in = yy + s * 1.2
        parts.append(block('shop_glass', -hx + 4.0, hx - 4.0, min(y_in, y_in + s * 0.05), max(y_in, y_in + s * 0.05), 0.4, LOBBY_H - 0.6, 'lobby_glass', coll=c))
        parts.append(block('shop_in', -hx + 4.0, hx - 4.0, min(yy + s * 3.9, yy + s * 4.0), max(yy + s * 3.9, yy + s * 4.0), 0.2, LOBBY_H - 0.4, 'bar_glow', coll=c))
        parts.append(block('shop_floor', -hx + 4.0, hx - 4.0, min(yy + s * 1.2, yy + s * 4.0), max(yy + s * 1.2, yy + s * 4.0), 0, 0.05, 'lobby_stone', coll=c))
        n = BASE[0]
        for k in range(1, n):
            x = -hx + k * BAY
            if abs(abs(x) - TX) < 2.0:
                continue
            parts.append(block('pier', x - 0.3, x + 0.3, min(yy, yy + s * 0.6), max(yy, yy + s * 0.6), 0, LOBBY_H, 'concrete_pale', coll=c))
    for cx in (-TX, TX):                                                # the two residential entrances (front)
        parts.append(block('canopy', cx - 3.5, cx + 3.5, -hy - 3.0, -hy + 0.3, 3.6, 3.9, 'steel_dark', coll=c))
        parts.append(block('canopy_light', cx - 3.3, cx + 3.3, -hy - 2.8, -hy, 3.55, 3.6, 'lamp_warm', coll=c))
        parts.append(block('door', cx - 1.5, cx + 1.5, -hy + 1.0, -hy + 1.1, 0, 3.0, 'lobby_glass', coll=c))
        parts.append(block('door_in', cx - 2.0, cx + 2.0, -hy + 3.9, -hy + 4.0, 0, 3.4, 'lobby_stone', coll=c))
        for x in (cx - 3.2, cx + 3.2):
            parts.append(cyl('canopy_post', x, -hy - 2.7, 0, 3.6, 0.07, 'steel_dark', 6, coll=c))
            parts.append(block('lamp', x - 0.15, x + 0.15, -hy - 0.4, -hy - 0.1, 2.2, 2.8, 'lamp_warm', coll=c))


def garden(parts, c, full):
    """The court on the base roof between the towers: lawns, a cross of paths, a fountain, trees, a pergola, a glass
    pavilion, lamp posts; a parapet with a glass rail round the edge."""
    hx, hy = BASE[0] * BAY / 2, BASE[1] * BAY / 2
    z = zf(BASE[3]) + 0.4
    parts.append(block('roof_paving', -hx, hx, -hy, hy, z - 0.1, z, 'limestone_dark', coll=c))
    if not full:
        parts.append(block('lawn', -TX + 12.5, TX - 12.5, -hy + 2.0, hy - 2.0, z, z + 0.1, 'lawn', coll=c))
        return
    ring(parts, 'parapet', -hx, hx, -hy, hy, z, z + 0.6, 0.35, 'concrete_pale', c)
    ring(parts, 'g_rail', -hx + 0.1, hx - 0.1, -hy + 0.1, hy - 0.1, z + 0.6, z + 1.3, 0.05, 'rail_glass', c)
    cw = TX - TOWER[0] * BAY / 2 - 1.0                                  # half width of the court between the towers
    for (x0, x1) in ((-cw, -1.2), (1.2, cw)):
        for (y0, y1) in ((-hy + 1.5, -1.2), (1.2, hy - 1.5)):
            parts.append(block('lawn', x0, x1, y0, y1, z, z + 0.12, 'lawn', coll=c))
            parts.append(block('lawn_edge', x0 - 0.15, x1 + 0.15, y0 - 0.15, y1 + 0.15, z, z + 0.05, 'concrete_pale', coll=c))
    # fountain in the middle of the cross
    parts.append(cyl('fountain_rim', 0, 0, z, z + 0.5, 2.6, 'concrete_pale', 16, coll=c))
    parts.append(cyl('fountain', 0, 0, z + 0.5, z + 0.52, 2.35, 'pool_water', 16, coll=c))
    parts.append(cyl('fountain_jet', 0, 0, z + 0.5, z + 1.6, 0.25, 'pool_water', 8, coll=c))
    # trees round the lawns, benches on the paths, lamp posts
    for x in (-cw + 1.5, -cw / 2, cw / 2, cw - 1.5):
        for y in (-hy + 2.8, hy - 2.8):
            tree(parts, x, y, z, 2.6 + rnd.random() * 1.2, c)
    for x in (-cw / 2 - 2.5, cw / 2 + 2.5):
        for y in (-0.8, 0.8):
            parts.append(block('bench', x - 0.9, x + 0.9, y - 0.22, y + 0.22, z, z + 0.45, 'wood_deck', coll=c))
    for (x, y) in ((-5.0, -4.0), (5.0, -4.0), (-5.0, 4.0), (5.0, 4.0), (-cw + 0.6, 0), (cw - 0.6, 0)):
        parts.append(cyl('post', x, y, z, z + 3.2, 0.05, 'steel_dark', 6, coll=c))
        parts.append(block('post_lamp', x - 0.15, x + 0.15, y - 0.15, y + 0.15, z + 3.2, z + 3.45, 'lamp_warm', coll=c))
    # pergola along the back with string lights, and a glass pavilion (residents' lounge) at the front
    py0, py1 = hy - 1.5 - 3.0, hy - 1.5
    for x in [-cw + 1.0 + k * (2 * cw - 2.0) / 6 for k in range(7)]:
        for y in (py0, py1):
            parts.append(block('pg_post', x - 0.1, x + 0.1, y - 0.1, y + 0.1, z, z + 2.8, 'wood_deck', coll=c))
    for k in range(13):
        x = -cw + 1.0 + k * (2 * cw - 2.0) / 12
        parts.append(block('pg_beam', x - 0.06, x + 0.06, py0 - 0.3, py1 + 0.3, z + 2.8, z + 3.0, 'wood_deck', coll=c))
    for y in (py0, py1):
        n = 22
        for k in range(n):
            x = -cw + 1.0 + (k + 0.5) * (2 * cw - 2.0) / n
            parts.append(block('bulb', x - 0.05, x + 0.05, y - 0.05, y + 0.05, z + 2.6, z + 2.7, 'string_light', coll=c))
    px0, px1, pyy0, pyy1 = -4.5, 4.5, -hy + 2.0, -hy + 7.0
    parts.append(block('pav_floor', px0, px1, pyy0, pyy1, z, z + 0.1, 'lobby_stone', coll=c))
    parts.append(block('pav_back', px0 + 0.2, px1 - 0.2, pyy1 - 0.3, pyy1 - 0.1, z, z + 3.2, 'bar_glow', coll=c))
    parts.append(block('pav_glass', px0, px1, pyy0, pyy0 + 0.06, z, z + 3.4, 'lobby_glass', coll=c))
    for x in (px0, px1 - 0.06):
        parts.append(block('pav_glass', x, x + 0.06, pyy0, pyy1, z, z + 3.4, 'lobby_glass', coll=c))
    parts.append(block('pav_roof', px0 - 0.5, px1 + 0.5, pyy0 - 0.5, pyy1 + 0.3, z + 3.4, z + 3.7, 'steel_dark', coll=c))


def lod_near(c):
    parts = []
    base(parts, c, True); garden(parts, c, True)
    for cx in (-TX, TX):
        tower(parts, c, cx, True)
    return parts


def lod_mid(c):
    parts = []
    base(parts, c, False); garden(parts, c, False)
    for cx in (-TX, TX):
        tower(parts, c, cx, False)
    return parts


def lod_far(c):
    parts = []
    hx, hy = BASE[0] * BAY / 2, BASE[1] * BAY / 2
    parts.append(block('base', -hx, hx, -hy, hy, 0, zf(BASE[3]) + 0.4, 'MAT-02', UV, c))
    parts.append(block('lawn', -TX + 12.5, TX - 12.5, -hy + 2.0, hy - 2.0, zf(BASE[3]) + 0.4, zf(BASE[3]) + 0.5, 'lawn', coll=c))
    h, hc = TOWER[0] * BAY / 2, CROWN[0] * BAY / 2
    for cx in (-TX, TX):
        parts.append(block('tower', cx - h, cx + h, -h, h, zf(TOWER[2]), zf(TOWER[3]), 'MAT-02', UV, c))
        parts.append(block('crown', cx - hc, cx + hc, -hc, hc, zf(CROWN[2]), zf(CROWN[3]), 'MAT-02', UV, c))
        z = zf(CROWN[3])
        parts.append(block('temple', cx - 7.4, cx + 7.4, -7.4, 7.4, z, z + 10.8, 'concrete_pale', coll=c))
        parts.append(block('temple_top', cx - 3.6, cx + 3.6, -3.6, 3.6, z + 10.8, z + 16.2, 'concrete_pale', coll=c))
    return parts


def sockets(lod, c):
    hy = BASE[1] * BAY / 2
    h = TOWER[0] * BAY / 2
    zc = zf(40)
    return [L.socket('SOCKET_base', (0, 0, 0), c),
            L.socket('SOCKET_door_W', (-TX, -hy - 0.5, 0), c), L.socket('SOCKET_door_E', (TX, -hy - 0.5, 0), c),
            L.socket('SOCKET_planter', (0, 0, zf(BASE[3]) + 0.4), c),
            L.socket('SOCKET_connector_W', (-TX + h, 0, zc), c), L.socket('SOCKET_connector_E', (TX - h, 0, zc), c),
            L.socket('SOCKET_roof_W', (-TX, 0, TOP + 22.7), c), L.socket('SOCKET_roof_E', (TX, 0, TOP + 22.7), c)]


h, hc = TOWER[0] * BAY / 2, CROWN[0] * BAY / 2
lg_spans = [[-h + b * BAY, -h + (b + 1) * BAY] for b in LG_BAYS]
results = {'asset_id': 'BLD-06', 'version': '1.0-mc02', 'units': 'm', 'origin': 'bottom-centre of the street floor',
           'family': 'residential', 'facade': 'residential', 'bays': [BASE[0], BASE[1]], 'bay_m': BAY,
           'floor_m': FLOOR, 'floors': FLOORS, 'mechanical_floor': None, 'skip': [], 'lobby_h_m': LOBBY_H,
           'shaft_base_m': SHAFT0, 'roof_m': TOP, 'finial_top_m': TOP + 22.7, 'towers_cx_m': [-TX, TX],
           'stages': [{'hx': BASE[0] * BAY / 2, 'hy': BASE[1] * BAY / 2, 'cx': 0.0, 'cy': 0.0, 'f0': BASE[2], 'f1': BASE[3]}]
           + [{'hx': h, 'hy': h, 'cx': cx, 'cy': 0.0, 'f0': TOWER[2], 'f1': TOWER[3],
               'loggia': {'d': LG_D, 'spans': [[a0 + 0.0, a1 + 0.0] for a0, a1 in lg_spans]}} for cx in (-TX, TX)]
           + [{'hx': hc, 'hy': hc, 'cx': cx, 'cy': 0.0, 'f0': CROWN[2], 'f1': CROWN[3]} for cx in (-TX, TX)]}
L.build('BLD-06', OUT, {'near': lod_near, 'mid': lod_mid, 'far': lod_far}, results, sockets,
        dict(tiles=TILES, tile='residential', mat='MAT-02', cam=(160, -230, 170), look=(0, 0, 105)))

"""Living Megacity MC02: BLD-08 Transport administration block (Blender 5.2, headless).

  blender -b --factory-startup --python bld08_transport.py -- <tiles dir> <outdir>

New York point of view (NYC-REFERENCE.md): 111 Eighth Avenue, the Port Authority's full-block freight terminal: a
heavy concrete frame on the MAT-03 grid (6.0 m bay x 4.5 m floor), a two-level truck deck at the base with big
openings, sodium lamps and a loading dock, access decks (galleries) along the front, a stair tower, a broad setback
used as an equipment yard, and a service roof (cooling units, generators, cable trays, a dish and a mast).
Writes BLD-08_<lod>.glb, BLD-08.json (incl. 'stages'), BLD-08.blend, BLD-08_review.png.
"""
import sys, os, math, random
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L
from lib_bld import block, cyl, ring

A = sys.argv[sys.argv.index('--') + 1:]
TILES, OUT = os.path.abspath(A[0]), os.path.abspath(A[1])
BAY, FLOOR = 6.0, 4.5
DECK_H, BAND_H = 10.0, 1.0
SHAFT0 = DECK_H + BAND_H
FLOORS = 16
TOP = SHAFT0 + FLOORS * FLOOR
UV = (BAY, FLOOR, SHAFT0)
STAGES = [(4, 4, 0, 12, 0.0), (4, 3, 12, 16, BAY / 2)]   # the setback opens the front: the equipment yard
GAL_D = 2.0
rnd = random.Random(8)
L.start()
L.common_mats()


def stage_box(st):
    bx, by, f0, f1, cy = st
    hx, hy = bx * BAY / 2, by * BAY / 2
    return -hx, hx, cy - hy, cy + hy, SHAFT0 + f0 * FLOOR, SHAFT0 + f1 * FLOOR


def base(parts, c, full):
    hx, hy = 14.0, 14.0                                   # the plinth fills the lot
    if not full:
        parts.append(block('plinth', -hx, hx, -hy, hy, 0, SHAFT0, 'concrete_struct', coll=c))
        return
    # plinth walls around three big truck openings on the front and one rail opening on each side
    ops = [(-10.5, -3.5), (-2.5, 2.5), (3.5, 10.5)]
    xs = [-hx] + [v for o in ops for v in o] + [hx]
    for a, b in zip(xs[0::2], xs[1::2]):
        parts.append(block('pier', a, b, -hy, -hy + 1.2, 0, DECK_H, 'concrete_struct', coll=c))
    for a, b in ops:
        parts.append(block('lintel', a, b, -hy, -hy + 1.2, 7.0, DECK_H, 'concrete_struct', coll=c))
        parts.append(block('hollow', a, b, -hy + 1.2, hy - 4.0, 0, 7.0, 'opening_dark', coll=c))     # the deck inside
        parts.append(block('sodium', (a + b) / 2 - 1.2, (a + b) / 2 + 1.2, -hy - 0.5, -hy, 7.3, 7.5, 'lamp_sodium', coll=c))
        parts.append(block('stripe', a, b, -hy - 0.02, -hy, 6.7, 7.0, 'hazard', coll=c))
        parts.append(block('stripe', a, a + 0.3, -hy - 0.02, -hy, 0, 6.7, 'hazard', coll=c))
        parts.append(block('stripe', b - 0.3, b, -hy - 0.02, -hy, 0, 6.7, 'hazard', coll=c))
    for sx in (-1, 1):                                    # side walls with a rail opening
        x0, x1 = (hx - 1.2, hx) if sx > 0 else (-hx, -hx + 1.2)
        parts.append(block('side', x0, x1, -hy, -4, 0, DECK_H, 'concrete_struct', coll=c))
        parts.append(block('side', x0, x1, 4, hy, 0, DECK_H, 'concrete_struct', coll=c))
        parts.append(block('side', x0, x1, -4, 4, 6.0, DECK_H, 'concrete_struct', coll=c))
        parts.append(block('rail_hollow', -hx + 1.2, hx - 1.2, -4, 4, 0, 6.0, 'opening_dark', coll=c))
    parts.append(block('back', -hx, hx, hy - 1.2, hy, 0, DECK_H, 'concrete_struct', coll=c))
    parts.append(block('deck_roof', -hx, hx, -hy, hy, DECK_H - 0.6, DECK_H, 'concrete_struct', coll=c))
    # loading dock: platform in front of the middle opening, bollards, a parked truck in the left opening
    parts.append(block('dock', -2.5, 2.5, -hy - 2.0, -hy + 1.2, 0, 1.2, 'concrete_struct', coll=c))
    for x in (-3.0, 3.0):
        parts.append(cyl('bollard', x, -hy - 1.6, 0, 1.1, 0.15, 'hazard', 8, coll=c))
    parts.append(block('truck_box', -9.8, -4.2, -hy + 2.0, -hy + 9.0, 0.9, 4.0, 'roof_metal', coll=c))
    parts.append(block('truck_cab', -9.3, -4.7, -hy + 9.0, -hy + 11.2, 0.6, 3.2, 'steel_dark', coll=c))
    parts.append(block('band', -hx - 0.3, hx + 0.3, -hy - 0.3, hy + 0.3, DECK_H, SHAFT0, 'concrete_struct', coll=c))


def shafts(parts, c, full):
    for st in STAGES:
        x0, x1, y0, y1, z0, z1 = stage_box(st)
        parts.append(block('shaft', x0, x1, y0, y1, z0, z1, 'MAT-03', UV, c))
        if full:
            parts.append(block('cornice', x0 - 0.4, x1 + 0.4, y0 - 0.4, y1 + 0.4, z1 - 0.2, z1 + 0.5, 'concrete_struct', coll=c))


def galleries(parts, c, full):
    """Access decks along the front of stage 1 on every floor, a stair tower at the west end."""
    x0, x1, y0, y1, z0, z1 = stage_box(STAGES[0])
    for f in range(1, STAGES[0][3]):
        z = SHAFT0 + f * FLOOR
        parts.append(block('gal', x0, x1, y0 - GAL_D, y0, z - 0.25, z, 'concrete_struct', coll=c))
        if not full:
            continue
        parts.append(block('gal_rail', x0, x1, y0 - GAL_D, y0 - GAL_D + 0.05, z + 1.05, z + 1.1, 'steel_rail', coll=c))
        parts.append(block('gal_mid', x0, x1, y0 - GAL_D, y0 - GAL_D + 0.03, z + 0.5, z + 0.54, 'steel_rail', coll=c))
        for k in range(13):
            x = x0 + k * (x1 - x0) / 12
            parts.append(block('gal_post', x - 0.03, x + 0.03, y0 - GAL_D, y0 - GAL_D + 0.06, z, z + 1.05, 'steel_rail', coll=c))
        for k in range(4):                                  # soffit lamps of the deck above (cool white)
            x = x0 + (k + 0.5) * BAY
            parts.append(block('gal_lamp', x - 0.4, x + 0.4, y0 - 1.2, y0 - 0.8, z + FLOOR - 0.27, z + FLOOR - 0.25, 'lamp_cool', coll=c))
    # stair tower: concrete with tall slot windows, past the west end of the decks
    sx0, sx1, sy0, sy1 = x0 - 4.0, x0, y0 - GAL_D, y0 + 2.0
    parts.append(block('stair', sx0, sx1, sy0, sy1, SHAFT0, SHAFT0 + STAGES[0][3] * FLOOR + 2.0, 'concrete_struct', coll=c))
    if full:
        for f in range(STAGES[0][3]):
            z = SHAFT0 + f * FLOOR
            parts.append(block('slot', sx0 + 1.6, sx0 + 2.4, sy0 - 0.02, sy0, z + 1.0, z + 3.6, 'lamp_dim', coll=c))


def yard(parts, c, x0, x1, y0, y1, z, full):
    """Equipment yard / service roof: cooling units with fans, generators, cable trays, hazard lines."""
    parts.append(block('yard', x0, x1, y0, y1, z, z + 0.2, 'roof_metal', coll=c))
    ring(parts, 'yrail', x0, x1, y0, y1, z + 0.2, z + 1.1, 0.06, 'steel_rail', c)
    if not full:
        return
    n = int((x1 - x0) / 5)
    for i in range(n):
        x = x0 + 2.5 + i * 5
        if rnd.random() < 0.6:
            parts.append(block('cooler', x - 1.8, x + 1.8, y0 + 0.8, y0 + 3.2, z + 0.2, z + 2.2, 'louvre_dark', coll=c))
            for dx in (-0.9, 0.9):
                parts.append(cyl('fan', x + dx, y0 + 2.0, z + 2.2, z + 2.35, 0.7, 'steel_dark', 12, coll=c))
        else:
            parts.append(block('gen', x - 1.5, x + 1.5, y0 + 1.0, y0 + 2.6, z + 0.2, z + 1.9, 'hazard', coll=c))
            parts.append(cyl('exhaust', x + 1.0, y0 + 1.8, z + 1.9, z + 3.2, 0.12, 'steel_dark', 8, coll=c))
    parts.append(block('tray', x0 + 0.5, x1 - 0.5, y1 - 1.2, y1 - 0.8, z + 0.6, z + 0.75, 'steel_rail', coll=c))
    parts.append(block('line', x0 + 0.3, x1 - 0.3, y0 + 0.4, y0 + 0.5, z + 0.2, z + 0.21, 'hazard', coll=c))


def tops(parts, c, full):
    s1, s2 = stage_box(STAGES[0]), stage_box(STAGES[1])
    yard(parts, c, s1[0], s1[1], s1[2], s2[2], s1[5] + 0.5, full)          # the setback: front strip of stage 1
    x0, x1, y0, y1, _, zt = s2
    zt += 0.5
    yard(parts, c, x0, x1, y0, y1, zt, full)
    # mast with a red light and a dish platform
    parts.append(block('mast', x1 - 2.2, x1 - 1.8, y1 - 2.2, y1 - 1.8, zt, zt + 14.0, 'steel_dark', coll=c))
    if full:
        parts.append(block('mast_light', x1 - 2.3, x1 - 1.7, y1 - 2.3, y1 - 1.7, zt + 14.0, zt + 14.4, 'lamp_sodium', coll=c))
        parts.append(block('dish_pf', x0 + 1.0, x0 + 5.0, y1 - 5.0, y1 - 1.0, zt + 0.2, zt + 2.5, 'steel_dark', coll=c))
        parts.append(cyl('dish', x0 + 3.0, y1 - 3.0, zt + 2.5, zt + 2.9, 1.8, 'roof_metal', 16, coll=c))
        parts.append(block('bulkhead', -2.0, 2.0, y1 - 4.0, y1 - 0.5, zt + 0.2, zt + 3.4, 'concrete_struct', coll=c))


def lod_near(c):
    parts = []
    base(parts, c, True); shafts(parts, c, True); galleries(parts, c, True); tops(parts, c, True)
    return parts


def lod_mid(c):
    parts = []
    base(parts, c, False); shafts(parts, c, False); galleries(parts, c, False); tops(parts, c, False)
    return parts


def lod_far(c):
    parts = []
    base(parts, c, False); shafts(parts, c, False)
    return parts


def sockets(lod, c):
    return [L.socket('SOCKET_base', (0, 0, 0), c), L.socket('SOCKET_roof', (0, 0, TOP + 0.7), c)]


results = {'asset_id': 'BLD-08', 'version': '1.0-mc02', 'units': 'm', 'origin': 'bottom-centre of the truck deck',
           'family': 'transport', 'bays': [STAGES[0][0], STAGES[0][1]], 'bay_m': BAY, 'floor_m': FLOOR, 'floors': FLOORS,
           'mechanical_floor': None, 'lobby_h_m': DECK_H, 'shaft_base_m': SHAFT0, 'roof_m': TOP, 'gallery_depth_m': GAL_D,
           'stages': [{'hx': s[0] * BAY / 2, 'hy': s[1] * BAY / 2, 'cy': s[4], 'f0': s[2], 'f1': s[3]} for s in STAGES]}
L.build('BLD-08', OUT, {'near': lod_near, 'mid': lod_mid, 'far': lod_far}, results, sockets,
        dict(tiles=TILES, tile='transport', mat='MAT-03', cam=(42, -62, 36), look=(0, 0, 40)))

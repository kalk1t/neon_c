"""Living Megacity MC02: BLD-02 Stepped corporate tower (Blender 5.2, headless).

  blender -b --factory-startup --python bld02_stepped.py -- <tiles dir> <outdir>

New York point of view (NYC-REFERENCE.md): 30 Rockefeller Plaza and the Empire State Building: a limestone setback
tower on the MAT-01B grid (3.0 m bay x 4.0 m office floor), a dominant shaft that steps in three times at mechanical
floors (louvred bands), planted terraces with stone balustrades, benches, lamp posts and warm uplights washing the
stone above, and a screened crown (stone fins round the plant floors) with a lit lantern and a mast. Never a
pasted-on crown: the top is the last setback of the same mass. Base: a three-storey stone podium with a bronze portal.
Writes BLD-02_<lod>.glb, BLD-02.json (incl. 'stages', 'skip'), BLD-02.blend, BLD-02_review.png.
"""
import sys, os, math, random
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L
from lib_bld import block, cyl, ring, tree

A = sys.argv[sys.argv.index('--') + 1:]
TILES, OUT = os.path.abspath(A[0]), os.path.abspath(A[1])
BAY, FLOOR = 3.0, 4.0
LOBBY_H, BAND_H = 12.0, 1.5
SHAFT0 = LOBBY_H + BAND_H
UV = (BAY, FLOOR, SHAFT0)
# stages: bays across (x), bays deep (y), first floor, end floor (exclusive); the top floor of stages 1-3 is a
# louvred mechanical band, stage 4 is the screened crown (plant floors, no rooms)
STAGES = [(13, 9, 0, 30), (11, 7, 30, 55), (9, 5, 55, 75), (5, 3, 75, 80)]
FLOORS = STAGES[-1][3]
TOP = SHAFT0 + FLOORS * FLOOR
MECH = [s[3] - 1 for s in STAGES[:3]]
SKIP = MECH + list(range(STAGES[3][2], STAGES[3][3]))
rnd = random.Random(2)
L.start()
L.common_mats()


def box(st):
    bx, by, f0, f1 = st
    hx, hy = bx * BAY / 2, by * BAY / 2
    return -hx, hx, -hy, hy, SHAFT0 + f0 * FLOOR, SHAFT0 + f1 * FLOOR


def louvres(parts, c, x0, x1, y0, y1, z0, z1, m='louvre_dark', step=0.36, inset=0.4):
    parts.append(block('mech', x0 + inset, x1 - inset, y0 + inset, y1 - inset, z0, z1, 'louvre_dark', coll=c))
    k = 0
    z = z0 + 0.25
    while z < z1 - 0.2:
        ring(parts, 'lv', x0, x1, y0, y1, z, z + 0.06, 0.12, m, c)
        z += step
        k += 1


def shafts(parts, c, full):
    for i, st in enumerate(STAGES[:3]):
        x0, x1, y0, y1, z0, z1 = box(st)
        zm = z1 - FLOOR
        parts.append(block('shaft', x0, x1, y0, y1, z0, zm, 'MAT-01B', UV, c))
        if full:
            louvres(parts, c, x0, x1, y0, y1, zm, z1)
            # stone piers at the corners carried up the full stage (the tower reads as one carved mass)
            for (px, py) in ((x0, y0), (x1, y0), (x0, y1), (x1, y1)):
                parts.append(block('corner_pier', px - 0.5, px + 0.5, py - 0.5, py + 0.5, z0, z1 + 0.3, 'limestone', coll=c))
        else:
            parts.append(block('mech', x0 + 0.2, x1 - 0.2, y0 + 0.2, y1 - 0.2, zm, z1, 'louvre_dark', coll=c))
        parts.append(block('cornice', x0 - 0.45, x1 + 0.45, y0 - 0.45, y1 + 0.45, z1, z1 + 0.6, 'limestone_dark', coll=c))


def terrace(parts, c, outer, inner, z, full):
    """The ring between a stage's outline (outer) and the next stage (inner): paving, a stone balustrade, planters,
    benches, lamp posts, and warm uplights along the inner edge washing the stone above."""
    ox0, ox1, oy0, oy1 = outer
    ix0, ix1, iy0, iy1 = inner
    for (a, b, d, e) in ((ox0, ox1, oy0, iy0), (ox0, ox1, iy1, oy1), (ox0, ix0, iy0, iy1), (ix1, ox1, iy0, iy1)):
        parts.append(block('paving', a, b, d, e, z, z + 0.15, 'limestone_dark', coll=c))
    if not full:
        return
    ring(parts, 'balustrade', ox0, ox1, oy0, oy1, z + 0.15, z + 1.1, 0.3, 'limestone', c)
    ring(parts, 'bal_cap', ox0 - 0.05, ox1 + 0.05, oy0 - 0.05, oy1 + 0.05, z + 1.1, z + 1.2, 0.4, 'limestone_dark', c)
    ring(parts, 'uplight', ix0 - 0.25, ix1 + 0.25, iy0 - 0.25, iy1 + 0.25, z + 0.15, z + 0.25, 0.12, 'uplight', c)
    w = min(ix0 - ox0, iy0 - oy0)
    mid = lambda a, b: (a + b) / 2
    for (y, side) in ((mid(oy0, iy0), 's'), (mid(iy1, oy1), 'n')):
        n = int((ox1 - ox0) / 6)
        for i in range(n):
            x = ox0 + 3 + i * (ox1 - ox0 - 6) / max(1, n - 1)
            if i % 2 == 0 and w >= 2.9:
                tree(parts, x, y, z + 0.15, 2.2 + rnd.random() * 0.8, c)
            else:
                parts.append(block('bench', x - 0.9, x + 0.9, y - 0.25, y + 0.25, z + 0.15, z + 0.6, 'wood_deck', coll=c))
                parts.append(cyl('post', x + 1.3, y, z + 0.15, z + 2.6, 0.05, 'bronze', 6, coll=c))
                parts.append(block('post_lamp', x + 1.15, x + 1.45, y - 0.15, y + 0.15, z + 2.6, z + 2.8, 'lamp_warm', coll=c))


def crown(parts, c, full):
    x0, x1, y0, y1, z0, z1 = box(STAGES[3])
    parts.append(block('crown_core', x0 + 0.6, x1 - 0.6, y0 + 0.6, y1 - 0.6, z0, z1, 'louvre_dark', coll=c))
    if full:
        # a screen of tall stone fins all round, lit from inside at the top
        for (a, b, yy, axis) in ((x0, x1, y0, 'x'), (x0, x1, y1, 'x'), (y0, y1, x0, 'y'), (y0, y1, x1, 'y')):
            n = int((b - a) / 0.75)
            for i in range(n + 1):
                u = a + (b - a) * i / n
                if axis == 'x':
                    parts.append(block('cfin', u - 0.12, u + 0.12, yy - 0.35, yy + 0.35, z0, z1, 'limestone', coll=c))
                else:
                    parts.append(block('cfin', yy - 0.35, yy + 0.35, u - 0.12, u + 0.12, z0, z1, 'limestone', coll=c))
        ring(parts, 'crown_glow', x0 + 0.65, x1 - 0.65, y0 + 0.65, y1 - 0.65, z1 - 3.0, z1 - 0.4, 0.05, 'uplight', c)
    parts.append(block('crown_cap', x0 - 0.5, x1 + 0.5, y0 - 0.5, y1 + 0.5, z1, z1 + 0.8, 'limestone_dark', coll=c))
    # lantern (lit box with bronze frame) and the mast
    lx, ly = 3.0, 1.8
    parts.append(block('lantern', -lx, lx, -ly, ly, z1 + 0.8, z1 + 6.0, 'lantern' if full else 'limestone', coll=c))
    if full:
        for (px, py) in ((-lx, -ly), (lx, -ly), (-lx, ly), (lx, ly)):
            parts.append(block('lframe', px - 0.15, px + 0.15, py - 0.15, py + 0.15, z1 + 0.8, z1 + 6.0, 'bronze', coll=c))
    parts.append(block('lantern_cap', -lx - 0.4, lx + 0.4, -ly - 0.4, ly + 0.4, z1 + 6.0, z1 + 6.6, 'bronze', coll=c))
    parts.append(cyl('mast', 0, 0, z1 + 6.6, z1 + 30.0, 0.25, 'bronze', 8, coll=c))
    if full:
        parts.append(block('mast_light', -0.3, 0.3, -0.3, 0.3, z1 + 30.0, z1 + 30.5, 'red_light', coll=c))


def base(parts, c, full):
    x0, x1, y0, y1, _, _ = box(STAGES[0])
    x0, x1, y0, y1 = x0 - 0.5, x1 + 0.5, y0 - 0.5, y1 + 0.5
    if not full:
        parts.append(block('podium', x0, x1, y0, y1, 0, SHAFT0, 'limestone', coll=c))
        return
    # stone walls with a tall bronze portal on the front and shop windows either side
    portal = (-4.5, 4.5)
    parts.append(block('podium_back', x0, x1, y1 - 1.5, y1, 0, SHAFT0, 'limestone', coll=c))
    for (a, b) in ((x0, x0 + 1.5), (x1 - 1.5, x1)):
        parts.append(block('podium_side', a, b, y0, y1, 0, SHAFT0, 'limestone', coll=c))
    xs = [x0 + 1.5, portal[0], portal[1], x1 - 1.5]
    for (a, b) in ((xs[0], xs[1]), (xs[2], xs[3])):
        parts.append(block('podium_wall', a, b, y0, y0 + 1.5, 0, 1.2, 'limestone_dark', coll=c))
        parts.append(block('podium_wall', a, b, y0, y0 + 1.5, 6.5, SHAFT0, 'limestone', coll=c))
        parts.append(block('shop', a + 0.3, b - 0.3, y0 + 1.2, y0 + 1.3, 1.2, 6.5, 'lobby_glass', coll=c))
        parts.append(block('shop_in', a + 0.3, b - 0.3, y0 + 3.0, y0 + 3.2, 1.2, 6.5, 'lamp_warm', coll=c))
        n = int((b - a) / 3)
        for i in range(n + 1):
            x = a + (b - a) * i / n
            parts.append(block('shop_mull', x - 0.1, x + 0.1, y0 + 1.1, y0 + 1.4, 1.2, 6.5, 'bronze', coll=c))
    parts.append(block('portal_head', portal[0], portal[1], y0, y0 + 1.5, 10.0, SHAFT0, 'limestone', coll=c))
    parts.append(block('portal_glass', portal[0] + 0.4, portal[1] - 0.4, y0 + 2.4, y0 + 2.5, 0, 10.0, 'lobby_glass', coll=c))
    parts.append(block('portal_hall', portal[0] + 0.4, portal[1] - 0.4, y0 + 2.5, y0 + 16.0, 0, 0.05, 'lobby_stone', coll=c))
    parts.append(block('portal_hall_c', portal[0] + 0.4, portal[1] - 0.4, y0 + 2.5, y0 + 16.0, 9.6, 10.0, 'lobby_ceiling', coll=c))
    parts.append(block('portal_back', portal[0] + 0.4, portal[1] - 0.4, y0 + 15.8, y0 + 16.0, 0, 10.0, 'lobby_stone', coll=c))
    for x in (portal[0] + 0.4, portal[1] - 0.6):
        parts.append(block('portal_side', x, x + 0.2, y0 + 2.5, y0 + 16.0, 0, 10.0, 'lobby_stone', coll=c))
    for k in range(4):
        x = portal[0] + 0.9 + k * (portal[1] - portal[0] - 1.8) / 3
        parts.append(block('portal_mull', x - 0.08, x + 0.08, y0 + 2.3, y0 + 2.6, 0, 10.0, 'bronze', coll=c))
    parts.append(block('canopy', portal[0] - 1.0, portal[1] + 1.0, y0 - 3.0, y0 + 0.2, 6.0, 6.4, 'bronze', coll=c))
    parts.append(block('canopy_light', portal[0] - 0.8, portal[1] + 0.8, y0 - 2.8, y0, 5.95, 6.0, 'lamp_warm', coll=c))
    for x in (portal[0] - 1.6, portal[1] + 1.6):
        parts.append(block('lantern_l', x - 0.3, x + 0.3, y0 - 0.5, y0 - 0.1, 3.0, 4.0, 'lamp_warm', coll=c))
    parts.append(block('band', x0 - 0.3, x1 + 0.3, y0 - 0.3, y1 + 0.3, LOBBY_H, SHAFT0, 'limestone_dark', coll=c))


def tops(parts, c, full):
    for i in range(3):
        o, n = box(STAGES[i]), box(STAGES[i + 1])
        terrace(parts, c, o[:4], n[:4], o[5] + 0.6, full)


def lod_near(c):
    parts = []
    base(parts, c, True); shafts(parts, c, True); tops(parts, c, True); crown(parts, c, True)
    return parts


def lod_mid(c):
    parts = []
    base(parts, c, False); shafts(parts, c, False); tops(parts, c, False); crown(parts, c, False)
    return parts


def lod_far(c):
    parts = []
    base(parts, c, False)
    for st in STAGES[:3]:
        x0, x1, y0, y1, z0, z1 = box(st)
        parts.append(block('shaft', x0, x1, y0, y1, z0, z1 + 0.6, 'MAT-01B', UV, c))
    crown(parts, c, False)
    return parts


def sockets(lod, c):
    return [L.socket('SOCKET_base', (0, 0, 0), c), L.socket('SOCKET_roof', (0, 0, TOP + 30.5), c)]


results = {'asset_id': 'BLD-02', 'version': '1.0-mc02', 'units': 'm', 'origin': 'bottom-centre of the lobby floor',
           'family': 'office', 'facade': 'office_stone', 'bays': [STAGES[0][0], STAGES[0][1]], 'bay_m': BAY,
           'floor_m': FLOOR, 'floors': FLOORS, 'mechanical_floor': None, 'skip': SKIP, 'lobby_h_m': LOBBY_H,
           'shaft_base_m': SHAFT0, 'roof_m': TOP, 'mast_top_m': TOP + 30.5,
           'stages': [{'hx': s[0] * BAY / 2, 'hy': s[1] * BAY / 2, 'cy': 0.0, 'f0': s[2], 'f1': s[3]} for s in STAGES]}
L.build('BLD-02', OUT, {'near': lod_near, 'mid': lod_mid, 'far': lod_far}, results, sockets,
        dict(tiles=os.path.join(TILES, '..', 'out', 'tiles_b') if not os.path.exists(os.path.join(TILES, 'office_stone_basecolor.png')) else TILES,
             tile='office_stone', mat='MAT-01B', cam=(95, -190, 190), look=(0, 0, 175)))

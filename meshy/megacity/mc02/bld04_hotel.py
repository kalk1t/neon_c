"""Living Megacity MC02: BLD-04 Rounded hotel (Blender 5.2, headless).

  blender -b --factory-startup --python bld04_hotel.py -- <tiles dir> <outdir>

New York point of view (NYC-REFERENCE.md): the Lipstick Building (Philip Johnson / John Burgee, 1986): an oval
tower in red-brown granite bands that telescopes in twice, scaled up as a hotel. Rounded corners on a regular room
grid (MAT-05: 3.6 m bay = one hotel room, 3.2 m floor; each quarter arc is exactly four bays, so the windows run on
round the curves), French balconies on every room (in the tile), louvred service floors at the setbacks, a sheltered
bronze entrance canopy over a glazed lobby ring behind granite piers, small planted terraces, and a rooftop bar with
a pool, loungers, a glass pavilion and warm string lights. Restrained room light: the rooms light one by one.
Writes BLD-04_<lod>.glb, BLD-04.json (incl. 'stages' with corner radius 'r', 'skip'), BLD-04.blend, BLD-04_review.png.
"""
import sys, os, math, random
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L
from lib_bld import block, cyl, rblock

A = sys.argv[sys.argv.index('--') + 1:]
TILES, OUT = os.path.abspath(A[0]), os.path.abspath(A[1])
BAY, FLOOR = 3.6, 3.2
LOBBY_H, BAND_H = 9.0, 1.0
SHAFT0 = LOBBY_H + BAND_H
UV = (BAY, FLOOR, SHAFT0)
R = 4 * BAY * 2 / math.pi                      # corner radius: a quarter arc = four bays (9.17 m)
# stages: straight bays across (x), straight bays deep (y), first floor, end floor (exclusive)
STAGES = [(8, 3, 0, 36), (6, 2, 36, 50), (4, 1, 50, 60)]
FLOORS = STAGES[-1][3]
TOP = SHAFT0 + FLOORS * FLOOR
MECH = [18, 35, 49]                            # louvred service floors (the top floor of stages 1 and 2, mid-shaft)
rnd = random.Random(4)
L.start()
L.common_mats()


def hs(st):
    """Half sizes of a stage's outline."""
    return st[0] * BAY / 2 + R, st[1] * BAY / 2 + R


def zf(f):
    return SHAFT0 + f * FLOOR


def shafts(parts, c, seg, full):
    for st in STAGES:
        hx, hy = hs(st)
        cuts = [m for m in MECH if st[2] <= m < st[3]]
        f = st[2]
        for m in cuts + [st[3]]:                                         # facade runs between the service floors
            if m > f:
                parts.append(rblock('shaft', hx, hy, 0, R, zf(f), zf(m), 'MAT-05', UV, seg, caps=False, coll=c))
            if m < st[3]:
                service(parts, c, hx, hy, zf(m), zf(m + 1), seg, full)
            f = m + 1
        # cornice + roof of the stage (the next stage's terrace, or the rooftop)
        parts.append(rblock('cornice', hx + 0.35, hy + 0.35, 0, R + 0.35, zf(st[3]), zf(st[3]) + 0.5, 'granite_dark', seg=seg, coll=c))


def service(parts, c, hx, hy, z0, z1, seg, full):
    parts.append(rblock('svc', hx - 0.35, hy - 0.35, 0, R - 0.35, z0, z1, 'louvre_dark', seg=seg, caps=False, coll=c))
    if full:
        z = z0 + 0.3
        while z < z1 - 0.2:
            parts.append(rblock('louvre', hx - 0.05, hy - 0.05, 0, R - 0.05, z, z + 0.07, 'steel_rail', seg=seg, caps=False, coll=c))
            z += 0.4
    parts.append(rblock('svc_band', hx + 0.1, hy + 0.1, 0, R + 0.1, z1 - 0.25, z1, 'granite_dark', seg=seg, caps=False, coll=c))


def terraces(parts, c, seg, full):
    """The setback rings: glass rail on the edge, planters with small trees at the ends, lamps."""
    for i in range(2):
        hx, hy = hs(STAGES[i])
        z = zf(STAGES[i][3]) + 0.5
        if not full:
            continue
        parts.append(rblock('t_rail', hx, hy, 0, R, z, z + 1.1, 'rail_glass', seg=seg, caps=False, coll=c))
        parts.append(rblock('t_cap', hx + 0.04, hy + 0.04, 0, R + 0.04, z + 1.1, z + 1.16, 'bronze', seg=seg, caps=False, coll=c))
        nx, _ = hs(STAGES[i + 1])
        x = (hx + nx) / 2                                                # the wider terraces at the round ends
        for sx in (-1, 1):
            L.tree(parts, sx * x, 0, z, 2.4, c)
            for sy in (-1, 1):
                parts.append(block('t_planter', sx * x - 0.5, sx * x + 0.5, sy * 4.5 - 0.5, sy * 4.5 + 0.5, z, z + 0.7, 'planter', coll=c))
                parts.append(block('t_lamp', sx * x - 0.12, sx * x + 0.12, sy * 6.5 - 0.12, sy * 6.5 + 0.12, z, z + 0.9, 'uplight', coll=c))


def base(parts, c, seg, full):
    hx, hy = hs(STAGES[0])
    if not full:
        parts.append(rblock('lobby', hx - 0.8, hy - 0.8, 0, R - 0.8, 0, LOBBY_H, 'lobby_glass' if seg > 2 else 'granite_dark', seg=seg, caps=False, coll=c))
        parts.append(rblock('band', hx + 0.2, hy + 0.2, 0, R + 0.2, LOBBY_H, SHAFT0, 'granite', seg=seg, coll=c))
        return
    # glazed lobby ring set back 2.5 m behind granite piers on the bay lines; lit lobby inside
    parts.append(rblock('lobby_glass', hx - 2.5, hy - 2.5, 0, R - 2.5, 0, LOBBY_H, 'lobby_glass', seg=seg, caps=False, coll=c))
    parts.append(rblock('lobby_back', hx - 9.0, hy - 9.0, 0, max(0.5, R - 9.0), 0, LOBBY_H, 'lobby_stone', seg=seg, caps=False, coll=c))
    parts.append(rblock('lobby_floor', hx - 2.5, hy - 2.5, 0, R - 2.5, 0, 0.05, 'lobby_stone', seg=seg, coll=c))
    parts.append(rblock('lobby_ceil', hx - 2.5, hy - 2.5, 0, R - 2.5, LOBBY_H - 0.4, LOBBY_H - 0.35, 'bar_glow', seg=seg, coll=c))
    parts.append(rblock('band', hx + 0.2, hy + 0.2, 0, R + 0.2, LOBBY_H, SHAFT0, 'granite', seg=seg, caps=False, coll=c))
    parts.append(rblock('soffit', hx + 0.2, hy + 0.2, 0, R + 0.2, LOBBY_H - 0.35, LOBBY_H, 'granite_dark', seg=seg, coll=c))
    # piers: one per bay along the outline (straights and arcs)
    per = 2 * (STAGES[0][0] + STAGES[0][1]) * BAY + 2 * math.pi * R
    n = round(per / BAY)
    for k in range(n):
        x, y, a = outline_at(hx, hy, (k + 0.5) * per / n)
        if abs(x) < 6.5 and y < 0:                                       # the entrance: no piers under the canopy
            continue
        o = block('pier', -0.45, 0.45, -0.3, 0.3, 0, LOBBY_H - 0.35, 'granite', coll=c)
        o.location = (x, y, 0)
        o.rotation_euler = (0, 0, a)
        parts.append(o)
    # the entrance: a bronze canopy on two posts, lit underneath, lanterns, a revolving door drum
    y0 = -hy
    parts.append(block('canopy', -7.0, 7.0, y0 - 5.0, y0 + 2.5, 5.2, 5.6, 'bronze', coll=c))
    parts.append(block('canopy_light', -6.7, 6.7, y0 - 4.7, y0 + 2.2, 5.15, 5.2, 'lamp_warm', coll=c))
    parts.append(block('canopy_edge', -7.05, 7.05, y0 - 5.05, y0 - 4.95, 5.6, 5.9, 'bronze', coll=c))
    for x in (-6.5, 6.5):
        parts.append(cyl('canopy_post', x, y0 - 4.6, 0, 5.2, 0.12, 'bronze', 8, coll=c))
        parts.append(block('lantern_l', x - 0.25, x + 0.25, y0 - 4.85, y0 - 4.35, 2.4, 3.2, 'lamp_warm', coll=c))
    parts.append(cyl('door_drum', 0, y0 + 2.3, 0, 3.2, 1.6, 'lobby_glass', 16, coll=c))
    parts.append(cyl('door_top', 0, y0 + 2.3, 3.2, 3.5, 1.7, 'bronze', 16, coll=c))
    # inside: reception desk facing the door, sofas, plants, a lit feature wall
    parts.append(block('reception', -4.0, 4.0, y0 + 7.0, y0 + 8.0, 0.05, 1.1, 'granite', coll=c))
    parts.append(block('reception_top', -4.1, 4.1, y0 + 6.9, y0 + 8.1, 1.1, 1.18, 'bronze', coll=c))
    parts.append(block('feature_wall', -5.0, 5.0, y0 + 8.9, y0 + 9.0, 0.05, LOBBY_H - 0.4, 'bar_glow', coll=c))
    for sx in (-1, 1):
        for k in range(3):
            x = sx * (8.0 + k * 4.5)
            parts.append(block('lobby_sofa', x - 1.0, x + 1.0, y0 + 4.0, y0 + 4.8, 0.05, 0.5, 'tank_wood', coll=c))
            parts.append(block('lobby_sofa_b', x - 1.0, x + 1.0, y0 + 4.6, y0 + 4.8, 0.5, 0.9, 'tank_wood', coll=c))
            L.tree(parts, x + 2.0, y0 + 5.5, 0.05, 1.8, c)
    parts.append(block('carpet', -1.2, 1.2, y0 - 5.0, y0 + 0.6, 0.0, 0.03, 'granite_dark', coll=c))


def outline_at(hx, hy, s):
    """Point and outward angle at arc length s along a stage outline (from the left end of the front straight)."""
    sx, sy = 2 * (hx - R), 2 * (hy - R)
    q = math.pi * R / 2
    legs = [('l', sx, (-hx + R, -hy), 0.0), ('a', q, (hx - R, -hy + R), -90), ('l', sy, (hx, -hy + R), 90),
            ('a', q, (hx - R, hy - R), 0), ('l', sx, (hx - R, hy), 180), ('a', q, (-hx + R, hy - R), 90),
            ('l', sy, (-hx, hy - R), 270), ('a', q, (-hx + R, -hy + R), 180)]
    for kind, ln, p, a0 in legs:
        if s <= ln:
            if kind == 'l':
                d = math.radians(a0)
                return p[0] + math.cos(d) * s, p[1] + math.sin(d) * s, d
            a = math.radians(a0 + 90 * s / ln)
            return p[0] + R * math.cos(a), p[1] + R * math.sin(a), a + math.pi / 2
        s -= ln
    return -hx + R, -hy, 0.0


def rooftop(parts, c, seg, full):
    """The roof of the top stage: a bar with a pool, loungers, a glass pavilion, string lights, a glass rail."""
    hx, hy = hs(STAGES[-1])
    z = TOP + 0.5
    parts.append(rblock('deck', hx - 0.5, hy - 0.5, 0, R - 0.5, z, z + 0.12, 'wood_deck', seg=seg, coll=c))
    # plant room at the back (north) and the bar pavilion on the east end
    parts.append(block('plant_room', -6.0, 6.0, 3.5, hy - 2.0, z, z + 4.2, 'louvre_dark', coll=c))
    parts.append(block('plant_cap', -6.3, 6.3, 3.2, hy - 1.7, z + 4.2, z + 4.5, 'granite_dark', coll=c))
    if not full:
        return
    parts.append(rblock('r_rail', hx - 0.3, hy - 0.3, 0, R - 0.3, z, z + 1.15, 'rail_glass', seg=seg, caps=False, coll=c))
    parts.append(rblock('r_cap', hx - 0.26, hy - 0.26, 0, R - 0.26, z + 1.15, z + 1.21, 'bronze', seg=seg, caps=False, coll=c))
    # pool on the west end, lit from within, a stone coping
    px0, px1, py0, py1 = -hx + 3.0, -2.0, -6.5, 1.5
    parts.append(block('pool_coping', px0 - 0.4, px1 + 0.4, py0 - 0.4, py1 + 0.4, z + 0.12, z + 0.3, 'pool_tile', coll=c))
    parts.append(block('pool', px0, px1, py0, py1, z + 0.3, z + 0.31, 'pool_water', coll=c))
    for k in range(5):
        x = px0 + 1.0 + k * (px1 - px0 - 2.0) / 4
        parts.append(block('lounger', x - 0.35, x + 0.35, py0 - 2.6, py0 - 0.8, z + 0.12, z + 0.45, 'pool_tile', coll=c))
        parts.append(block('lounger_back', x - 0.35, x + 0.35, py0 - 1.0, py0 - 0.8, z + 0.45, z + 0.9, 'pool_tile', coll=c))
    # bar pavilion: glass box, lit inside, bronze roof, a counter
    bx0, bx1, by0, by1 = 3.0, hx - 4.0, -7.0, 1.0
    parts.append(block('bar_in', bx0 + 0.3, bx1 - 0.3, by0 + 0.3, by1 - 0.3, z + 0.12, z + 0.2, 'lobby_stone', coll=c))
    parts.append(block('bar_back', bx0 + 0.3, bx1 - 0.3, by1 - 0.6, by1 - 0.3, z + 0.12, z + 3.2, 'bar_glow', coll=c))
    parts.append(block('bar_counter', bx0 + 1.0, bx1 - 1.0, by1 - 2.2, by1 - 1.6, z + 0.12, z + 1.15, 'granite', coll=c))
    parts.append(block('bar_glass', bx0, bx1, by0, by0 + 0.08, z + 0.12, z + 3.3, 'lobby_glass', coll=c))
    for x in (bx0, bx1 - 0.08):
        parts.append(block('bar_glass_s', x, x + 0.08, by0, by1, z + 0.12, z + 3.3, 'lobby_glass', coll=c))
    parts.append(block('bar_roof', bx0 - 0.6, bx1 + 0.6, by0 - 1.2, by1 + 0.3, z + 3.3, z + 3.6, 'bronze', coll=c))
    parts.append(block('bar_soffit', bx0 - 0.5, bx1 + 0.5, by0 - 1.1, by1 + 0.2, z + 3.25, z + 3.3, 'lamp_warm', coll=c))
    for x in (bx0 + 1.5, (bx0 + bx1) / 2, bx1 - 1.5):                   # tables in front of the bar
        parts.append(cyl('table', x, by0 - 2.6, z + 0.12, z + 1.05, 0.4, 'bronze', 8, coll=c))
    # string lights: posts round the deck, sagging runs of lamps between them
    posts = [(-hx + 2.0, -hy + 4.0), (-2.0, -hy + 1.5), (hx - 2.0, -hy + 4.0), (hx - 2.0, hy - 4.0),
             (-hx + 2.0, hy - 4.0)]
    for x, y in posts:
        parts.append(cyl('sl_post', x, y, z + 0.12, z + 3.6, 0.06, 'steel_dark', 6, coll=c))
    for (a, b) in zip(posts, posts[1:] + posts[:1]):
        n = max(4, int(math.hypot(b[0] - a[0], b[1] - a[1]) / 1.2))
        for k in range(1, n):
            t = k / n
            x, y = a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t
            zz = z + 3.5 - 0.8 * math.sin(math.pi * t)
            parts.append(block('bulb', x - 0.06, x + 0.06, y - 0.06, y + 0.06, zz - 0.06, zz + 0.06, 'string_light', coll=c))
    # beacon on the plant room
    parts.append(cyl('mast', 0, hy - 4.0, z + 4.5, z + 14.0, 0.12, 'steel_dark', 6, coll=c))
    parts.append(block('mast_light', -0.2, 0.2, hy - 4.2, hy - 3.8, z + 14.0, z + 14.4, 'red_light', coll=c))


def lod(seg, full):
    def fn(c):
        parts = []
        base(parts, c, seg, full); shafts(parts, c, seg, full); terraces(parts, c, seg, full); rooftop(parts, c, seg, full)
        return parts
    return fn


def lod_far(c):
    parts = []
    hx, hy = hs(STAGES[0])
    parts.append(rblock('base', hx, hy, 0, R, 0, SHAFT0, 'granite_dark', seg=2, coll=c))
    for st in STAGES:
        hx, hy = hs(st)
        parts.append(rblock('shaft', hx, hy, 0, R, zf(st[2]), zf(st[3]) + 0.5, 'MAT-05', UV, seg=2, coll=c))
    return parts


def sockets(lod_, c):
    hx, hy = hs(STAGES[0])
    return [L.socket('SOCKET_base', (0, 0, 0), c), L.socket('SOCKET_entrance_canopy', (0, -hy - 5.0, 5.2), c),
            L.socket('SOCKET_roof', (0, 0, TOP + 0.62), c)]


results = {'asset_id': 'BLD-04', 'version': '1.0-mc02', 'units': 'm', 'origin': 'bottom-centre of the lobby floor',
           'family': 'hotel', 'facade': 'hotel', 'bays': [STAGES[0][0], STAGES[0][1]], 'bay_m': BAY,
           'floor_m': FLOOR, 'floors': FLOORS, 'mechanical_floor': None, 'skip': MECH, 'lobby_h_m': LOBBY_H,
           'shaft_base_m': SHAFT0, 'roof_m': TOP, 'corner_radius_m': round(R, 3), 'mast_top_m': TOP + 0.5 + 14.4,
           'stages': [{'hx': round(hs(s)[0], 3), 'hy': round(hs(s)[1], 3), 'cy': 0.0, 'r': round(R, 3), 'f0': s[2], 'f1': s[3]}
                      for s in STAGES]}
L.build('BLD-04', OUT, {'near': lod(8, True), 'mid': lod(4, False), 'far': lod_far}, results, sockets,
        dict(tiles=TILES, tile='hotel', mat='MAT-05', cam=(120, -250, 140), look=(0, 0, 100)))

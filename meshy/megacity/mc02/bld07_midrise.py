"""Living Megacity MC02: BLD-07 Residential midrise, two variants (Blender 5.2, headless).

  blender -b --factory-startup --python bld07_midrise.py -- <tiles dir> <outdir> <A|B>

New York point of view (NYC-REFERENCE.md): the prewar apartment blocks of Emery Roth and his contemporaries
(1920s-30s, 12-20 storeys): a broad masonry mass with punched windows (MAT-02 on the 3.4 m grid), a pale stone base
and stringcourse, a cornice at every setback, small setbacks near the top with planted terraces, old-style steel
balconies, wooden water tanks on steel legs, an elevator bulkhead and machinery, a small roof deck. The lower layer of
the city: the roofs are seen from the elevated road, so they are worked (decks, tanks, planters, lights).
Variants differ by plan, floors and terraces, not by scaling:
  A  U-plan court block, 16 x 10 bays: two wings (floors 0-15) round a front court with a garden forecourt, a back bar
     to floor 17, a stepped top (floors 18-19), wing-roof terraces
  B  solid block with setbacks, 14 x 9 bays: floors 0-12, a terrace ring at 13-16 (inset one bay), a penthouse at
     17-18 (inset three bays), balcony stacks on the front
Writes BLD-07<v>_<lod>.glb, BLD-07<v>.json (stages, sockets), BLD-07<v>.blend, review.
"""
import sys, os, math, random
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L
from lib_bld import block, cyl, ring, tree

A = sys.argv[sys.argv.index('--') + 1:]
TILES, OUT, VAR = os.path.abspath(A[0]), os.path.abspath(A[1]), A[2].upper()
ASSET = 'BLD-07' + VAR
BAY = FLOOR = 3.4
LOBBY_H, BAND_H = 4.6, 0.8
SHAFT0 = LOBBY_H + BAND_H
UV = (BAY, FLOOR, SHAFT0)
rnd = random.Random(7 if VAR == 'A' else 17)

# stages: (x0, x1, y0, y1) in bays from the centre line (x: -nx/2..nx/2, y: -ny/2..ny/2; -y is the street front), f0, f1
if VAR == 'A':
    NX, NY = 16, 10
    # stages never overlap in plan on the same floor (so no stage's rooms sit inside another): the wings round the
    # court (front and back halves), the back bar between them, the full-width bar over all three, the stepped top
    STAGES = [(-8, -4, -5, 1, 0, 16), (4, 8, -5, 1, 0, 16), (-8, -4, 1, 5, 0, 16), (4, 8, 1, 5, 0, 16),
              (-4, 4, 1, 5, 0, 16), (-8, 8, 1, 5, 16, 18), (-7, 7, 1.5, 4.5, 18, 20)]
    BALC = [((-8, -4), (-6.5,)), ((4, 8), (6.5,))]                 # balcony stacks on the wing ends (bay centres)
    TANKS = [(-4.7, 3.0), (-3.1, 3.0)]
else:
    NX, NY = 14, 9
    STAGES = [(-7, 7, -4.5, 4.5, 0, 13), (-6, 6, -3.5, 3.5, 13, 17), (-4, 4, -2.5, 2.5, 17, 19)]
    BALC = [((-7, 7), (-4.5, -2.5, 2.5, 4.5))]
    TANKS = [(-2.0, -0.5)]
FLOORS = max(s[5] for s in STAGES)
HX, HY = NX * BAY / 2, NY * BAY / 2
L.start()
L.common_mats()
L.mat('brick_cornice', (0.55, 0.5, 0.44), 0.7, 0.0)


def zf(f):
    return SHAFT0 + f * FLOOR


def rect(s):
    return s[0] * BAY, s[1] * BAY, s[2] * BAY, s[3] * BAY


def covered(s):
    """A stage whose roof is wholly under the stage above (its roof is a floor, not a terrace)."""
    return any(o[4] == s[5] and o[0] <= s[0] and o[1] >= s[1] and o[2] <= s[2] and o[3] >= s[3] for o in STAGES)


def roof_of(s):
    """The parts of a stage's roof nothing stands on (the stage above, if any, is inset in it)."""
    up = [o for o in STAGES if o[4] == s[5] and o[0] >= s[0] and o[1] <= s[1] and o[2] >= s[2] and o[3] <= s[3]]
    return up[0] if up else None


def masonry(parts, c, full):
    for s in STAGES:
        x0, x1, y0, y1 = rect(s)
        parts.append(block('mass', x0, x1, y0, y1, zf(s[4]), zf(s[5]), 'MAT-02', UV, c))
        # cornice at the top of every stage, stringcourse over floor 1 on the lowest stages
        o = 0.45
        parts.append(block('cornice', x0 - o, x1 + o, y0 - o, y1 + o, zf(s[5]) - 0.3, zf(s[5]) + 0.25, 'brick_cornice', coll=c))
        if full and s[4] == 0:
            parts.append(block('string', x0 - 0.15, x1 + 0.15, y0 - 0.15, y1 + 0.15, zf(2) - 0.2, zf(2) + 0.1, 'concrete_pale', coll=c))
        if covered(s):
            continue
        if full:                                                     # parapet round the roof
            ring(parts, 'parapet', x0, x1, y0, y1, zf(s[5]) + 0.25, zf(s[5]) + 1.1, 0.3, 'brick_cornice', c)
        if s[5] < FLOORS:
            terrace(parts, c, s, full)


def terrace(parts, c, s, full):
    """A setback roof: wooden deck strips, planters with trees, a few lamps, the rail is the parapet."""
    x0, x1, y0, y1 = rect(s)
    z = zf(s[5]) + 0.25
    up = roof_of(s)
    ux0, ux1, uy0, uy1 = rect(up) if up else (0, 0, 0, 0)
    if not up and full:                                               # a wing roof: stair bulkhead and condensers at the back
        xm = (x0 + x1) / 2
        parts.append(block('w_bulk', xm - 2.2, xm + 2.2, y1 - 5.0, y1 - 0.6, z, z + 3.4, 'brick_cornice', coll=c))
        parts.append(block('w_bulk_cap', xm - 2.4, xm + 2.4, y1 - 5.2, y1 - 0.4, z + 3.4, z + 3.65, 'concrete_pale', coll=c))
        for dx in (-4.5, 4.5):
            parts.append(block('w_ac', xm + dx - 1.0, xm + dx + 1.0, y1 - 3.6, y1 - 1.8, z, z + 1.3, 'roof_metal', coll=c))
        y1 -= 5.6
    strips = [(x0 + 0.4, x1 - 0.4, y0 + 0.4, (uy0 if up else y1) - 0.2)]           # the front strip
    if up:
        strips += [(x0 + 0.4, ux0 - 0.2, uy0, uy1), (ux1 + 0.2, x1 - 0.4, uy0, uy1), (x0 + 0.4, x1 - 0.4, uy1 + 0.2, y1 - 0.4)]
    for (a0, a1, b0, b1) in strips:
        if a1 - a0 < 1.0 or b1 - b0 < 1.0:
            continue
        parts.append(block('deck', a0, a1, b0, b1, z, z + 0.08, 'wood_deck', coll=c))
        if not full:
            continue
        n = max(1, int((a1 - a0) / 7.0))
        for k in range(n):
            x = a0 + (k + 0.5) * (a1 - a0) / n
            y = b0 + 0.7 if b1 - b0 > 2.0 else (b0 + b1) / 2
            tree(parts, x, y, z, 1.6 + rnd.random() * 1.2, c)
        if b1 - b0 > 3.0:
            for k in range(n):
                x = a0 + (k + 1.0) * (a1 - a0) / (n + 1)
                parts.append(block('lounger', x - 0.35, x + 0.35, b1 - 2.4, b1 - 0.6, z, z + 0.4, 'wood_deck', coll=c))


def street(parts, c, full):
    """Street floor: a pale stone base, shops on the front, the entrance with a canopy to the curb."""
    for s in STAGES:
        if s[4] != 0:
            continue
        x0, x1, y0, y1 = rect(s)
        parts.append(block('stone_base', x0, x1, y0, y1, 0, LOBBY_H, 'concrete_pale', coll=c))
        parts.append(block('band', x0 - 0.2, x1 + 0.2, y0 - 0.2, y1 + 0.2, LOBBY_H, SHAFT0, 'concrete_pale', coll=c))
        if not full or y0 > -HY + 0.1:
            continue
        # shop windows on the street front, lit inside, piers on the bay lines
        parts.append(block('shop_glass', x0 + 0.6, x1 - 0.6, y0 - 0.04, y0 + 0.02, 0.5, LOBBY_H - 0.7, 'lobby_glass', coll=c))
        parts.append(block('shop_in', x0 + 0.6, x1 - 0.6, y0 + 2.8, y0 + 2.9, 0.2, LOBBY_H - 0.4, 'bar_glow', coll=c))
        parts.append(block('shop_cut', x0 + 0.6, x1 - 0.6, y0, y0 + 2.8, 0.45, LOBBY_H - 0.65, 'opening_dark', coll=c))
        for k in range(int(round((x1 - x0) / BAY)) + 1):
            x = x0 + k * BAY
            parts.append(block('pier', x - 0.3, x + 0.3, y0 - 0.25, y0, 0, LOBBY_H, 'concrete_pale', coll=c))
        parts.append(block('awning', x0 + 0.6, x1 - 0.6, y0 - 1.4, y0, LOBBY_H - 0.9, LOBBY_H - 0.6, 'steel_dark', coll=c))
        parts.append(block('awning_light', x0 + 0.8, x1 - 0.8, y0 - 1.3, y0 - 0.2, LOBBY_H - 0.95, LOBBY_H - 0.9, 'lamp_warm', coll=c))
    if not full:
        return
    # the entrance: on the back bar facing the court (A), on the front centre (B); a canopy out to the curb
    ye = 1 * BAY if VAR == 'A' else -HY
    yc = ye - 4.0 if VAR == 'A' else -HY - 3.0
    parts.append(block('door', -1.6, 1.6, ye - 0.06, ye, 0, 3.2, 'lobby_glass', coll=c))
    parts.append(block('door_in', -2.2, 2.2, ye + 2.0, ye + 2.1, 0, 3.6, 'lamp_warm', coll=c))
    parts.append(block('door_surround', -2.2, 2.2, ye - 0.3, ye, 3.2, 4.2, 'concrete_pale', coll=c))
    parts.append(block('canopy', -1.6, 1.6, yc, ye, 3.4, 3.65, 'steel_dark', coll=c))
    parts.append(block('canopy_light', -1.4, 1.4, yc + 0.2, ye - 0.2, 3.35, 3.4, 'lamp_warm', coll=c))
    for x in (-1.5, 1.5):
        parts.append(cyl('canopy_post', x, yc + 0.2, 0, 3.4, 0.06, 'steel_rail', 6, coll=c))
    if VAR == 'A':                                                   # the forecourt garden in the court, with a gate
        cx0, cx1, cy0, cy1 = -4 * BAY, 4 * BAY, -HY, 1 * BAY
        parts.append(block('court_paving', cx0, cx1, cy0, cy1, 0, 0.1, 'limestone_dark', coll=c))
        for sx in (-1, 1):
            parts.append(block('court_lawn', sx * 2.5 if sx > 0 else cx0 + 1.5, cx1 - 1.5 if sx > 0 else -2.5, cy0 + 4.0, cy1 - 2.0, 0.1, 0.2, 'planter', coll=c))
            for k in range(3):
                y = cy0 + 5.5 + k * 5.0
                tree(parts, sx * 7.5, y, 0.2, 3.5 + rnd.random(), c)
            parts.append(block('gate_pier', sx * 3.0 - 0.4, sx * 3.0 + 0.4, cy0 - 0.4, cy0 + 0.4, 0, 3.0, 'concrete_pale', coll=c))
            parts.append(block('gate_lamp', sx * 3.0 - 0.25, sx * 3.0 + 0.25, cy0 - 0.25, cy0 + 0.25, 3.0, 3.5, 'lamp_warm', coll=c))
            parts.append(block('fence', sx * 3.4 if sx > 0 else cx0, cx1 if sx > 0 else -3.4, cy0 - 0.05, cy0 + 0.05, 0, 1.4, 'steel_dark', coll=c))


def balconies(parts, c):
    """Old-style steel balconies: a steel plate and a bar rail, one bay wide, every floor of the lower stage."""
    s0 = STAGES[0]
    for (xr, xs) in BALC:
        top = min(s[5] for s in STAGES if s[4] == 0 and s[0] <= xr[0] and s[1] >= xr[1]) if VAR == 'B' else 16
        for xb in xs:
            x = xb * BAY
            for f in range(2, top - 1):
                z = zf(f)
                y = -HY
                parts.append(block('balc', x - 1.4, x + 1.4, y - 1.1, y, z - 0.1, z, 'steel_dark', coll=c))
                parts.append(block('balc_rail', x - 1.4, x + 1.4, y - 1.1, y - 1.05, z + 0.95, z + 1.0, 'steel_dark', coll=c))
                for k in range(8):
                    xx = x - 1.35 + k * 2.7 / 7
                    parts.append(block('balc_bar', xx - 0.02, xx + 0.02, y - 1.1, y - 1.06, z, z + 0.95, 'steel_dark', coll=c))
                for xx in (x - 1.4, x + 1.37):
                    parts.append(block('balc_side', xx, xx + 0.03, y - 1.1, y, z + 0.95, z + 1.0, 'steel_dark', coll=c))


def tank(parts, x, y, z, c, full):
    """A New York wooden water tank: four steel legs, a platform, the staved drum with hoops, a conical cap."""
    for (dx, dy) in ((-1.6, -1.6), (1.6, -1.6), (-1.6, 1.6), (1.6, 1.6)):
        parts.append(block('tank_leg', x + dx - 0.1, x + dx + 0.1, y + dy - 0.1, y + dy + 0.1, z, z + 3.6, 'steel_dark', coll=c))
    parts.append(block('tank_plat', x - 2.0, x + 2.0, y - 2.0, y + 2.0, z + 3.6, z + 3.8, 'steel_dark', coll=c))
    parts.append(cyl('tank', x, y, z + 3.8, z + 8.0, 2.1, 'tank_wood', 14 if full else 8, coll=c))
    for k, (r, h) in enumerate(((1.9, 0.35), (1.3, 0.35), (0.6, 0.35))):
        zz = z + 8.0 + k * 0.35
        parts.append(cyl('tank_cap', x, y, zz, zz + h, r, 'tank_wood', 12 if full else 6, coll=c))
    if full:
        for zz in (4.4, 5.5, 6.6, 7.6):
            parts.append(cyl('tank_hoop', x, y, z + zz, z + zz + 0.08, 2.14, 'steel_dark', 14, coll=c))
        parts.append(block('tank_ladder', x + 2.1, x + 2.2, y - 0.25, y + 0.25, z + 3.8, z + 8.0, 'steel_rail', coll=c))


def roof(parts, c, full):
    """The top roof: elevator bulkhead, water tanks, machinery, a small roof deck with string lights."""
    s = [o for o in STAGES if o[5] == FLOORS][0]
    x0, x1, y0, y1 = rect(s)
    z = zf(FLOORS) + 0.25
    bx = 0.0 if VAR == 'B' else 2.0
    parts.append(block('bulkhead', bx - 3.0, bx + 3.0, y1 - 6.0, y1 - 1.0, z, z + 4.2, 'brick_cornice', coll=c))
    parts.append(block('bulk_cap', bx - 3.2, bx + 3.2, y1 - 6.2, y1 - 0.8, z + 4.2, z + 4.5, 'concrete_pale', coll=c))
    for (tx, ty) in TANKS:
        tank(parts, tx * BAY, ty * BAY, z, c, full)
    if not full:
        return
    for k in range(3):                                               # machinery: condensers along one edge
        x = x0 + 2.5 + k * 3.2
        parts.append(block('ac', x - 1.2, x + 1.2, y0 + 1.0, y0 + 2.6, z, z + 1.4, 'roof_metal', coll=c))
        parts.append(cyl('ac_fan', x, y0 + 1.8, z + 1.4, z + 1.5, 0.6, 'steel_dark', 8, coll=c))
    # the residents' roof deck: decking, planters, a pergola frame with string lights
    dx0, dx1, dy0, dy1 = x1 - 12.0, x1 - 1.0, y0 + 1.0, y0 + 7.0
    parts.append(block('roof_deck', dx0, dx1, dy0, dy1, z, z + 0.1, 'wood_deck', coll=c))
    for x in (dx0, dx1):
        for y in (dy0, dy1):
            parts.append(block('pg_post', x - 0.08, x + 0.08, y - 0.08, y + 0.08, z, z + 2.6, 'wood_deck', coll=c))
    for y in (dy0, dy1):
        parts.append(block('pg_beam', dx0, dx1, y - 0.08, y + 0.08, z + 2.6, z + 2.75, 'wood_deck', coll=c))
        for k in range(12):
            x = dx0 + (k + 0.5) * (dx1 - dx0) / 12
            parts.append(block('bulb', x - 0.05, x + 0.05, y - 0.05, y + 0.05, z + 2.45, z + 2.55, 'string_light', coll=c))
    for k in range(3):
        x = dx0 + 1.5 + k * 4.0
        parts.append(block('table', x - 0.5, x + 0.5, (dy0 + dy1) / 2 - 0.5, (dy0 + dy1) / 2 + 0.5, z + 0.1, z + 0.85, 'wood_deck', coll=c))
    tree(parts, dx0 + 0.6, dy1 - 0.6, z + 0.1, 1.8, c)
    tree(parts, dx1 - 0.6, dy1 - 0.6, z + 0.1, 1.8, c)


def lod_near(c):
    parts = []
    street(parts, c, True); masonry(parts, c, True); balconies(parts, c); roof(parts, c, True)
    return parts


def lod_mid(c):
    parts = []
    street(parts, c, False); masonry(parts, c, False); roof(parts, c, False)
    return parts


def lod_far(c):
    parts = []
    for s in STAGES:
        x0, x1, y0, y1 = rect(s)
        parts.append(block('mass', x0, x1, y0, y1, 0 if s[4] == 0 else zf(s[4]), zf(s[5]) + 0.25, 'MAT-02', UV, c))
    z = zf(FLOORS) + 0.25
    for (tx, ty) in TANKS:
        x, y = tx * BAY, ty * BAY
        parts.append(block('tank', x - 2.0, x + 2.0, y - 2.0, y + 2.0, z + 3.6, z + 8.6, 'tank_wood', coll=c))
    return parts


def sockets(lod, c):
    s = [L.socket('SOCKET_base', (0, 0, 0), c),
         L.socket('SOCKET_door', (0, (1 * BAY if VAR == 'A' else -HY) - 0.5, 0), c),
         L.socket('SOCKET_roof_deck', (HX - 6.5, -HY + 4.0 if VAR == 'B' else 1 * BAY + 4.0, zf(FLOORS) + 0.35), c)]
    for i, st in enumerate(STAGES):                                  # floor sockets: every stage roof / terrace
        x0, x1, y0, y1 = rect(st)
        s.append(L.socket(f'SOCKET_floor_{st[5]:02d}_{i}', ((x0 + x1) / 2, (y0 + y1) / 2, zf(st[5]) + 0.25), c))
    return s


stages = [{'hx': (s[1] - s[0]) * BAY / 2, 'hy': (s[3] - s[2]) * BAY / 2, 'cx': (s[0] + s[1]) * BAY / 2,
           'cy': (s[2] + s[3]) * BAY / 2, 'f0': s[4], 'f1': s[5]} for s in STAGES]
results = {'asset_id': ASSET, 'version': '1.0-mc02', 'variant': VAR, 'units': 'm', 'origin': 'bottom-centre of the street floor',
           'family': 'residential', 'facade': 'residential', 'bays': [NX, NY], 'bay_m': BAY,
           'floor_m': FLOOR, 'floors': FLOORS, 'mechanical_floor': None, 'skip': [], 'lobby_h_m': LOBBY_H,
           'shaft_base_m': SHAFT0, 'roof_m': zf(FLOORS) + 1.1, 'tank_top_m': zf(FLOORS) + 0.25 + 9.05,
           'crown_m': 10.0, 'bounds_m': {'x': [-HX - 0.5, HX + 0.5], 'y': [-HY - 3.0, HY + 0.5], 'z': [0, zf(FLOORS) + 9.3]},
           'stages': stages}
L.build(ASSET, OUT, {'near': lod_near, 'mid': lod_mid, 'far': lod_far}, results, sockets,
        dict(tiles=TILES, tile='residential', mat='MAT-02', cam=(90, -120, 70), look=(0, 0, 35)))

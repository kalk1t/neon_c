"""Living Megacity MC02: BLD-11 Pencil tower (Blender 5.2, headless).

  blender -b --factory-startup --python bld11_pencil.py -- <tiles dir> <outdir>

New York point of view (NYC-REFERENCE.md): 432 Park Avenue (Rafael Vinoly, 2015, ~426 m on a ~28 m square): a sheer
square shaft of white concrete, the same square window in a square grid on every face (MAT-07, 4.75 m bay x 4.75 m
floor, 3.05 m panes), open double-height mechanical / wind floors every twelve storeys where the wind passes through
the grid and the city shows through the building, and an open crown whose top wind floor is lit. Slenderness ~1:15.
Here: a 28.5 m square (6 x 6 bays), 88 floor lines on a 12 m glass lobby: seven occupied blocks of twelve floors
(the top one two), six wind floors (12-13, 26-27, 40-41, 54-55, 68-69, 82-83), the lit open crown (86-87), roof 430 m.
A low podium (40 x 36 m) with glass shops and a roof garden round the tower foot; no balconies (sheer).
Writes BLD-11_<lod>.glb, BLD-11.json (stages, skip: wind and crown floors), BLD-11.blend, review.
"""
import sys, os, math, random
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L
from lib_bld import block, cyl, ring, tree

A = sys.argv[sys.argv.index('--') + 1:]
TILES, OUT = os.path.abspath(A[0]), os.path.abspath(A[1])
BAY = FLOOR = 4.75
NB = 6
H = NB * BAY / 2                                # 14.25: half the tower side
SHAFT0 = 12.0                                   # lobby height: the first floor line
WIND = [12, 26, 40, 54, 68, 82]                 # each the first of two open floors
CROWN = 86                                      # floors 86-87: the open, lit crown
NF = 88
TOP = SHAFT0 + NF * FLOOR                       # 430 m
UV = (BAY, FLOOR, SHAFT0)
PX, PY, PH = 20.0, 18.0, 12.0                   # podium half sizes and height
CORE = 5.0                                      # half size of the core seen through the wind floors
rnd = random.Random(11)
L.start()
L.common_mats()
L.mat('wind_glow', (0.85, 0.9, 1.0), 0.6, 0.0, (0.75, 0.85, 1.0), 0.35)
L.mat('crown_glow', (1.0, 0.92, 0.8), 0.6, 0.0, (1.0, 0.85, 0.65), 0.9)
L.mat('concrete_white_d', (0.6, 0.59, 0.57), 0.7, 0.0)


def zf(f):
    return SHAFT0 + f * FLOOR


def blocks():
    """Occupied floor ranges [a, b) between the open floors."""
    out, f = [], 0
    for w in WIND + [CROWN]:
        out.append((f, w))
        f = w + 2
    return out


def open_floor(parts, c, f, full, crown=False):
    """Two open floors: the concrete grid goes on (columns on every bay line, the slab bars), the core and the plant
    inside are seen through it, a soft light under the slabs (the crown: warm and bright)."""
    z0, z1 = zf(f), zf(f + 2)
    glow = 'crown_glow' if crown else 'wind_glow'
    if not full:                                                          # mid: a dark (or lit) inset, corner columns, the middle bar
        parts.append(block('w_in', -H + 0.5, H - 0.5, -H + 0.5, H - 0.5, z0, z1, 'crown_glow' if crown else 'steel_dark', coll=c))
        for (sx, sy) in ((-1, -1), (1, -1), (-1, 1), (1, 1)):
            parts.append(block('w_corner', min(sx * H, sx * (H - 0.85)), max(sx * H, sx * (H - 0.85)),
                               min(sy * H, sy * (H - 0.85)), max(sy * H, sy * (H - 0.85)), z0, z1, 'concrete_white', coll=c))
        ring(parts, 'w_bar', -H, H, -H, H, zf(f + 1) - 0.85, zf(f + 1) + 0.85, 0.5, 'concrete_white', c)
        return
    # columns: 1.7 m bars on the bay lines (the corners: an L of two half bars), 0.5 m deep like the grid
    for k in range(NB + 1):
        u = -H + k * BAY
        a0, a1 = max(-H, u - 0.85), min(H, u + 0.85)
        for (x0, x1, y0, y1) in ((a0, a1, -H, -H + 0.5), (a0, a1, H - 0.5, H), (-H, -H + 0.5, a0, a1), (H - 0.5, H, a0, a1)):
            parts.append(block('w_col', x0, x1, y0, y1, z0, z1, 'concrete_white', coll=c))
    # slab bars: the lower and upper halves of the floor-line bars (the blocks carry the rest), and the middle one
    for (za, zb) in ((z0, z0 + 0.85), (zf(f + 1) - 0.85, zf(f + 1) + 0.85), (z1 - 0.85, z1)):
        ring(parts, 'w_bar', -H, H, -H, H, za, zb, 0.5, 'concrete_white', c)
    parts.append(block('w_core', -CORE, CORE, -CORE, CORE, z0, z1, 'concrete_white_d', coll=c))
    if not full:
        parts.append(block('w_slab', -H + 0.5, H - 0.5, -H + 0.5, H - 0.5, zf(f + 1) - 0.3, zf(f + 1), 'concrete_white_d', coll=c))
        return
    for ff in (f, f + 1):                                                 # slabs, plant on them, light under them
        z = zf(ff)
        if ff > f:
            parts.append(block('w_slab', -H + 0.5, H - 0.5, -H + 0.5, H - 0.5, z - 0.3, z, 'concrete_white_d', coll=c))
        parts.append(block('w_soffit', -H + 1.5, H - 1.5, -H + 1.5, H - 1.5, z + FLOOR - 0.92, z + FLOOR - 0.9, glow, coll=c))
        if crown:                                                         # the crown floors: a lit warm floor plate
            parts.append(block('crown_floor', -H + 0.5, H - 0.5, -H + 0.5, H - 0.5, z, z + 0.05, 'crown_glow', coll=c))
            continue
        for (x, y) in ((-9.0, -9.0), (9.0, -9.0), (-9.0, 9.0), (9.0, 9.0), (0.0, -9.5), (9.5, 0.0)):
            w, d, h = 1.4 + rnd.random() * 1.6, 1.2 + rnd.random() * 1.2, 1.4 + rnd.random() * 1.4
            parts.append(block('plant', x - w / 2, x + w / 2, y - d / 2, y + d / 2, z, z + h, rnd.choice(['roof_metal', 'steel_dark']), coll=c))
        for (x, y) in ((-6.5, 0.0), (0.0, 6.5)):                          # ducts from the core
            parts.append(block('duct', x - 0.6, x + 0.6, y - 0.6, y + 0.6, z, z + FLOOR - 0.95, 'roof_metal', coll=c))
    parts.append(block('w_core_band', -CORE - 0.02, CORE + 0.02, -CORE - 0.02, CORE + 0.02, zf(f + 1) - 1.2, zf(f + 1) - 1.0, glow, coll=c))
    if crown:                                                             # the crown: lit core faces, a roof slab
        for (x0, x1, y0, y1) in ((-CORE - 0.03, CORE + 0.03, -CORE - 0.03, -CORE), (-CORE - 0.03, CORE + 0.03, CORE, CORE + 0.03),
                                 (-CORE - 0.03, -CORE, -CORE, CORE), (CORE, CORE + 0.03, -CORE, CORE)):
            parts.append(block('crown_core', x0, x1, y0, y1, z0 + 1.0, z1 - 1.5, 'crown_glow', coll=c))


def tower(parts, c, full):
    for (a, b) in blocks():
        parts.append(block('shaft', -H, H, -H, H, zf(a), zf(b), 'MAT-07', UV, c))
    for w in WIND:
        open_floor(parts, c, w, full)
    open_floor(parts, c, CROWN, full, crown=True)
    parts.append(block('roof', -H, H, -H, H, TOP, TOP + 1.2, 'concrete_white', coll=c))
    if full:
        ring(parts, 'roof_rail', -H + 0.6, H - 0.6, -H + 0.6, H - 0.6, TOP + 1.2, TOP + 2.3, 0.08, 'steel_rail', c)
        parts.append(block('bmu', -3.0, 3.0, -1.0, 1.0, TOP + 1.2, TOP + 3.2, 'roof_metal', coll=c))      # window-cleaning rig
        parts.append(block('bmu_arm', -0.4, 0.4, -12.0, 1.0, TOP + 3.2, TOP + 3.8, 'hazard', coll=c))
        for (x, y) in ((-H + 0.5, -H + 0.5), (H - 0.5, -H + 0.5), (-H + 0.5, H - 0.5), (H - 0.5, H - 0.5)):
            parts.append(block('roof_light', x - 0.2, x + 0.2, y - 0.2, y + 0.2, TOP + 1.2, TOP + 1.6, 'red_light', coll=c))


def lobby(parts, c, full):
    """The lobby inside the podium (tower foot) and the podium: glass shops round three sides, a roof garden."""
    parts.append(block('podium', -PX, PX, -PY, PY, 8.0 if full else 0, PH - 0.6, 'concrete_white', coll=c))
    if full:                                                              # ground floor: a core 3 m behind the glass all round
        parts.append(block('pod_core', -PX + 3.1, PX - 3.1, -PY + 3.1, PY - 3.1, 0, 8.0, 'concrete_white_d', coll=c))
        parts.append(block('pod_floor', -PX, PX, -PY, PY, 0, 0.05, 'lobby_stone', coll=c))
        for (sx, sy) in ((-1, -1), (1, -1), (-1, 1), (1, 1)):
            parts.append(block('pod_pier', min(sx * PX, sx * (PX - 1.0)), max(sx * PX, sx * (PX - 1.0)),
                               min(sy * PY, sy * (PY - 1.0)), max(sy * PY, sy * (PY - 1.0)), 0, 8.0, 'concrete_white', coll=c))
    parts.append(block('pod_cap', -PX - 0.3, PX + 0.3, -PY - 0.3, PY + 0.3, PH - 0.6, PH, 'concrete_white_d', coll=c))
    parts.append(block('tower_foot', -H, H, -H, H, PH, SHAFT0, 'concrete_white', coll=c))
    if not full:
        return
    for (x0, x1, y0, y1, s) in ((-PX + 1.0, PX - 1.0, -PY - 0.05, -PY + 0.02, 'y'), (-PX + 1.0, PX - 1.0, PY - 0.02, PY + 0.05, 'y'),
                                (-PX - 0.05, -PX + 0.02, -PY + 1.0, PY - 1.0, 'x'), (PX - 0.02, PX + 0.05, -PY + 1.0, PY - 1.0, 'x')):
        parts.append(block('shop_glass', x0, x1, y0, y1, 0.4, 8.0, 'lobby_glass', coll=c))
    parts.append(block('shop_in_f', -PX + 3.1, PX - 3.1, -PY + 3.05, -PY + 3.1, 0.2, 7.6, 'bar_glow', coll=c))
    for (x0, x1) in ((-PX + 3.05, -PX + 3.1), (PX - 3.1, PX - 3.05)):
        parts.append(block('shop_in_s', x0, x1, -PY + 3.1, PY - 3.1, 0.2, 7.6, 'bar_glow', coll=c))
    parts.append(block('pod_ceiling', -PX + 0.2, PX - 0.2, -PY + 0.2, PY - 0.2, 7.95, 8.0, 'lamp_dim', coll=c))
    for k in range(9):                                                    # mullions on the shop fronts
        x = -PX + 1.0 + k * (2 * PX - 2.0) / 8
        parts.append(block('mull', x - 0.1, x + 0.1, -PY - 0.12, -PY, 0.4, 8.0, 'concrete_white_d', coll=c))
    # the residents' entrance: a deep canopy, a lit lobby behind
    parts.append(block('canopy', -5.0, 5.0, -PY - 5.0, -PY, 5.0, 5.4, 'concrete_white', coll=c))
    parts.append(block('canopy_light', -4.8, 4.8, -PY - 4.8, -PY - 0.2, 4.95, 5.0, 'lamp_warm', coll=c))
    parts.append(block('lobby_door', -2.5, 2.5, -PY - 0.08, -PY - 0.04, 0, 3.6, 'lobby_glass', coll=c))
    # roof garden round the tower foot: lawn, hedges, trees, lamps, a parapet with a glass rail
    z = PH
    parts.append(block('garden', -PX + 0.6, PX - 0.6, -PY + 0.6, PY - 0.6, z, z + 0.15, 'planter', coll=c))
    ring(parts, 'pod_rail', -PX, PX, -PY, PY, z, z + 1.2, 0.08, 'rail_glass', c)
    for x in (-PX + 2.5, -PX + 8.5, PX - 8.5, PX - 2.5):
        for y in (-PY + 2.0, PY - 2.0):
            tree(parts, x, y, z + 0.15, 3.0 + rnd.random(), c)
    for (x, y) in ((-PX + 2.0, 0), (PX - 2.0, 0)):
        tree(parts, x, y, z + 0.15, 3.0 + rnd.random(), c)
    for (x, y) in ((-H - 1.5, -H - 1.5), (H + 1.5, -H - 1.5), (-H - 1.5, H + 1.5), (H + 1.5, H + 1.5)):
        parts.append(cyl('post', x, y, z, z + 3.0, 0.05, 'steel_dark', 6, coll=c))
        parts.append(block('post_lamp', x - 0.15, x + 0.15, y - 0.15, y + 0.15, z + 3.0, z + 3.25, 'lamp_warm', coll=c))


def lod_near(c):
    parts = []
    lobby(parts, c, True); tower(parts, c, True)
    return parts


def lod_mid(c):
    parts = []
    lobby(parts, c, False); tower(parts, c, False)
    return parts


def lod_far(c):
    parts = [block('podium', -PX, PX, -PY, PY, 0, PH, 'concrete_white', coll=c),
             block('foot', -H, H, -H, H, PH, SHAFT0, 'concrete_white', coll=c)]
    for (a, b) in blocks():
        parts.append(block('shaft', -H, H, -H, H, zf(a), zf(b), 'MAT-07', UV, c))
    for w in WIND:
        parts.append(block('wind', -H + 0.5, H - 0.5, -H + 0.5, H - 0.5, zf(w), zf(w + 2), 'steel_dark', coll=c))
    parts.append(block('crown', -H + 0.5, H - 0.5, -H + 0.5, H - 0.5, zf(CROWN), TOP, 'crown_glow', coll=c))
    parts.append(block('roof', -H, H, -H, H, TOP, TOP + 1.2, 'concrete_white', coll=c))
    return parts


def sockets(lod, c):
    s = [L.socket('SOCKET_base', (0, 0, 0), c), L.socket('SOCKET_door', (0, -PY - 0.5, 0), c),
         L.socket('SOCKET_podium_garden', (0, -PY + 2.0, PH + 0.15), c), L.socket('SOCKET_roof', (0, 0, TOP + 1.2), c)]
    for w in WIND + [CROWN]:
        s.append(L.socket(f'SOCKET_wind_{w:02d}', (0, 0, zf(w)), c))
    return s


skip = sorted([f for w in WIND + [CROWN] for f in (w, w + 1)])     # floors above the last stage need skipping too
results = {'asset_id': 'BLD-11', 'version': '1.0-mc02', 'units': 'm', 'origin': 'bottom-centre of the lobby floor',
           'family': 'pencil', 'facade': 'pencil (MAT-07)', 'bays': [NB, NB], 'bay_m': BAY, 'floor_m': FLOOR,
           'floors': NF, 'skip': skip, 'wind_floors': [[w, w + 1] for w in WIND], 'crown_floors': [CROWN, CROWN + 1],
           'lobby_h_m': SHAFT0, 'shaft_base_m': SHAFT0, 'roof_m': TOP + 1.2, 'slenderness': round((TOP + 1.2) / (2 * H), 1),
           'crown_m': 2 * FLOOR + 1.2,
           'stages': [{'hx': H, 'hy': H, 'cx': 0.0, 'cy': 0.0, 'f0': 0, 'f1': CROWN}],
           'solids': [{'x0': -PX, 'x1': PX, 'y0': -PY, 'y1': PY, 'z0': 0.0, 'z1': PH}]}
L.build('BLD-11', OUT, {'near': lod_near, 'mid': lod_mid, 'far': lod_far}, results, sockets,
        dict(tiles=TILES, tile='pencil', mat='MAT-07', cam=(150, -260, 260), look=(0, 0, 230)))

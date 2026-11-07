"""Living Megacity MC02: BLD-09 Utility / maintenance tower (Blender 5.2, headless).

  blender -b --factory-startup --python bld09_utility.py -- <tiles dir> <outdir>

New York point of view (NYC-REFERENCE.md): 33 Thomas Street (John Carl Warnecke, 1974, ~170 m): a windowless
telephone exchange in granite-clad precast, its mass carried by projecting service shafts, with huge vents at the
equipment floors. Here: a square MAT-06 utility block (28 equipment "floors" of 5.5 m) with six projecting riser and
stair shafts that rise past the roof, two two-storey louvre banks (floors 9-10 and 24-25) recessed between them, panel
joints, a single lit operations strip (floor 12, front), two duct risers up the back from the base, and a maintenance
base: garage doors with sodium lamps, a loading apron, cooling units and a gantry on its roof, a guard booth. On the
roof: an antenna and dish platform (lattice mast, three dishes, whips with red lights).
No real interiors (interiors: false); colliders from 'solids'.
Writes BLD-09_<lod>.glb, BLD-09.json, BLD-09.blend, review.
"""
import sys, os, math, random
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L
from lib_bld import block, cyl, ring

A = sys.argv[sys.argv.index('--') + 1:]
TILES, OUT = os.path.abspath(A[0]), os.path.abspath(A[1])
FL, NF = 5.5, 28
BASE_H = 10.0
H = 15.0                                       # tower half size (30 x 30 m)
TOP = BASE_H + NF * FL
LOUVRE = [(9, 11), (24, 26)]                   # floor ranges (end exclusive) of the louvre banks
OPS = 12                                       # the lit operations floor
BX, BY = 23.0, 20.0                            # maintenance base half sizes
# projecting shafts: (x, y) centre on the face line, half width along the face, projection
SHAFTS = [(-H + 3.0, -H, 2.5, 2.5), (H - 3.0, -H, 2.5, 2.5), (-H + 3.0, H, 2.5, 2.5), (H - 3.0, H, 2.5, 2.5),
          (-H, 0.0, 3.0, 3.0), (H, 0.0, 3.0, 3.0)]
rnd = random.Random(9)
L.start()
L.common_mats()
L.mat('MAT-06', (0.38, 0.33, 0.31), 0.7, 0.0)                  # granite-faced precast (33 Thomas Street's pinkish grey)
L.mat('MAT-06_dark', (0.24, 0.21, 0.2), 0.75, 0.0)
L.mat('joint', (0.12, 0.11, 0.11), 0.9, 0.0)
L.mat('ops_glow', (0.6, 0.85, 1.0), 0.5, 0.0, (0.5, 0.8, 1.0), 0.7)
L.mat('screen_g', (0.2, 0.9, 0.6), 0.5, 0.0, (0.2, 0.9, 0.6), 2.0)
L.mat('dish', (0.82, 0.83, 0.84), 0.4, 0.2)


def zf(f):
    return BASE_H + f * FL


def body(parts, c, full):
    # the block, cut by the louvre banks (recessed 1.5 m) on all four faces
    f = 0
    for a, b in LOUVRE + [(NF, NF)]:
        if a > f:
            parts.append(block('body', -H, H, -H, H, zf(f), zf(a), 'MAT-06', coll=c))
        if b > a:
            parts.append(block('louvre_core', -H + 1.5, H - 1.5, -H + 1.5, H - 1.5, zf(a), zf(b), 'louvre_dark', coll=c))
            if full:
                z = zf(a) + 0.3
                while z < zf(b) - 0.2:
                    ring(parts, 'louvre', -H + 0.6, H - 0.6, -H + 0.6, H - 0.6, z, z + 0.12, 0.25, 'steel_rail', c)
                    z += 0.5
                for k in range(1, 6):                                    # mullions in the vents
                    u = -H + k * (2 * H) / 6
                    for (x0, x1, y0, y1) in ((u - 0.15, u + 0.15, -H + 0.4, -H + 1.0), (u - 0.15, u + 0.15, H - 1.0, H - 0.4),
                                             (-H + 0.4, -H + 1.0, u - 0.15, u + 0.15), (H - 1.0, H - 0.4, u - 0.15, u + 0.15)):
                        parts.append(block('vent_mull', x0, x1, y0, y1, zf(a), zf(b), 'MAT-06_dark', coll=c))
        f = b
    parts.append(block('roof', -H, H, -H, H, TOP, TOP + 1.2, 'MAT-06_dark', coll=c))
    if not full:
        return
    # panel joints: a groove at every floor line, vertical joints every 3 m (skipping the vents)
    for k in range(1, NF):
        if any(a <= k <= b for a, b in LOUVRE):
            continue
        ring(parts, 'joint_h', -H - 0.02, H + 0.02, -H - 0.02, H + 0.02, zf(k) - 0.06, zf(k) + 0.06, 0.04, 'joint', c)
    for k in range(1, 10):
        u = -H + k * 3.0
        for (x0, x1, y0, y1) in ((u - 0.04, u + 0.04, -H - 0.02, -H + 0.02), (u - 0.04, u + 0.04, H - 0.02, H + 0.02),
                                 (-H - 0.02, -H + 0.02, u - 0.04, u + 0.04), (H - 0.02, H + 0.02, u - 0.04, u + 0.04)):
            parts.append(block('joint_v', x0, x1, y0, y1, zf(0), TOP, 'joint', coll=c))
    # the operations strip: one lit ribbon window on the front, consoles seen against the light
    za, zb = zf(OPS) + 1.2, zf(OPS) + 3.4
    parts.append(block('ops_glass', -H + 6.0, H - 6.0, -H - 0.05, -H + 0.02, za, zb, 'lobby_glass', coll=c))
    parts.append(block('ops_in', -H + 6.0, H - 6.0, -H + 4.0, -H + 4.1, zf(OPS), zf(OPS + 1), 'ops_glow', coll=c))
    parts.append(block('ops_cut', -H + 6.0, H - 6.0, -H, -H + 4.0, za - 0.02, zb + 0.02, 'opening_dark', coll=c))
    for k in range(7):
        x = -H + 7.5 + k * 2.5
        parts.append(block('console', x - 0.9, x + 0.9, -H + 1.6, -H + 2.2, zf(OPS), zf(OPS) + 1.9, 'steel_dark', coll=c))
        parts.append(block('console_scr', x - 0.7, x + 0.7, -H + 1.55, -H + 1.6, zf(OPS) + 1.3, zf(OPS) + 1.8, 'screen_g', coll=c))
    ring(parts, 'ops_sill', -H + 6.0, H - 6.0, -H - 0.3, -H + 0.05, za - 0.15, za, 0.3, 'MAT-06_dark', c, sides='s')


def shafts(parts, c, full):
    for (x, y, w, p) in SHAFTS:
        top = TOP + (9.0 if abs(x) < 1 else 6.0)
        if abs(y) == H:                                                # on the front / back faces
            s = 1 if y > 0 else -1
            parts.append(block('shaft', x - w, x + w, min(y, y + s * p), max(y, y + s * p), BASE_H, top, 'MAT-06', coll=c))
            if full:
                parts.append(block('shaft_cap', x - w - 0.3, x + w + 0.3, min(y, y + s * (p + 0.3)), max(y, y + s * (p + 0.3)), top, top + 0.6, 'MAT-06_dark', coll=c))
                for k in range(0, NF, 4):                              # stair slots: tiny lit openings, every fourth floor
                    z = zf(k) + 2.0
                    parts.append(block('slot', x - 0.15, x + 0.15, min(y + s * p, y + s * (p + 0.02)), max(y + s * p, y + s * (p + 0.02)), z, z + 1.6, 'lamp_dim', coll=c))
        else:                                                          # on the side faces (the big riser shafts)
            s = 1 if x > 0 else -1
            parts.append(block('shaft', min(x, x + s * p), max(x, x + s * p), y - w, y + w, BASE_H, top, 'MAT-06', coll=c))
            if full:
                parts.append(block('shaft_cap', min(x, x + s * (p + 0.3)), max(x, x + s * (p + 0.3)), y - w - 0.3, y + w + 0.3, top, top + 0.6, 'MAT-06_dark', coll=c))
                for k in range(NF):                                    # vents up the riser, every floor
                    z = zf(k) + 4.4
                    parts.append(block('riser_vent', min(x + s * p, x + s * (p + 0.05)), max(x + s * p, x + s * (p + 0.05)), y - 1.6, y + 1.6, z, z + 0.6, 'louvre_dark', coll=c))


def base(parts, c, full):
    parts.append(block('base', -BX, BX, -BY, BY, 0, BASE_H, 'concrete_struct', coll=c))
    parts.append(block('base_cap', -BX - 0.3, BX + 0.3, -BY - 0.3, BY + 0.3, BASE_H - 0.6, BASE_H, 'MAT-06_dark', coll=c))
    if not full:
        return
    # garage doors on the front with sodium lamps over them, a hazard-striped apron
    for k in range(4):
        x = -BX + 5.0 + k * 8.5
        parts.append(block('door', x - 2.6, x + 2.6, -BY - 0.05, -BY + 0.02, 0, 5.2, 'steel_rail', coll=c))
        for j in range(8):
            z = 0.6 * (j + 1)
            parts.append(block('door_rib', x - 2.6, x + 2.6, -BY - 0.09, -BY - 0.05, z - 0.04, z + 0.04, 'steel_dark', coll=c))
        parts.append(block('door_lamp', x - 0.4, x + 0.4, -BY - 0.5, -BY, 5.6, 5.9, 'lamp_sodium', coll=c))
        parts.append(block('door_frame', x - 2.8, x + 2.8, -BY - 0.12, -BY, 5.2, 5.4, 'hazard', coll=c))
    parts.append(block('apron', -BX, BX, -BY - 8.0, -BY, 0, 0.05, 'concrete_struct', coll=c))
    for k in range(12):
        x = -BX + 1.0 + k * 4.0
        parts.append(block('apron_stripe', x, x + 1.4, -BY - 8.0, -BY - 7.6, 0.05, 0.07, 'hazard', coll=c))
    parts.append(block('booth', BX - 4.0, BX - 1.0, -BY - 6.0, -BY - 3.0, 0, 2.8, 'concrete_pale', coll=c))
    parts.append(block('booth_glass', BX - 4.05, BX - 0.95, -BY - 6.05, -BY - 5.95, 1.0, 2.4, 'ops_glow', coll=c))
    # cooling units and a gantry on the base roof, two duct risers up the back of the tower
    z = BASE_H
    for (x, y) in ((-BX + 5.0, -BY + 3.0), (-BX + 11.0, -BY + 3.0), (BX - 11.0, -BY + 3.0), (BX - 5.0, -BY + 3.0)):
        parts.append(block('cooler', x - 2.4, x + 2.4, y - 1.5, y + 1.5, z, z + 2.4, 'roof_metal', coll=c))
        for dx in (-1.2, 1.2):
            parts.append(cyl('fan', x + dx, y, z + 2.4, z + 2.6, 0.9, 'steel_dark', 10, coll=c))
    for x in (-BX + 2.0, BX - 2.0):
        parts.append(block('gantry_leg', x - 0.2, x + 0.2, BY - 3.0, BY - 2.6, z, z + 6.0, 'hazard', coll=c))
    parts.append(block('gantry_beam', -BX + 1.8, BX - 1.8, BY - 3.1, BY - 2.5, z + 6.0, z + 6.6, 'hazard', coll=c))
    for x in (-7.0, 7.0):
        parts.append(block('duct', x - 1.1, x + 1.1, H, H + 2.2, z, zf(LOUVRE[0][0]) + 2.0, 'roof_metal', coll=c))
        for k in range(0, int(zf(LOUVRE[0][0]) - z), 6):
            parts.append(block('duct_band', x - 1.2, x + 1.2, H, H + 2.3, z + k, z + k + 0.25, 'steel_dark', coll=c))
    for (x, y) in ((-BX + 0.5, -BY + 0.5), (BX - 0.5, -BY + 0.5)):
        parts.append(block('base_lamp', x - 0.3, x + 0.3, y - 0.6, y, 7.0, 7.4, 'lamp_sodium', coll=c))


def roof(parts, c, full):
    """Antenna and dish platform: steel deck on the roof, a lattice mast, three dishes, whip antennas."""
    z = TOP + 1.2
    parts.append(block('platform', -10.0, 10.0, -10.0, 10.0, z + 2.0, z + 2.4, 'steel_dark', coll=c))
    for (x, y) in ((-9.5, -9.5), (9.5, -9.5), (-9.5, 9.5), (9.5, 9.5)):
        parts.append(block('plat_leg', x - 0.2, x + 0.2, y - 0.2, y + 0.2, z, z + 2.0, 'steel_dark', coll=c))
    zp = z + 2.4
    hm = 32.0
    if full:
        ring(parts, 'plat_rail', -10.0, 10.0, -10.0, 10.0, zp, zp + 1.1, 0.06, 'hazard', c)
        for (x, y) in ((-1.2, -1.2), (1.2, -1.2), (-1.2, 1.2), (1.2, 1.2)):         # lattice mast legs + braces
            parts.append(cyl('mast_leg', x, y, zp, zp + hm, 0.18, 'steel_rail', 6, coll=c))
        for k in range(int(hm / 3)):
            zz = zp + 1.5 + k * 3.0
            ring(parts, 'mast_brace', -1.3, 1.3, -1.3, 1.3, zz, zz + 0.15, 0.12, 'steel_rail', c)
        parts.append(block('mast_light', -0.3, 0.3, -0.3, 0.3, zp + hm, zp + hm + 0.5, 'red_light', coll=c))
        # dishes: a post, a dish turned up and out
        for (x, y, a) in ((-6.5, -6.0, 0.6), (6.5, -5.5, -0.5), (6.0, 6.5, 2.6)):
            parts.append(cyl('dish_post', x, y, zp, zp + 2.0, 0.15, 'steel_dark', 6, coll=c))
            d = cyl('dish', 0, 0, -0.25, 0.25, 2.0, 'dish', 16, coll=c)
            d.location = (x, y, zp + 3.2)
            d.rotation_euler = (math.radians(55), 0, a)
            parts.append(d)
            f = cyl('feed', 0, 0, 0.25, 1.6, 0.06, 'steel_dark', 6, coll=c)
            f.location = (x, y, zp + 3.2)
            f.rotation_euler = (math.radians(55), 0, a)
            parts.append(f)
        for (x, y, h) in ((-8.5, 8.5, 12.0), (8.5, 8.5, 9.0), (-8.5, -8.5, 7.0)):    # whip antennas
            parts.append(cyl('whip', x, y, zp, zp + h, 0.05, 'steel_rail', 5, coll=c))
            parts.append(block('whip_light', x - 0.15, x + 0.15, y - 0.15, y + 0.15, zp + h, zp + h + 0.3, 'red_light', coll=c))
    else:
        parts.append(block('mast', -1.2, 1.2, -1.2, 1.2, zp, zp + hm, 'steel_rail', coll=c))


def lod(full):
    def fn(c):
        parts = []
        base(parts, c, full); body(parts, c, full); shafts(parts, c, full); roof(parts, c, full)
        return parts
    return fn


def lod_far(c):
    parts = [block('base', -BX, BX, -BY, BY, 0, BASE_H, 'concrete_struct', coll=c),
             block('body', -H, H, -H, H, BASE_H, TOP + 1.2, 'MAT-06', coll=c)]
    for (a, b) in LOUVRE:
        parts.append(block('vent', -H - 0.05, H + 0.05, -H - 0.05, H + 0.05, zf(a), zf(b), 'louvre_dark', coll=c))
    parts.append(block('mast', -1.2, 1.2, -1.2, 1.2, TOP + 3.6, TOP + 35.6, 'steel_rail', coll=c))
    return parts


def sockets(lod_, c):
    s = [L.socket('SOCKET_base', (0, 0, 0), c), L.socket('SOCKET_maintenance_door', (-BX + 5.0, -BY - 0.5, 0), c),
         L.socket('SOCKET_roof_platform', (0, 0, TOP + 3.6), c)]
    for i, x in enumerate((-7.0, 7.0)):                                # duct and riser anchors
        s.append(L.socket(f'SOCKET_duct_{i}', (x, H + 1.1, BASE_H), c))
    for i, (x, y, w, p) in enumerate(SHAFTS[4:]):
        s.append(L.socket(f'SOCKET_riser_{i}', (x + (p if x > 0 else -p), y, TOP), c))
    return s


results = {'asset_id': 'BLD-09', 'version': '1.0-mc02', 'units': 'm', 'origin': 'bottom-centre of the base floor',
           'family': 'utility', 'facade': 'MAT-06 (modelled, no window shader)', 'interiors': False,
           'floor_m': FL, 'floors': NF, 'base_h_m': BASE_H, 'roof_m': TOP + 1.2, 'mast_top_m': TOP + 3.6 + 32.5,
           'louvre_floors': [list(range(a, b)) for a, b in LOUVRE], 'operations_floor': OPS,
           'solids': [{'x0': -BX, 'x1': BX, 'y0': -BY, 'y1': BY, 'z0': 0.0, 'z1': BASE_H},
                      {'x0': -H - 3.0, 'x1': H + 3.0, 'y0': -H - 2.5, 'y1': H + 2.5, 'z0': BASE_H, 'z1': TOP + 9.6},
                      {'x0': -10.0, 'x1': 10.0, 'y0': -10.0, 'y1': 10.0, 'z0': TOP + 1.2, 'z1': TOP + 3.6}]}
L.build('BLD-09', OUT, {'near': lod(True), 'mid': lod(False), 'far': lod_far}, results, sockets,
        dict(tiles=TILES, tile='transport', mat='MAT-03', cam=(120, -200, 120), look=(0, 0, 90)))

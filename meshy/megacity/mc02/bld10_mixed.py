"""Living Megacity MC02: BLD-10 Mixed-use commercial block (Blender 5.2, headless).

  blender -b --factory-startup --python bld10_mixed.py -- <tiles dir> <outdir>

New York point of view (NYC-REFERENCE.md): Central Park Tower (Adrian Smith + Gordon Gill, 2020, ~472 m): a
department-store base, the tower above, a cantilever. Here: a six-storey retail base (5 m floors, display glass
between stone piers, lit sales floors, window vignettes, a corner entrance cut out of the block with a wrap-round
canopy and blade signs), then three stacked families, each its own facade and floor height, floor indices counting
through: offices (MAT-01, floors 0-39), a louvred transfer floor (40), a hotel (MAT-05, floors 41-70) cantilevered
5.7 m east over the base, a plant floor (71), apartments (MAT-02, floors 72-101) flush with the cantilevered face, and
a glass rooftop restaurant with a terrace, a plant screen and a mast. Setback terraces: a hotel pool deck on the
office roof, planters on the hotel roof.
Writes BLD-10_<lod>.glb, BLD-10.json (stages with their own 'fam', 'floor_m', 'y0_m'; 'skip'; 'solids'), review.
"""
import sys, os, math, random
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L
from lib_bld import block, cyl, ring, tree

A = sys.argv[sys.argv.index('--') + 1:]
TILES, OUT = os.path.abspath(A[0]), os.path.abspath(A[1])
RET_F, RET_N = 5.0, 6                          # retail floors
RET_H = RET_F * RET_N
BAND_H = 1.0
BX, BY = 22.5, 16.5                            # retail base half sizes
CUT = 9.0                                      # corner entrance cut (front-left corner, two storeys)
Z_OFF = RET_H + BAND_H                         # first office floor
# stages: name, family facade, bay, floor, half x, half y, centre x, f0, f1 (end exclusive), first floor z
ST = []
def stage(name, mat, bay, flo, hx, hy, cx, f0, f1, z0):
    ST.append(dict(name=name, mat=mat, bay=bay, floor=flo, hx=hx, hy=hy, cx=cx, f0=f0, f1=f1, z0=z0))
    return z0 + (f1 - f0) * flo
z = stage('office', 'MAT-01', 3.0, 4.0, 19.5, 12.0, 0.0, 0, 41, Z_OFF)            # floor 40: transfer (louvres)
z = stage('hotel', 'MAT-05', 3.6, 3.2, 16.2, 10.8, 9.0, 41, 72, z)                # floor 71: plant
TOP = stage('residential', 'MAT-02', 3.4, 3.4, 11.9, 8.5, 25.2 - 11.9, 72, 102, z)
SKIP = [40, 71]
FAMS = {'MAT-01': 'office', 'MAT-05': 'hotel', 'MAT-02': 'residential'}
rnd = random.Random(10)
L.start()
L.common_mats()
L.mat('shop_mid', (1.0, 0.9, 0.75), 0.6, 0.0, (1.0, 0.82, 0.6), 0.45)
L.mat('sign_red', (1.0, 0.25, 0.35), 0.5, 0.0, (1.0, 0.2, 0.32), 1.4)
L.mat('sign_band', (1.0, 0.95, 0.88), 0.5, 0.0, (1.0, 0.9, 0.78), 0.8)
L.mat('mannequin', (0.85, 0.83, 0.8), 0.5, 0.0)
L.mat('goods', (0.6, 0.2, 0.25), 0.6, 0.0)
L.mat('lawn', (0.07, 0.2, 0.06), 0.9, 0.0, (0.1, 0.3, 0.08), 0.03)


def zf(s, f):
    return s['z0'] + (f - s['f0']) * s['floor']


def uv(s):
    return (s['bay'], s['floor'], s['z0'] - s['f0'] * s['floor'])


def ext(s):
    return s['cx'] - s['hx'], s['cx'] + s['hx'], -s['hy'], s['hy']


# ---------------------------------------------------------------- retail base
def retail(parts, c, full):
    x0, x1, y0, y1 = -BX, BX, -BY, BY
    if not full:
        parts.append(block('retail', x0, x1, y0, y1, 0, RET_H, 'shop_mid', coll=c))
        parts.append(block('retail_cut', x0, x0 + CUT, y0, y0 + CUT, 0, 2 * RET_F, 'opening_dark', coll=c))
        parts.append(block('band', x0 - 0.3, x1 + 0.3, y0 - 0.3, y1 + 0.3, RET_H, Z_OFF, 'limestone', coll=c))
        return
    # the solid core of the store (inside the lit sales floors) and per floor: slab band, glass, lit floor behind
    parts.append(block('store_core', x0 + 6.0, x1 - 6.0, y0 + 6.0, y1 - 6.0, 0, RET_H, 'concrete_struct', coll=c))
    for k in range(RET_N):
        za, zb = k * RET_F, (k + 1) * RET_F
        cut = k < 2
        lit = 'bar_glow' if k < 2 else 'shop_mid'
        # floor plate + ceiling light lines (slab ring between the glass and the core)
        parts.append(block('plate', x0 + 0.4, x1 - 0.4, y0 + 0.4, y1 - 0.4, za, za + 0.15, 'lobby_stone', coll=c))
        for (a, b, d, e) in ((x0 + 0.5, x1 - 0.5, y0 + 5.9, y0 + 6.0), (x0 + 0.5, x1 - 0.5, y1 - 6.0, y1 - 5.9),
                             (x0 + 5.9, x0 + 6.0, y0 + 0.5, y1 - 0.5), (x1 - 6.0, x1 - 5.9, y0 + 0.5, y1 - 0.5)):
            parts.append(block('salesfloor', a, b, d, e, za + 0.15, zb - 0.4, lit, coll=c))
        ring(parts, 'ceil_light', x0 + 2.5, x1 - 2.5, y0 + 2.5, y1 - 2.5, zb - 0.45, zb - 0.4, 0.15, 'lamp_dim', c)
        # glass on all four sides (the cut corner gets its own glass)
        gl = [(x0 + (CUT if cut else 0), x1, y0, y0 + 0.1), (x0, x1, y1 - 0.1, y1), (x1 - 0.1, x1, y0, y1),
              (x0, x0 + 0.1, y0 + (CUT if cut else 0), y1)]
        for (a, b, d, e) in gl:
            parts.append(block('display_glass', a, b, d, e, za + 0.6, zb - 0.6, 'lobby_glass', coll=c))
        # the stone slab band at each floor line, with the sign band over the ground floor
        ring(parts, 'slab_band', x0 - 0.15, x1 + 0.15, y0 - 0.15, y1 + 0.15, zb - 0.6, zb + 0.6 if k < RET_N - 1 else zb, 0.5, 'limestone', c,
             sides='nsew')
        if cut:
            parts.append(block('cut_glass_x', x0 + CUT - 0.1, x0 + CUT, y0, y0 + CUT, za, zb, 'lobby_glass', coll=c))
            parts.append(block('cut_glass_y', x0, x0 + CUT, y0 + CUT - 0.1, y0 + CUT, za, zb, 'lobby_glass', coll=c))
    # stone piers on the bay lines (4.5 m front / back, 5.5 m sides), full base height
    for k in range(11):
        x = x0 + k * 4.5
        for (ya, yb) in ((y0 - 0.35, y0 + 0.2), (y1 - 0.2, y1 + 0.35)):
            if ya < 0 and x < x0 + CUT + 0.1:
                za = 2 * RET_F                                            # over the cut corner: piers start above it
            else:
                za = 0
            parts.append(block('pier', x - 0.35, x + 0.35, ya, yb, za, RET_H, 'limestone', coll=c))
    for k in range(7):
        y = y0 + k * 5.5
        for (xa, xb) in ((x0 - 0.35, x0 + 0.2), (x1 - 0.2, x1 + 0.35)):
            za = 2 * RET_F if (xa < 0 and y < y0 + CUT + 0.1) else 0
            parts.append(block('pier', xa, xb, y - 0.35, y + 0.35, za, RET_H, 'limestone', coll=c))
    # the cut corner: a stone column on the corner, the soffit lit, a wrap-round canopy, blade signs over it
    parts.append(block('corner_col', x0 - 0.5, x0 + 0.5, y0 - 0.5, y0 + 0.5, 0, 2 * RET_F, 'limestone', coll=c))
    parts.append(block('cut_soffit', x0, x0 + CUT, y0, y0 + CUT, 2 * RET_F - 0.4, 2 * RET_F, 'lobby_ceiling', coll=c))
    parts.append(block('cut_floor', x0, x0 + CUT, y0, y0 + CUT, 0, 0.05, 'lobby_stone', coll=c))
    for (a, b, d, e) in ((x0 - 3.0, x0 + CUT + 6.0, y0 - 3.0, y0 + 0.5), (x0 - 3.0, x0 + 0.5, y0 - 3.0, y0 + CUT + 6.0)):
        parts.append(block('canopy', a, b, d, e, 4.6, 5.0, 'bronze', coll=c))
        parts.append(block('canopy_light', a + 0.2, b - 0.2, d + 0.2, e - 0.2, 4.55, 4.6, 'lamp_dim', coll=c))
    parts.append(block('sign_blade_x', x0 - 1.6, x0 - 0.5, y0 + CUT + 1.0, y0 + CUT + 1.3, 11.0, 26.0, 'sign_red', coll=c))
    parts.append(block('sign_blade_y', x0 + CUT + 1.0, x0 + CUT + 1.3, y0 - 1.6, y0 - 0.5, 11.0, 26.0, 'sign_red', coll=c))
    parts.append(block('sign_front', -6.0, 12.0, y0 - 0.45, y0 - 0.3, 3.4, 4.3, 'sign_band', coll=c))
    # window vignettes on the ground floor front: mannequins and display tables behind the glass
    for k in range(6):
        x = -10.0 + k * 4.0
        if k % 2 == 0:
            for j in (-0.6, 0.6):
                parts.append(cyl('mannequin', x + j, y0 + 1.2, 0.15, 1.75, 0.18, 'mannequin', 8, coll=c))
                parts.append(block('head', x + j - 0.1, x + j + 0.1, y0 + 1.1, y0 + 1.3, 1.75, 2.0, 'mannequin', coll=c))
        else:
            parts.append(block('display_table', x - 1.0, x + 1.0, y0 + 0.8, y0 + 1.8, 0.15, 0.95, 'wood_deck', coll=c))
            for j in range(4):
                parts.append(block('goods', x - 0.8 + j * 0.45, x - 0.5 + j * 0.45, y0 + 1.1, y0 + 1.4, 0.95, 1.2 + 0.1 * j, 'goods', coll=c))
    # escalators seen through the glass in the cut corner
    parts.append(block('escalator', x0 + 6.5, x0 + 7.5, y0 + 6.5, y0 + 12.5, 0.15, RET_F, 'steel_rail', coll=c))
    parts.append(block('band', x0 - 0.3, x1 + 0.3, y0 - 0.3, y1 + 0.3, RET_H, Z_OFF, 'limestone', coll=c))


# ---------------------------------------------------------------- tower
def tower(parts, c, full):
    for s in ST:
        x0, x1, y0, y1 = ext(s)
        cuts = [f for f in SKIP if s['f0'] <= f < s['f1']]
        f = s['f0']
        for m in cuts + [s['f1']]:
            if m > f:
                parts.append(block(s['name'], x0, x1, y0, y1, zf(s, f), zf(s, m), s['mat'], uv(s), c))
            if m < s['f1']:
                za, zb = zf(s, m), zf(s, m + 1)
                parts.append(block('louvre_core', x0 + 0.4, x1 - 0.4, y0 + 0.4, y1 - 0.4, za, zb, 'louvre_dark', coll=c))
                if full:
                    zz = za + 0.3
                    while zz < zb - 0.2:
                        ring(parts, 'lv', x0, x1, y0, y1, zz, zz + 0.06, 0.1, 'steel_rail', c)
                        zz += 0.4
            f = m + 1
        parts.append(block('cornice', x0 - 0.3, x1 + 0.3, y0 - 0.3, y1 + 0.3, zf(s, s['f1']), zf(s, s['f1']) + 0.4, 'limestone', coll=c))
    # the cantilever: lit soffit under the hotel where it runs past the offices
    h = ST[1]
    ox = ext(ST[0])[1]
    if full:
        parts.append(block('soffit', ox, ext(h)[1], -h['hy'], h['hy'], h['z0'] - 0.6, h['z0'], 'steel_dark', coll=c))
        for k in range(4):
            y = -h['hy'] + 2.0 + k * (2 * h['hy'] - 4.0) / 3
            parts.append(block('soffit_light', ox + 0.5, ext(h)[1] - 0.5, y - 0.1, y + 0.1, h['z0'] - 0.65, h['z0'] - 0.6, 'lamp_dim', coll=c))


def terraces(parts, c, full):
    """Hotel pool deck on the office roof (west of the hotel) and planters on the hotel roof (west of the flats)."""
    o, h, r = ST
    z = zf(o, o['f1']) + 0.4
    ox0, ox1, oy0, oy1 = ext(o)
    hx0 = ext(h)[0]
    parts.append(block('deck', ox0, hx0, oy0, oy1, z - 0.1, z, 'wood_deck', coll=c))
    if full:
        ring(parts, 't_rail', ox0, ox1, oy0, oy1, z, z + 1.15, 0.05, 'rail_glass', c, sides='nsw')
        parts.append(block('pool_rim', ox0 + 2.0, hx0 - 2.0, -5.0, 5.0, z, z + 0.3, 'pool_tile', coll=c))
        parts.append(block('pool', ox0 + 2.3, hx0 - 2.3, -4.7, 4.7, z + 0.3, z + 0.31, 'pool_water', coll=c))
        for k in range(4):
            x = ox0 + 2.5 + k * 2.2
            for y in (-7.5, 7.5):
                parts.append(block('lounger', x - 0.35, x + 0.35, y - 1.0, y + 1.0, z, z + 0.4, 'pool_tile', coll=c))
        for y in (oy0 + 1.2, oy1 - 1.2):
            tree(parts, ox0 + 1.2, y, z, 2.4, c)
    z = zf(h, h['f1']) + 0.4
    hx0, hx1, hy0, hy1 = ext(h)
    rx0 = ext(r)[0]
    parts.append(block('deck', hx0, rx0, hy0, hy1, z - 0.1, z, 'limestone_dark', coll=c))
    if full:
        ring(parts, 't_rail', hx0, hx1, hy0, hy1, z, z + 1.15, 0.05, 'rail_glass', c, sides='nsw')
        for y in (hy0 + 1.5, 0.0, hy1 - 1.5):
            tree(parts, hx0 + 1.5, y, z, 2.2, c)
            parts.append(block('bench', hx0 + 3.5, hx0 + 4.0, y - 0.9, y + 0.9, z, z + 0.45, 'wood_deck', coll=c))


def rooftop(parts, c, full):
    """A glass restaurant on the roof of the flats, its terrace with tables and lights, a plant screen, a mast."""
    s = ST[2]
    x0, x1, y0, y1 = ext(s)
    z = zf(s, s['f1']) + 0.4
    parts.append(block('roof', x0, x1, y0, y1, z - 0.1, z, 'wood_deck', coll=c))
    rx0, rx1, ry0, ry1 = x0 + 2.5, x1 - 2.5, y0 + 2.5, y1 - 2.5
    parts.append(block('rest_roof', rx0 - 0.4, rx1 + 0.4, ry0 - 0.4, ry1 + 0.4, z + 8.6, z + 9.0, 'bronze', coll=c))
    parts.append(block('plant', rx0 + 2.0, rx1 - 2.0, ry0 + 1.5, ry1 - 1.5, z + 9.0, z + 14.0, 'louvre_dark', coll=c))
    parts.append(cyl('mast', (x0 + x1) / 2, 0, z + 14.0, z + 38.0, 0.18, 'steel_dark', 6, coll=c))
    if not full:
        parts.append(block('rest', rx0, rx1, ry0, ry1, z, z + 8.6, 'bar_glow', coll=c))
        return
    parts.append(block('mast_light', (x0 + x1) / 2 - 0.25, (x0 + x1) / 2 + 0.25, -0.25, 0.25, z + 38.0, z + 38.5, 'red_light', coll=c))
    ring(parts, 'rest_glass', rx0, rx1, ry0, ry1, z, z + 8.6, 0.06, 'lobby_glass', c)
    parts.append(block('rest_back', rx0 + 3.0, rx1 - 3.0, ry0 + 3.0, ry1 - 3.0, z, z + 8.6, 'bar_glow', coll=c))
    parts.append(block('rest_mezz', rx0 + 0.1, rx1 - 0.1, ry0 + 0.1, ry1 - 0.1, z + 4.2, z + 4.4, 'bronze', coll=c))
    for k in range(5):                                                   # tables along the glass, both levels
        x = rx0 + 1.5 + k * (rx1 - rx0 - 3.0) / 4
        for zz in (z, z + 4.4):
            parts.append(cyl('table', x, ry0 + 1.3, zz, zz + 0.75, 0.45, 'pool_tile', 8, coll=c))
            parts.append(block('table_lamp', x - 0.08, x + 0.08, ry0 + 1.22, ry0 + 1.38, zz + 0.75, zz + 0.95, 'lamp_warm', coll=c))
    ring(parts, 'r_rail', x0, x1, y0, y1, z, z + 1.15, 0.05, 'rail_glass', c)
    for (a, b) in (((x0 + 0.5, y0 + 0.5), (x1 - 0.5, y0 + 0.5)), ((x0 + 0.5, y1 - 0.5), (x1 - 0.5, y1 - 0.5))):
        n = 18
        for k in range(n + 1):
            t = k / n
            x, y = a[0] + (b[0] - a[0]) * t, a[1]
            zz = z + 3.0 - 0.5 * math.sin(math.pi * t)
            parts.append(block('bulb', x - 0.05, x + 0.05, y - 0.05, y + 0.05, zz - 0.05, zz + 0.05, 'string_light', coll=c))


def lod(full):
    def fn(c):
        parts = []
        retail(parts, c, full); tower(parts, c, full); terraces(parts, c, full); rooftop(parts, c, full)
        return parts
    return fn


def lod_far(c):
    parts = [block('retail', -BX, BX, -BY, BY, 0, Z_OFF, 'shop_mid', coll=c)]
    for s in ST:
        x0, x1, y0, y1 = ext(s)
        parts.append(block(s['name'], x0, x1, y0, y1, s['z0'], zf(s, s['f1']) + 0.4, s['mat'], uv(s), c))
    x0, x1, y0, y1 = ext(ST[2])
    parts.append(block('top', x0 + 2.5, x1 - 2.5, y0 + 2.5, y1 - 2.5, TOP + 0.4, TOP + 14.4, 'bar_glow', coll=c))
    return parts


def sockets(lod_, c):
    return [L.socket('SOCKET_base', (0, 0, 0), c),
            L.socket('SOCKET_entrance_corner', (-BX + CUT / 2, -BY + CUT / 2, 0), c),
            L.socket('SOCKET_lobby_prop24', (-BX + CUT + 2.0, -BY + CUT + 2.0, 0.15), c),
            L.socket('SOCKET_canopy', (-BX, -BY, 4.6), c),
            L.socket('SOCKET_sign_corner', (-BX - 1.0, -BY - 1.0, 11.0), c),
            L.socket('SOCKET_sign_front', (3.0, -BY - 0.45, 3.4), c),
            L.socket('SOCKET_roof', ((ext(ST[2])[0] + ext(ST[2])[1]) / 2, 0, TOP + 38.5), c)]


results = {'asset_id': 'BLD-10', 'version': '1.0-mc02', 'units': 'm', 'origin': 'bottom-centre of the retail floor',
           'family': 'office', 'facade': 'office + hotel + residential', 'bays': [13, 8], 'bay_m': 3.0, 'floor_m': 4.0,
           'floors': ST[-1]['f1'], 'mechanical_floor': None, 'skip': SKIP, 'retail_floors': RET_N, 'retail_floor_m': RET_F,
           'shaft_base_m': Z_OFF, 'roof_m': TOP, 'mast_top_m': TOP + 38.9,
           'solids': [{'x0': -BX, 'x1': BX, 'y0': -BY, 'y1': BY, 'z0': 0.0, 'z1': Z_OFF}],
           'stages': [{'hx': s['hx'], 'hy': s['hy'], 'cx': s['cx'], 'cy': 0.0, 'f0': s['f0'], 'f1': s['f1'],
                       'fam': FAMS[s['mat']], 'floor_m': s['floor'], 'y0_m': s['z0']} for s in ST]}
L.build('BLD-10', OUT, {'near': lod(True), 'mid': lod(False), 'far': lod_far}, results, sockets,
        dict(tiles=TILES, tile='office', mat='MAT-01', cam=(260, -330, 230), look=(0, 0, 200)))

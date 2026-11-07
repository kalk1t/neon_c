"""Living Megacity MC02: BLD-03 Twin office assembly (Blender 5.2, headless).

  blender -b --factory-startup --python bld03_twin.py -- <tiles dir> <outdir>

New York point of view (NYC-REFERENCE.md): Time Warner Center (SOM / David Childs, 2003): two glass office shafts of
unequal height on one podium, with a clear gap between them. Here: an 8-floor podium (MAT-01 curtain wall, 3.0 m bay x
4.0 m floor) with a two-storey glass lobby (lit hall, trees, escalators, a long canopy), a roof garden on the podium
(lawns, a reflecting pool, trees, benches), two staggered towers (floors 8-49 and 8-59, a louvred plant floor at 25),
a glass sky bridge on floors 30-31 between them (+ bridge sockets on real floors), and a screened glass crown on each
tower (plant behind fins and glass, a cool glow line at the top).
Writes BLD-03_<lod>.glb, BLD-03.json (incl. 'stages' with tower offsets, 'skip'), BLD-03.blend, review.
"""
import sys, os, math, random
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L
from lib_bld import block, cyl, ring, tree

A = sys.argv[sys.argv.index('--') + 1:]
TILES, OUT = os.path.abspath(A[0]), os.path.abspath(A[1])
BAY, FLOOR = 3.0, 4.0
LOBBY_H, BAND_H = 10.0, 1.0
SHAFT0 = LOBBY_H + BAND_H
UV = (BAY, FLOOR, SHAFT0)
POD = (26, 14, 0, 8)                           # bays x, bays y, floors
TW = [dict(bx=10, by=8, cx=-21.0, cy=-3.0, f0=8, f1=50), dict(bx=10, by=8, cx=21.0, cy=3.0, f0=8, f1=60)]
MECH = 25
BRIDGE = (30, 32)                              # bridge floors (end exclusive)
FLOORS = 60
TOP = SHAFT0 + FLOORS * FLOOR
rnd = random.Random(3)
L.start()
L.common_mats()
L.mat('lawn', (0.07, 0.2, 0.06), 0.9, 0.0, (0.1, 0.3, 0.08), 0.03)
L.mat('crown_glow', (0.8, 0.9, 1.0), 0.5, 0.0, (0.75, 0.88, 1.0), 0.9)


def zf(f):
    return SHAFT0 + f * FLOOR


def ext(t):
    hx, hy = t['bx'] * BAY / 2, t['by'] * BAY / 2
    return t['cx'] - hx, t['cx'] + hx, t['cy'] - hy, t['cy'] + hy


def podium(parts, c, full):
    hx, hy = POD[0] * BAY / 2, POD[1] * BAY / 2
    parts.append(block('podium', -hx, hx, -hy, hy, zf(0), zf(POD[3]), 'MAT-01', UV, c))
    parts.append(block('pod_cap', -hx - 0.3, hx + 0.3, -hy - 0.3, hy + 0.3, zf(POD[3]), zf(POD[3]) + 0.5, 'fin_alu', coll=c))
    parts.append(block('band', -hx - 0.1, hx + 0.1, -hy - 0.1, hy + 0.1, LOBBY_H, SHAFT0, 'steel_dark', coll=c))
    if not full:
        parts.append(block('lobby', -hx, hx, -hy, hy, 0, LOBBY_H, 'lobby_glass', coll=c))
        parts.append(block('lobby_in', -hx + 3, hx - 3, -hy + 3, hy - 3, 0, LOBBY_H, 'bar_glow', coll=c))
        return
    # the lobby: a glass box set 3 m back behind slim columns, a lit hall with trees and escalators
    parts.append(block('lobby_glass', -hx + 3.0, hx - 3.0, -hy + 3.0, -hy + 3.1, 0, LOBBY_H, 'lobby_glass', coll=c))
    parts.append(block('lobby_glass_b', -hx + 3.0, hx - 3.0, hy - 3.1, hy - 3.0, 0, LOBBY_H, 'lobby_glass', coll=c))
    for x in (-hx + 3.0, hx - 3.1):
        parts.append(block('lobby_glass_s', x, x + 0.1, -hy + 3.0, hy - 3.0, 0, LOBBY_H, 'lobby_glass', coll=c))
    parts.append(block('lobby_floor', -hx + 3.0, hx - 3.0, -hy + 3.0, hy - 3.0, 0, 0.05, 'lobby_stone', coll=c))
    parts.append(block('lobby_ceil', -hx + 3.0, hx - 3.0, -hy + 3.0, hy - 3.0, LOBBY_H - 0.3, LOBBY_H, 'lobby_ceiling', coll=c))
    parts.append(block('lobby_core', -18.0, 18.0, 4.0, 14.0, 0.05, LOBBY_H - 0.3, 'lobby_stone', coll=c))
    for k in range(9):                                                  # ceiling light lines
        x = -hx + 6.0 + k * (2 * hx - 12.0) / 8
        parts.append(block('lobby_light', x - 0.15, x + 0.15, -hy + 3.5, hy - 3.5, LOBBY_H - 0.35, LOBBY_H - 0.3, 'lamp_cool', coll=c))
    for x in (-12.0, 12.0):                                             # escalators up to the mezzanine
        parts.append(block('escalator', x - 1.0, x + 1.0, -8.0, 2.0, 0.05, 1.0, 'steel_rail', coll=c))
        parts.append(block('escalator_up', x - 1.0, x + 1.0, -2.0, 2.0, 1.0, 5.0, 'steel_rail', coll=c))
    parts.append(block('mezzanine', -hx + 3.0, hx - 3.0, 2.0, hy - 3.0, 5.0, 5.3, 'lobby_stone', coll=c))
    for x in (-24.0, -4.0, 4.0, 24.0):
        tree(parts, x, -10.0, 0.05, 3.6, c)
    n = POD[0] // 2
    for k in range(n + 1):                                              # perimeter columns on every second bay
        x = -hx + k * 2 * BAY
        for y in (-hy, hy - 0.6):
            parts.append(block('col', x - 0.3, x + 0.3, y, y + 0.6, 0, LOBBY_H, 'fin_alu', coll=c))
    parts.append(block('canopy', -15.0, 15.0, -hy - 6.0, -hy + 3.0, 6.0, 6.4, 'fin_alu', coll=c))
    parts.append(block('canopy_glass', -14.8, 14.8, -hy - 5.8, -hy + 2.8, 6.4, 6.45, 'rail_glass', coll=c))
    parts.append(block('canopy_light', -14.6, 14.6, -hy - 5.6, -hy + 2.6, 5.95, 6.0, 'lamp_dim', coll=c))


def garden(parts, c, full):
    hx, hy = POD[0] * BAY / 2, POD[1] * BAY / 2
    z = zf(POD[3]) + 0.5
    parts.append(block('roof', -hx, hx, -hy, hy, z - 0.1, z, 'limestone_dark', coll=c))
    if not full:
        return
    ring(parts, 'g_rail', -hx + 0.1, hx - 0.1, -hy + 0.1, hy - 0.1, z, z + 1.2, 0.05, 'rail_glass', c)
    # strips in front of and behind the towers, and the court in the gap: lawns, a reflecting pool, trees, benches
    for (y0, y1) in ((-hy + 1.0, ext(TW[0])[2] - 1.0), (ext(TW[1])[3] + 1.0, hy - 1.0)):
        parts.append(block('lawn', -hx + 1.0, hx - 1.0, y0, y1, z, z + 0.1, 'lawn', coll=c))
    parts.append(block('pool_rim', -3.5, 3.5, -12.0, 12.0, z, z + 0.3, 'limestone', coll=c))
    parts.append(block('pool', -3.2, 3.2, -11.7, 11.7, z + 0.3, z + 0.31, 'pool_water', coll=c))
    for x in (-hx + 3.0, -hx + 12.0, hx - 12.0, hx - 3.0):
        for y in (-hy + 2.5, hy - 2.5):
            tree(parts, x, y, z, 2.8 + rnd.random(), c)
    for y in (-14.0, 14.0):
        for x in (-9.0, 9.0):
            parts.append(block('bench', x - 1.0, x + 1.0, y - 0.25, y + 0.25, z, z + 0.45, 'wood_deck', coll=c))
    for (x, y) in ((-5.0, -14.0), (5.0, 14.0), (-5.0, 14.0), (5.0, -14.0)):
        parts.append(cyl('post', x, y, z, z + 3.0, 0.05, 'steel_dark', 6, coll=c))
        parts.append(block('post_lamp', x - 0.15, x + 0.15, y - 0.15, y + 0.15, z + 3.0, z + 3.25, 'lamp_warm', coll=c))


def towers(parts, c, full):
    for t in TW:
        x0, x1, y0, y1 = ext(t)
        z0, z1 = zf(t['f0']), zf(t['f1'])
        zm0, zm1 = zf(MECH), zf(MECH + 1)
        parts.append(block('shaft', x0, x1, y0, y1, z0, zm0, 'MAT-01', UV, c))
        parts.append(block('shaft', x0, x1, y0, y1, zm1, z1, 'MAT-01', UV, c))
        parts.append(block('mech', x0 + 0.4, x1 - 0.4, y0 + 0.4, y1 - 0.4, zm0, zm1, 'louvre_dark', coll=c))
        if full:
            z = zm0 + 0.3
            while z < zm1 - 0.2:
                ring(parts, 'lv', x0, x1, y0, y1, z, z + 0.06, 0.1, 'steel_rail', c)
                z += 0.4
            # vertical fins on the corners (the glass shafts read as sharp prisms)
            for (px, py) in ((x0, y0), (x1, y0), (x0, y1), (x1, y1)):
                parts.append(block('corner_fin', px - 0.25, px + 0.25, py - 0.25, py + 0.25, z0, z1 + 10.0, 'fin_alu', coll=c))
        crown(parts, c, x0, x1, y0, y1, z1, full)


def crown(parts, c, x0, x1, y0, y1, z, full):
    """A screened plant crown: plant core behind a glass and fin screen 10 m high, a cool glow line at its top."""
    parts.append(block('crown_core', x0 + 3.0, x1 - 3.0, y0 + 3.0, y1 - 3.0, z, z + 7.0, 'louvre_dark', coll=c))
    parts.append(block('roof', x0, x1, y0, y1, z, z + 0.3, 'roof_metal', coll=c))
    if full:
        ring(parts, 'crown_glass', x0 + 0.05, x1 - 0.05, y0 + 0.05, y1 - 0.05, z, z + 10.0, 0.05, 'lobby_glass', c)
        for (a, b, yy, ax) in ((x0, x1, y0, 'x'), (x0, x1, y1, 'x'), (y0, y1, x0, 'y'), (y0, y1, x1, 'y')):
            n = int((b - a) / BAY)
            for k in range(n + 1):
                u = a + (b - a) * k / n
                if ax == 'x':
                    parts.append(block('crown_fin', u - 0.08, u + 0.08, yy - 0.3, yy + 0.3, z, z + 10.0, 'fin_alu', coll=c))
                else:
                    parts.append(block('crown_fin', yy - 0.3, yy + 0.3, u - 0.08, u + 0.08, z, z + 10.0, 'fin_alu', coll=c))
        ring(parts, 'crown_glow', x0 - 0.1, x1 + 0.1, y0 - 0.1, y1 + 0.1, z + 9.6, z + 10.0, 0.12, 'crown_glow', c)
        parts.append(cyl('mast', (x0 + x1) / 2, (y0 + y1) / 2, z + 7.0, z + 20.0, 0.15, 'steel_dark', 6, coll=c))
        parts.append(block('mast_light', (x0 + x1) / 2 - 0.2, (x0 + x1) / 2 + 0.2, (y0 + y1) / 2 - 0.2, (y0 + y1) / 2 + 0.2, z + 20.0, z + 20.4, 'red_light', coll=c))
    else:
        parts.append(block('crown_screen', x0, x1, y0, y1, z + 7.0, z + 10.0, 'fin_alu', coll=c))


def bridge(parts, c, full):
    """A glass bridge across the gap on floors 30-31: glass box, steel frame, lit floor edge."""
    xa, xb = ext(TW[0])[1], ext(TW[1])[0]
    z0, z1 = zf(BRIDGE[0]), zf(BRIDGE[1])
    y0, y1 = -4.0, 4.0
    parts.append(block('br_floor', xa, xb, y0, y1, z0 - 0.5, z0, 'steel_dark', coll=c))
    parts.append(block('br_roof', xa, xb, y0, y1, z1 - 0.3, z1, 'steel_dark', coll=c))
    if not full:
        parts.append(block('br_glass', xa, xb, y0 + 0.1, y1 - 0.1, z0, z1 - 0.3, 'lobby_glass', coll=c))
        return
    for y in (y0, y1 - 0.06):
        parts.append(block('br_glass', xa, xb, y, y + 0.06, z0, z1 - 0.3, 'lobby_glass', coll=c))
        parts.append(block('br_edge', xa, xb, y - 0.05, y + 0.11, z0 - 0.1, z0, 'lamp_cool', coll=c))
        for k in range(5):                                             # truss diagonals
            xx = xa + k * (xb - xa) / 4
            parts.append(block('br_post', xx - 0.1, xx + 0.1, y - 0.05, y + 0.11, z0, z1 - 0.3, 'steel_dark', coll=c))
    parts.append(block('br_in', xa, xb, y0 + 0.2, y1 - 0.2, z0, z0 + 0.05, 'lobby_stone', coll=c))
    parts.append(block('br_light', xa, xb, -0.3, 0.3, z0 + 3.6, z0 + 3.65, 'lamp_cool', coll=c))
    parts.append(block('br_mid', xa, xb, y0 + 0.2, y1 - 0.2, zf(BRIDGE[0] + 1) - 0.2, zf(BRIDGE[0] + 1), 'steel_dark', coll=c))


def lod_near(c):
    parts = []
    podium(parts, c, True); garden(parts, c, True); towers(parts, c, True); bridge(parts, c, True)
    return parts


def lod_mid(c):
    parts = []
    podium(parts, c, False); garden(parts, c, False); towers(parts, c, False); bridge(parts, c, False)
    return parts


def lod_far(c):
    parts = []
    hx, hy = POD[0] * BAY / 2, POD[1] * BAY / 2
    parts.append(block('podium', -hx, hx, -hy, hy, 0, zf(POD[3]) + 0.5, 'MAT-01', UV, c))
    for t in TW:
        x0, x1, y0, y1 = ext(t)
        parts.append(block('tower', x0, x1, y0, y1, zf(t['f0']), zf(t['f1']) + 10.0, 'MAT-01', UV, c))
    xa, xb = ext(TW[0])[1], ext(TW[1])[0]
    parts.append(block('bridge', xa, xb, -4.0, 4.0, zf(BRIDGE[0]) - 0.5, zf(BRIDGE[1]), 'steel_dark', coll=c))
    return parts


def sockets(lod, c):
    hy = POD[1] * BAY / 2
    xa, xb = ext(TW[0])[1], ext(TW[1])[0]
    s = [L.socket('SOCKET_base', (0, 0, 0), c), L.socket('SOCKET_entrance', (0, -hy - 6.0, 0), c),
         L.socket('SOCKET_garden', (0, 0, zf(POD[3]) + 0.5), c)]
    for f in (BRIDGE[0], 45):                                          # bridge sockets on real floors, both towers
        s += [L.socket(f'SOCKET_bridge_W_f{f}', (xa, 0, zf(f)), c), L.socket(f'SOCKET_bridge_E_f{f}', (xb, 0, zf(f)), c)]
    for i, t in enumerate(TW):
        s.append(L.socket(f'SOCKET_roof_{"WE"[i]}', (t['cx'], t['cy'], zf(t['f1']) + 20.4), c))
    return s


results = {'asset_id': 'BLD-03', 'version': '1.0-mc02', 'units': 'm', 'origin': 'bottom-centre of the lobby floor',
           'family': 'office', 'facade': 'office', 'bays': [POD[0], POD[1]], 'bay_m': BAY, 'floor_m': FLOOR,
           'floors': FLOORS, 'mechanical_floor': None, 'skip': [MECH], 'lobby_h_m': LOBBY_H, 'shaft_base_m': SHAFT0,
           'roof_m': [zf(t['f1']) for t in TW], 'crown_top_m': [zf(t['f1']) + 20.4 for t in TW],
           'bridge_floors': list(range(*BRIDGE)),
           'stages': [{'hx': POD[0] * BAY / 2, 'hy': POD[1] * BAY / 2, 'cx': 0.0, 'cy': 0.0, 'f0': POD[2], 'f1': POD[3]}]
           + [{'hx': t['bx'] * BAY / 2, 'hy': t['by'] * BAY / 2, 'cx': t['cx'], 'cy': t['cy'], 'f0': t['f0'], 'f1': t['f1']} for t in TW]}
L.build('BLD-03', OUT, {'near': lod_near, 'mid': lod_mid, 'far': lod_far}, results, sockets,
        dict(tiles=TILES, tile='office', mat='MAT-01', cam=(180, -260, 190), look=(0, 0, 120)))

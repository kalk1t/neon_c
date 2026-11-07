"""Living Megacity MC02: BLD-12 Main tower (Blender 5.2, headless).

  blender -b --factory-startup --python bld12_main.py -- <tiles dir> <outdir>

New York point of view (NYC-REFERENCE.md): the setback skyscraper taken to mega scale: Empire State massing logic
(a broad base, shafts stepping in at setbacks) with the chamfered corners of One World Trade Center; LMK-03's roof
garden (Manhattan Municipal Building: a civic block with a central arch) on its podium.
Here, 1,200 m tall and 300 m wide at the base:
  podium   300 x 300 x 48 m, MAT-01B limestone, a central arch portal on the front (the start deck side), a roof
           garden round the tower foot (lawns, paths, trees, fountains, pavilions, lamps)
  shafts   four chamfered MAT-01 stages: 210 m (48-360), 170 m (376-640), 132 m (656-880), 110 m (896-1060), each
           step a lit garden sky-lobby (16 m, glass set back 6 m, trees on the terrace ring)
  roof     at 1,062 m: the FREE ROAM spawn deck (24 lit pads, 6 across x 4 rows, cars face the front), a glass rail,
           edge lights; at the back an observation crown (two glass levels to 1,110 m) and a mast to 1,200 m
No interiors (the window shader shows rooms; real rooms on a 300 m tower are for later). Colliders from 'solids'.
Writes BLD-12_<lod>.glb, BLD-12.json (incl. 'spawn': deck height and pad positions), BLD-12.blend, review.
"""
import sys, os, math, random
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L
from lib_bld import block, rblock, cyl, ring, tree

A = sys.argv[sys.argv.index('--') + 1:]
TILES, OUT = os.path.abspath(A[0]), os.path.abspath(A[1])
UV = (3.0, 4.0, 48.0)
UV_POD = (3.0, 4.0, 0.0)
PH = 48.0                                       # podium height
P = 150.0                                       # podium half size
# shaft stages: half size, chamfer, z0, z1; garden sky-lobby bands of GH metres between them
GH = 16.0
STAGES = [(105.0, 24.0, PH, 360.0), (85.0, 20.0, 376.0, 640.0), (66.0, 16.0, 656.0, 880.0), (55.0, 12.0, 896.0, 1060.0)]
ROOF = 1062.0                                   # the spawn deck's surface
CROWN = (18.0, 30.0, ROOF, 1110.0)              # half size, centre y (back), z0, z1
MAST_TOP = 1200.0
FL, Z0 = 4.0, PH                                # office floor height; floor 0 at the podium roof
MECH = [38, 114, 180, 232]                      # two-floor mechanical bands (louvres), one per stage


def fz(f):
    return Z0 + f * FL


def fl(z):
    return int(round((z - Z0) / FL))


SLOTS = [((c - 2.5) * 12.0, -40.0 + r * 13.0) for r in range(4) for c in range(6)]   # spawn pads (x, y)
rnd = random.Random(12)
L.start()
L.common_mats()
L.mat('lawn', (0.07, 0.2, 0.06), 0.9, 0.0, (0.1, 0.3, 0.08), 0.03)
L.mat('garden_glow', (1.0, 0.9, 0.75), 0.6, 0.0, (1.0, 0.85, 0.6), 0.55)
L.mat('pad_light', (0.4, 0.9, 1.0), 0.5, 0.0, (0.3, 0.85, 1.0), 2.5)
L.mat('deck_dark', (0.09, 0.1, 0.12), 0.6, 0.3)
L.mat('obs_glow', (0.7, 0.85, 1.0), 0.5, 0.0, (0.6, 0.8, 1.0), 0.8)


def oct_(name, h, ch, z0, z1, m, uv=None, c=None, cy=0.0):
    """A chamfered square (One WTC corners): a rounded block with one segment per corner."""
    return rblock(name, h, h, cy, ch, z0, z1, m, uv, seg=1, coll=c)


def podium(parts, c, full):
    parts.append(block('podium', -P, P, -P, P, 0, PH, 'MAT-01B', UV_POD, c))
    parts.append(block('pod_cornice', -P - 0.8, P + 0.8, -P - 0.8, P + 0.8, PH - 1.2, PH, 'limestone_dark', coll=c))
    if not full:
        return
    # the central arch portal on the front: a 40 m opening, 32 m to the crown, lit hall behind, columns
    aw, ah = 20.0, 32.0
    parts.append(block('portal_frame', -aw - 4.0, aw + 4.0, -P - 2.0, -P, 0, ah + 6.0, 'limestone', coll=c))
    parts.append(block('portal_hall', -aw, aw, -P - 0.5, -P + 0.1, 0, ah, 'opening_dark', coll=c))
    parts.append(block('portal_glow', -aw + 1.0, aw - 1.0, -P - 0.6, -P - 0.5, 0.5, ah - 4.0, 'lobby_glass', coll=c))
    parts.append(block('portal_in', -aw + 1.0, aw - 1.0, -P + 0.2, -P + 0.3, 0, ah - 2.0, 'garden_glow', coll=c))
    n = 12                                                          # the arch: a stepped half circle of voussoirs
    for k in range(n):
        a0, a1 = math.pi * k / n, math.pi * (k + 1) / n
        x0, x1 = sorted((aw * math.cos(a0), aw * math.cos(a1)))
        z = ah - 6.0 + 6.0 * math.sin((a0 + a1) / 2)
        parts.append(block('voussoir', x0, x1, -P - 2.6, -P - 1.9, z, ah + 1.0, 'limestone_dark', coll=c))
    for x in (-aw - 2.0, -aw + 8.0, aw - 8.0, aw + 2.0):
        parts.append(cyl('portal_col', x, -P - 4.5, 0, ah - 6.0, 1.3, 'limestone', 12, coll=c))
    parts.append(block('portal_entab', -aw - 4.0, aw + 4.0, -P - 6.0, -P - 2.0, ah - 6.0, ah - 3.5, 'limestone', coll=c))
    for x in range(-int(P) + 12, int(P) - 11, 24):                  # shop and hall windows round the podium
        if abs(x) < aw + 8:
            continue
        for (y0, y1) in ((-P - 0.05, -P + 0.02), (P - 0.02, P + 0.05)):
            parts.append(block('pod_win', x - 7.0, x + 7.0, y0, y1, 2.0, 14.0, 'bar_glow', coll=c))
        for (x0, x1) in ((-P - 0.05, -P + 0.02), (P - 0.02, P + 0.05)):
            parts.append(block('pod_win', x0, x1, x - 7.0, x + 7.0, 2.0, 14.0, 'bar_glow', coll=c))


def roof_garden(parts, c, full):
    """LMK-03's roof garden on the podium round the tower foot."""
    z = PH
    h0 = STAGES[0][0]
    parts.append(block('pod_roof', -P, P, -P, P, z - 0.1, z, 'limestone_dark', coll=c))
    for (x0, x1, y0, y1) in ((-P + 4, P - 4, -P + 4, -h0 - 6), (-P + 4, P - 4, h0 + 6, P - 4),
                             (-P + 4, -h0 - 6, -h0 - 6, h0 + 6), (h0 + 6, P - 4, -h0 - 6, h0 + 6)):
        parts.append(block('lawn', x0, x1, y0, y1, z, z + 0.2, 'lawn', coll=c))
    if not full:
        return
    ring(parts, 'pod_rail', -P, P, -P, P, z, z + 1.2, 0.1, 'rail_glass', c)
    # paths: a cross on each side, fountains in the four corners, trees along the edges, pavilions, lamps
    for s in (-1, 1):
        parts.append(block('path', -3.0, 3.0, min(s * (h0 + 2), s * (P - 2)), max(s * (h0 + 2), s * (P - 2)), z + 0.2, z + 0.25, 'limestone', coll=c))
        parts.append(block('path', min(s * (h0 + 2), s * (P - 2)), max(s * (h0 + 2), s * (P - 2)), -3.0, 3.0, z + 0.2, z + 0.25, 'limestone', coll=c))
    for (sx, sy) in ((-1, -1), (1, -1), (-1, 1), (1, 1)):
        x, y = sx * 128.0, sy * 128.0
        parts.append(cyl('fountain_rim', x, y, z, z + 0.8, 9.0, 'limestone', 20, coll=c))
        parts.append(cyl('fountain', x, y, z + 0.8, z + 0.85, 8.4, 'pool_water', 20, coll=c))
        parts.append(cyl('jet', x, y, z + 0.8, z + 5.0, 0.6, 'pool_water', 8, coll=c))
    for k in range(-6, 7):
        u = k * 22.0
        for (x, y) in ((u, -P + 8), (u, P - 8), (-P + 8, u), (P - 8, u)):
            if abs(u) < 8:
                continue
            tree(parts, x, y, z + 0.2, 5.0 + rnd.random() * 2.5, c)
    for (x, y) in ((-60.0, -128.0), (60.0, -128.0), (-60.0, 128.0), (60.0, 128.0)):
        parts.append(block('pavilion_glass', x - 8, x + 8, y - 5, y + 5, z + 0.2, z + 4.5, 'lobby_glass', coll=c))
        parts.append(block('pavilion_in', x - 7.5, x + 7.5, y + 4.4, y + 4.5, z + 0.2, z + 4.3, 'bar_glow', coll=c))
        parts.append(block('pavilion_roof', x - 9, x + 9, y - 6, y + 6, z + 4.5, z + 5.0, 'steel_dark', coll=c))
    for k in range(-5, 6):
        u = k * 26.0
        for (x, y) in ((u, -h0 - 10), (u, h0 + 10), (-h0 - 10, u), (h0 + 10, u)):
            parts.append(cyl('post', x, y, z + 0.2, z + 4.5, 0.08, 'steel_dark', 6, coll=c))
            parts.append(block('post_lamp', x - 0.25, x + 0.25, y - 0.25, y + 0.25, z + 4.5, z + 4.9, 'lamp_warm', coll=c))


def shafts(parts, c, full):
    for i, (h, ch, z0, z1) in enumerate(STAGES):
        parts.append(oct_('shaft', h, ch, z0, z1, 'MAT-01', UV, c))
        parts.append(oct_('cap', h + 0.6, ch + 0.25, z1 - 0.6, z1 + 0.4, 'steel_dark', None, c))
        for m in MECH:                                                  # the mechanical band: louvres, proud of the glass
            if z0 <= fz(m) < z1:
                parts.append(oct_('mech', h + 0.35, ch + 0.15, fz(m), fz(m + 2), 'louvre_dark', None, c))
                if full:
                    for k in range(1, 8):
                        zz = fz(m) + k * 1.0
                        parts.append(oct_('mech_fin', h + 0.6, ch + 0.25, zz - 0.06, zz + 0.06, 'steel_rail', None, c))
                parts.append(oct_('mech_edge', h + 0.6, ch + 0.25, fz(m) - 0.3, fz(m), 'steel_dark', None, c))
                parts.append(oct_('mech_edge', h + 0.6, ch + 0.25, fz(m + 2), fz(m + 2) + 0.3, 'steel_dark', None, c))
        if full:                                                        # a slim metal band every tenth floor: scale
            for f in range(fl(z0) + 10, fl(z1) - 2, 10):
                if any(m - 1 <= f <= m + 3 for m in MECH):
                    continue
                parts.append(oct_('band', h + 0.18, ch + 0.07, fz(f) - 0.18, fz(f) + 0.18, 'steel_rail', None, c))
        if i + 1 < len(STAGES):
            sky_lobby(parts, c, full, STAGES[i], STAGES[i + 1])


def sky_lobby(parts, c, full, lower, upper):
    """The garden floor between two stages: glass set back 6 m from the upper stage, lit; the lower stage's roof is a
    terrace ring (lawn, trees), the upper stage starts above it."""
    h, ch, _, z1 = lower
    hu, chu, z0u, _ = upper
    z = z1 + 0.4
    parts.append(oct_('lobby_glass', hu - 6.0, max(2.0, chu - 2.5), z, z0u, 'lobby_glass', None, c))
    parts.append(oct_('terrace', h - 1.0, ch - 0.4, z1 + 0.4, z1 + 0.6, 'lawn', None, c))
    if not full:
        parts.append(oct_('lobby_core', hu - 9.0, max(2.0, chu - 3.5), z, z0u, 'garden_glow', None, c))
        return
    # inside the glass: a stone floor, a dark core with lit lift doors, a lit ceiling with light rows, a mezzanine,
    # trees in planters, benches and people-scale lamps; the glow is in the lights, not a wall
    hi = hu - 6.4
    parts.append(oct_('lobby_floor', hi, max(2.0, chu - 2.7), z, z + 0.15, 'lobby_stone', None, c))
    parts.append(oct_('lobby_ceil', hi, max(2.0, chu - 2.7), z0u - 0.5, z0u - 0.3, 'concrete_pale', None, c))
    core = hu * 0.42
    parts.append(oct_('lobby_core', core, 3.0, z, z0u, 'concrete_pale', None, c))
    for k in range(-2, 3):                                              # lift doors on the four core faces
        u = k * 5.0
        for (x0, x1, y0, y1) in ((u - 1.1, u + 1.1, -core - 0.05, -core), (u - 1.1, u + 1.1, core, core + 0.05),
                                 (-core - 0.05, -core, u - 1.1, u + 1.1), (core, core + 0.05, u - 1.1, u + 1.1)):
            parts.append(block('lift', x0, x1, y0, y1, z + 0.15, z + 3.2, 'bronze' if 'bronze' in L.M else 'steel_rail', coll=c))
    for k in range(6):                                                  # light rows in the ceiling (rings)
        r_ = core + 2.0 + k * (hi - core - 3.0) / 5
        ring(parts, 'lobby_light', -r_, r_, -r_, r_, z0u - 0.56, z0u - 0.5, 1.2, 'garden_glow', c)
    for k in range(-3, 4):                                              # warm lit stone panels on the core between the lifts
        u = k * 5.0 + 2.5
        if abs(u) > core - 2.0:
            continue
        for (x0, x1, y0, y1) in ((u - 1.0, u + 1.0, -core - 0.06, -core), (u - 1.0, u + 1.0, core, core + 0.06),
                                 (-core - 0.06, -core, u - 1.0, u + 1.0), (core, core + 0.06, u - 1.0, u + 1.0)):
            parts.append(block('core_panel', x0, x1, y0, y1, z + 3.6, z0u - 1.0, 'bar_glow', coll=c))
    zm = z + 8.0                                                        # a mezzanine ring round the core
    ring(parts, 'mezz', -core - 6.0, core + 6.0, -core - 6.0, core + 6.0, zm - 0.3, zm, 6.0, 'concrete_pale', c)
    ring(parts, 'mezz_rail', -core - 6.0, core + 6.0, -core - 6.0, core + 6.0, zm, zm + 1.1, 0.05, 'rail_glass', c)
    n = max(4, int(2 * (hi - core) / 9))
    for k in range(n):                                                  # trees and benches in the hall, on all four sides
        u = -hi + 10 + k * (2 * hi - 20) / max(1, n - 1)
        for (x, y) in ((u, -(core + hi) / 2), (u, (core + hi) / 2), (-(core + hi) / 2, u), ((core + hi) / 2, u)):
            if max(abs(x), abs(y)) < core + 2 or abs(x) + abs(y) > 2 * hi - max(2.0, chu - 2.7) - 2:
                continue
            if k % 2 == 0:
                parts.append(cyl('planter', x, y, z + 0.15, z + 0.9, 1.4, 'planter', 10, coll=c))
                tree(parts, x, y, z + 0.9, 5.5 + rnd.random() * 1.5, c)
            else:
                parts.append(block('bench', x - 1.2, x + 1.2, y - 0.3, y + 0.3, z + 0.15, z + 0.6, 'wood_deck', coll=c))
                parts.append(cyl('lamp_post', x + 1.8, y, z + 0.15, z + 3.0, 0.06, 'steel_dark', 6, coll=c))
                parts.append(block('lamp_head', x + 1.6, x + 2.0, y - 0.2, y + 0.2, z + 3.0, z + 3.3, 'lamp_warm', coll=c))
    n = int(2 * h / 14)
    for k in range(n + 1):
        u = -h + 6 + k * (2 * h - 12) / n
        for (x, y) in ((u, -h + 4), (u, h - 4), (-h + 4, u), (h - 4, u)):
            if abs(abs(x) - abs(y)) < 0.1 and abs(x) > h - ch:      # not on the chamfers
                continue
            if max(abs(x), abs(y)) > h - 3 or abs(x) + abs(y) > 2 * h - ch - 3:
                continue
            tree(parts, x, y, z1 + 0.6, 6.0 + rnd.random() * 3.0, c)
    for k in range(int(2 * hu / 6)):                                # mullions on the lobby glass
        u = -hu + 6.0 + k * 6.0
        if abs(u) > hu - 6.0 - chu:
            continue
        for (x0, x1, y0, y1) in ((u - 0.15, u + 0.15, -hu + 5.8, -hu + 6.2), (u - 0.15, u + 0.15, hu - 6.2, hu - 5.8),
                                 (-hu + 5.8, -hu + 6.2, u - 0.15, u + 0.15), (hu - 6.2, hu - 5.8, u - 0.15, u + 0.15)):
            parts.append(block('lobby_mull', x0, x1, y0, y1, z, z0u, 'steel_dark', coll=c))


def roof(parts, c, full):
    h = STAGES[-1][0]
    ch = STAGES[-1][1]
    z = ROOF
    parts.append(oct_('deck', h - 0.5, ch - 0.2, STAGES[-1][3] + 0.4, z, 'deck_dark', None, c))
    # observation crown at the back: two glass levels round a core, a slab between, a cap
    hc, cy, z0, z1 = CROWN
    parts.append(block('obs_core', -hc + 4, hc - 4, cy - hc + 4, cy + hc - 4, z0, z1, 'concrete_pale', coll=c))
    parts.append(block('obs_glass', -hc, hc, cy - hc, cy + hc, z0, z1 - 2.0, 'lobby_glass', coll=c))
    parts.append(block('obs_in', -hc + 0.6, hc - 0.6, cy - hc + 0.6, cy + hc - 0.6, z0 + 0.1, z0 + 0.3, 'obs_glow', coll=c))
    parts.append(block('obs_slab', -hc - 1.0, hc + 1.0, cy - hc - 1.0, cy + hc + 1.0, (z0 + z1) / 2 - 0.5, (z0 + z1) / 2 + 0.5, 'steel_dark', coll=c))
    parts.append(block('obs_cap', -hc - 1.5, hc + 1.5, cy - hc - 1.5, cy + hc + 1.5, z1 - 2.0, z1, 'alu' if 'alu' in L.M else 'steel_rail', coll=c))
    # the mast: a tapered stack to 1,200 m, rings, a beacon
    zz, r = z1, 3.2
    for k in range(6):
        zt = z1 + (MAST_TOP - z1) * (k + 1) / 6
        parts.append(cyl('mast', 0, cy, zz, zt, r, 'steel_rail', 12 if full else 6, coll=c))
        if full:
            parts.append(cyl('mast_ring', 0, cy, zt - 0.6, zt, r + 0.5, 'steel_dark', 12, coll=c))
        zz, r = zt, r * 0.72
    parts.append(block('beacon', -0.8, 0.8, cy - 0.8, cy + 0.8, MAST_TOP, MAST_TOP + 1.6, 'red_light', coll=c))
    if not full:
        return
    # the spawn deck: a lit pad per slot, guide lines, a rail with edge lights
    for (x, y) in SLOTS:
        for (x0, x1, y0, y1) in ((x - 4.5, x + 4.5, y - 5.5, y - 5.3), (x - 4.5, x + 4.5, y + 5.3, y + 5.5),
                                 (x - 4.5, x - 4.3, y - 5.5, y + 5.5), (x + 4.3, x + 4.5, y - 5.5, y + 5.5)):
            parts.append(block('pad', x0, x1, y0, y1, z, z + 0.04, 'pad_light', coll=c))
        parts.append(block('pad_arrow', x - 0.3, x + 0.3, y - 4.0, y - 1.0, z, z + 0.04, 'pad_light', coll=c))
    for x in (-36.0, 36.0):
        parts.append(block('guide', x - 0.2, x + 0.2, -h + 4, CROWN[1] - CROWN[0] - 4, z, z + 0.04, 'lamp_cool', coll=c))
    parts.append(block('deck_edge', -h + 3, h - 3, -h + 2.0, -h + 2.4, z, z + 0.04, 'pad_light', coll=c))
    ring(parts, 'deck_rail', -h + 1.0, h - 1.0, -h + 1.0, h - 1.0, z, z + 1.1, 0.06, 'rail_glass', c)
    for k in range(-5, 6):
        u = k * 10.0
        for (x, y) in ((u, -h + 1.2), (u, h - 1.2), (-h + 1.2, u), (h - 1.2, u)):
            parts.append(block('edge_light', x - 0.25, x + 0.25, y - 0.25, y + 0.25, z + 1.1, z + 1.4, 'red_light', coll=c))


def lod_near(c):
    parts = []
    podium(parts, c, True); roof_garden(parts, c, True); shafts(parts, c, True); roof(parts, c, True)
    return parts


def lod_mid(c):
    parts = []
    podium(parts, c, False); roof_garden(parts, c, False); shafts(parts, c, False); roof(parts, c, False)
    return parts


def lod_far(c):
    parts = [block('podium', -P, P, -P, P, 0, PH, 'MAT-01B', UV_POD, c)]
    for (h, ch, z0, z1) in STAGES:
        parts.append(oct_('shaft', h, ch, z0 - GH if z0 > PH else z0, z1, 'MAT-01', UV, c))
    hc, cy, z0, z1 = CROWN
    parts.append(block('crown', -hc, hc, cy - hc, cy + hc, z0, z1, 'obs_glow', coll=c))
    parts.append(cyl('mast', 0, cy, z1, MAST_TOP, 2.0, 'steel_rail', 6, coll=c))
    return parts


def sockets(lod, c):
    s = [L.socket('SOCKET_base', (0, 0, 0), c), L.socket('SOCKET_portal', (0, -P - 6.0, 0), c),
         L.socket('SOCKET_roof_garden', (0, -P + 20.0, PH), c), L.socket('SOCKET_spawn_deck', (0, -20.0, ROOF), c),
         L.socket('SOCKET_crown', (0, CROWN[1], CROWN[3]), c), L.socket('SOCKET_mast_top', (0, CROWN[1], MAST_TOP), c)]
    for i, (h, ch, z0, z1) in enumerate(STAGES[:-1]):
        s.append(L.socket(f'SOCKET_sky_lobby_{i}', (0, -h + 4.0, z1 + 0.6), c))
    return s


solids = [{'x0': -P, 'x1': P, 'y0': -P, 'y1': P, 'z0': 0.0, 'z1': PH}]
for i, (h, ch, z0, z1) in enumerate(STAGES):
    top = ROOF if i == len(STAGES) - 1 else z1 + 0.6
    solids.append({'x0': -h, 'x1': h, 'y0': -h, 'y1': h, 'z0': z0 - (GH if i else 0.0), 'z1': top})
hc, cy, z0, z1 = CROWN
solids.append({'x0': -hc - 1.5, 'x1': hc + 1.5, 'y0': cy - hc - 1.5, 'y1': cy + hc + 1.5, 'z0': z0, 'z1': z1})
solids.append({'x0': -3.0, 'x1': 3.0, 'y0': cy - 3.0, 'y1': cy + 3.0, 'z0': z1, 'z1': MAST_TOP + 1.6})
skip = sorted({f for (h, ch, z0, z1) in STAGES[1:] for f in range(fl(z0) - 4, fl(z0))} | {f for m in MECH for f in (m, m + 1)})
stages = [{'hx': h, 'hy': h, 'cx': 0.0, 'cy': 0.0, 'f0': fl(z0), 'f1': fl(z1), 'r': ch, 'chamfer': True} for (h, ch, z0, z1) in STAGES]
results = {'asset_id': 'BLD-12', 'version': '1.1-mc02', 'units': 'm', 'origin': 'bottom-centre of the podium',
           'family': 'office', 'facade': 'MAT-01 shafts, MAT-01B podium',
           'floor_m': FL, 'shaft_base_m': Z0, 'floors': fl(STAGES[-1][3]), 'skip': skip, 'mechanical_floors': MECH,
           'stages': stages, 'crown_m': 0.0,
           'podium_m': [2 * P, 2 * P, PH], 'stages_m': [{'w': 2 * h, 'chamfer': ch, 'z0': z0, 'z1': z1} for (h, ch, z0, z1) in STAGES],
           'roof_m': ROOF, 'crown_top_m': CROWN[3], 'mast_top_m': MAST_TOP + 1.6,
           'spawn': {'deck_m': ROOF, 'front': '-y', 'pads': [[x, y] for (x, y) in SLOTS]},
           'solids': solids}
L.build('BLD-12', OUT, {'near': lod_near, 'mid': lod_mid, 'far': lod_far}, results, sockets,
        dict(tiles=TILES, tile='office', mat='MAT-01', cam=(900, -1500, 700), look=(0, 0, 560)))

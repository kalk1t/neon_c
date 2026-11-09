# v0.3.9 tyre shop: every wheel of the three shop cars, modelled in Blender: the stock wheel of each car (into the car
# file: meshy/wheels/<model>_wheels.glb, put on the car by game/tools/split-wheels.mjs) and its ten shop tyres
# (public/tires/<car>.glb, swapped onto the car by the game: src/car.js useTires). Designs from the Meshy concept sheets
# (meshy/tires/concept_<car>.png, gpt-image-2): ROADSTER iron tyres (owner: no rubber needed), VIPER low-profile
# performance tyres and forged rims, TITAN chunky off-road tyres on beadlock rims.
#
# Each wheel: axle along X, origin at the wheel's centre; the L wheel's outer face looks to -X (the car's left), R is its
# mirror. Sizes per car (the car's wheel arches / pods): CARS[car]['size'] = (R, W), ROADSTER's rear pair bigger.
# Materials are all named wheel_* (the game keeps them out of paint, liveries and skins): wheel_tyre_<id>,
# wheel_rim_<id>, wheel_acc_<id> (a second finish: inserts, back discs), wheel_glow_<id> (emissive), wheel_dark.
# Hi: 64 segments, bevelled edges (highlights), weighted normals; lo: 24 segments, no lugs or knurls.
#
# Usage: blender -b --python build_tires.py -- [car ...] [--thumbs]
#   writes meshy/wheels/<model>_wheels.glb (stock) and meshy/tires/out/<car>/<code>.glb (each shop tyre), both with the
#   objects <lod>_<L|R>[_rear]; then game/tools/compress-tires.mjs; with --thumbs meshy/tires/out/thumbs/<car>-<code>.png (stock: <car>-stock.png)
import math, os, sys
import bpy, bmesh
from mathutils import Vector, Matrix

HERE = os.path.dirname(os.path.abspath(__file__))
argv = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else []
THUMBS = '--thumbs' in argv
ONLY = [a for a in argv if not a.startswith('--')]
OUT = os.path.join(HERE, 'out')
os.makedirs(os.path.join(OUT, 'thumbs'), exist_ok=True)
TAU = 2 * math.pi

# colours (linear RGB)
CHROME, GUN, BLACKCHROME, STEEL = (0.62, 0.63, 0.66), (0.16, 0.17, 0.19), (0.07, 0.07, 0.08), (0.36, 0.37, 0.4)
GOLD, BRONZE, CARBON, SILVER = (0.83, 0.55, 0.2), (0.42, 0.27, 0.15), (0.03, 0.03, 0.035), (0.75, 0.76, 0.78)
OLIVE, TAN, ORANGE, BLACK = (0.12, 0.14, 0.06), (0.42, 0.3, 0.17), (0.9, 0.22, 0.02), (0.02, 0.02, 0.022)
PINK, CYAN, AMBER, RED = (1.0, 0.05, 0.75), (0.05, 0.75, 1.0), (1.0, 0.45, 0.02), (1.0, 0.04, 0.03)
GREEN, VIOLET, ICE, WHITE, YELLOW = (0.15, 1.0, 0.1), (0.45, 0.1, 1.0), (0.35, 0.75, 1.0), (0.9, 0.95, 1.0), (1.0, 0.7, 0.05)


def W(code, name, tyre, rim, acc, glow, feats, chip):
    """One wheel design. tyre: (kind, options); rim/acc: (colour, roughness); glow: colour; feats: face features."""
    return dict(code=code, name=name, tyre=tyre, rim=rim, acc=acc, glow=glow, feats=feats, chip=chip)


IRON = lambda c=STEEL, r=0.3, **k: ('iron', dict(color=c, rough=r, **k))
LOW = lambda **k: ('low', k)
OFF = lambda **k: ('off', k)

CARS = {
    'roadster': dict(model='roadster4', size=dict(F=(0.38, 0.52), R=(0.405, 0.6)), wheels=[
        W('stock', 'STOCK', IRON(grooves=3), (GUN, 0.22), (BLACKCHROME, 0.3), PINK,
          [('spokes', dict(n=5, split=0.11, w=(0.05, 0.03))), ('ring', dict(r=0.86, kind='dash', n=8))], (0x2a2c30, 0xff26d4)),
        W('w01', 'TURBINE PINK', IRON(BLACKCHROME, 0.25, grooves=4), (BLACKCHROME, 0.18), (GUN, 0.3), PINK,
          [('spokes', dict(n=16, w=(0.022, 0.03), twist=0.9, sweep=0.35, depth=0.05)), ('ring', dict(r=0.9))], (0x15151a, 0xff26d4)),
        W('w02', 'SPLIT CHROME', IRON(CHROME, 0.2, grooves=2), (CHROME, 0.12), (BLACKCHROME, 0.3), PINK,
          [('spokes', dict(n=4, split=0.14, w=(0.045, 0.03), depth=0.04)), ('ring', dict(r=0.88))], (0xb8bcc4, 0xff26d4)),
        W('w03', 'NEON LATTICE', IRON(BLACKCHROME, 0.25, knurl=True), (SILVER, 0.2), (BLACKCHROME, 0.3), PINK,
          [('spokes', dict(n=14, w=(0.012, 0.012), sweep=1.1, depth=0.02)), ('spokes', dict(n=14, w=(0.012, 0.012), sweep=-1.1, depth=0.02, x=0.012)),
           ('ring', dict(r=0.92))], (0x9a9ea6, 0xff26d4)),
        W('w04', 'CYAN FAN', IRON(STEEL, 0.25, grooves=3), (STEEL, 0.28), (GUN, 0.3), CYAN,
          [('spokes', dict(n=6, w=(0.03, 0.13), twist=0.55, depth=0.02)), ('ring', dict(r=0.97, w=0.025))], (0x8a9098, 0x19e6ff)),
        W('w05', 'GOLD STAR', IRON(BLACKCHROME, 0.25, grooves=2), (GOLD, 0.2), (BLACKCHROME, 0.3), YELLOW,
          [('backdisc', dict(mat='acc')), ('spokes', dict(n=10, w=(0.016, 0.02), sweep=0.25, depth=0.035)), ('ring', dict(r=0.94))], (0x1a1a1e, 0xffc23a)),
        W('w06', 'CARBON SHIELD', IRON(BLACKCHROME, 0.25, segmented=24), (CARBON, 0.35), (GUN, 0.25), PINK,
          [('disc', dict(n=3, gap=0.18, r0=0.3, r1=0.96, dome=0.02)), ('ring', dict(r=0.62, w=0.015)), ('ring', dict(r=0.95))], (0x101014, 0xff26d4)),
        W('w07', 'VORTEX', IRON(BLACKCHROME, 0.25, knurl=True), (BLACKCHROME, 0.15), (GUN, 0.3), PINK,
          [('spokes', dict(n=24, w=(0.01, 0.016), sweep=0.6, twist=0.4, depth=0.03)), ('ring', dict(r=0.55, w=0.012)), ('ring', dict(r=0.93))], (0x202026, 0xc81ab0)),
        W('w08', 'GOLD TRIDENT', IRON(BLACKCHROME, 0.25, grooves=3), (GOLD, 0.18), (BRONZE, 0.45), YELLOW,
          [('backdisc', dict(mat='acc', hexes=True)), ('spokes', dict(n=3, w=(0.07, 0.04), sweep=0.25, depth=0.05)), ('ring', dict(r=0.96))], (0x2a2014, 0xffc23a)),
        W('w09', 'RED REACTOR', IRON(BLACKCHROME, 0.25, grooves=4), (STEEL, 0.35), (GUN, 0.3), RED,
          [('disc', dict(n=1, r0=0.2, r1=0.96, dome=0.03)), ('ring', dict(r=0.82, kind='bars', n=12)), ('ring', dict(r=0.22, w=0.015))], (0x6a6e74, 0xff2020)),
        W('w10', 'VENOM STAR', IRON(BLACKCHROME, 0.25, segmented=20), (BLACKCHROME, 0.2), (GUN, 0.3), GREEN,
          [('spokes', dict(n=5, w=(0.07, 0.025), sweep=0.15, depth=0.05)), ('ring', dict(r=0.93))], (0x16181a, 0x2bff7a)),
    ]),
    'viper': dict(model='viper', size=dict(F=(0.38, 0.5)), wheels=[
        W('stock', 'STOCK', LOW(), (SILVER, 0.18), (GUN, 0.3), PINK,
          [('spokes', dict(n=10, w=(0.022, 0.02), depth=0.03)), ('ring', dict(r=0.9, kind='dash', n=10))], (0xb8bcc4, 0xff2ad4)),
        W('w01', 'PINK SPLIT TEN', LOW(), (SILVER, 0.15), (BLACKCHROME, 0.3), PINK,
          [('spokes', dict(n=5, split=0.09, w=(0.03, 0.014), sweep=0.2, depth=0.035)), ('ring', dict(r=0.97, w=0.022))], (0xc8ccd4, 0xff2ad4)),
        W('w02', 'ICE DISC', LOW(), (SILVER, 0.12), (GUN, 0.3), ICE,
          [('disc', dict(n=4, gap=0.1, r0=0.22, r1=0.93, dome=0.03)), ('ring', dict(r=0.97, w=0.022))], (0xd8dce4, 0x3fd8ff)),
        W('w03', 'GOLD TURBINE', LOW(), (BLACKCHROME, 0.2), (GOLD, 0.2), YELLOW,
          [('spokes', dict(n=20, w=(0.014, 0.024), twist=1.0, sweep=0.5, depth=0.05)), ('hub', dict(mat='acc', r=0.24)), ('ring', dict(r=0.97, w=0.02))], (0x16161a, 0xffb020)),
        W('w04', 'TOXIC STAR', LOW(), (BLACKCHROME, 0.25), (GUN, 0.3), GREEN,
          [('spokes', dict(n=5, split=0.06, w=(0.026, 0.02), depth=0.04)), ('ring', dict(r=0.9, w=0.012)), ('ring', dict(r=0.97, w=0.018))], (0x16181a, 0x7dff1a)),
        W('w05', 'HEX VIOLET', LOW(), (BLACKCHROME, 0.2), (CARBON, 0.4), VIOLET,
          [('backdisc', dict(mat='acc')), ('hex', dict(n=6, at=0.6, size=0.26)), ('ring', dict(r=0.97, w=0.022))], (0x14121c, 0x9a55ff)),
        W('w06', 'SILVER FORGE', LOW(), (CHROME, 0.1), (GUN, 0.3), WHITE,
          [('spokes', dict(n=10, split=0.05, w=(0.016, 0.01), sweep=0.12, depth=0.03))], (0xd0d4dc, 0xe8f4ff)),
        W('w07', 'ICE BLADE', LOW(), (BLACKCHROME, 0.2), (GUN, 0.3), ICE,
          [('spokes', dict(n=3, w=(0.06, 0.1), sweep=0.9, twist=0.3, depth=0.03)), ('ring', dict(r=0.97, w=0.02))], (0x14161a, 0x3fd8ff)),
        W('w08', 'PINK AERO', LOW(), (BLACKCHROME, 0.25), (GUN, 0.3), PINK,
          [('disc', dict(n=1, r0=0.18, r1=0.96, dome=0.04)), ('ring', dict(r=0.6, kind='dash', n=4, w=0.012)), ('ring', dict(r=0.97, w=0.02))], (0x18181c, 0xff2ad4)),
        W('w09', 'BRONZE Y', LOW(), (BRONZE, 0.25), (GUN, 0.3), AMBER,
          [('spokes', dict(n=7, split=0.07, w=(0.028, 0.012), sweep=0.1, depth=0.035, y=True))], (0x8a5a34, 0xffa040)),
        W('w10', 'VIOLET TWIST', LOW(), (BLACKCHROME, 0.18), (GUN, 0.3), VIOLET,
          [('spokes', dict(n=8, w=(0.03, 0.045), sweep=1.0, twist=0.35, depth=0.04)), ('ring', dict(r=0.97, w=0.022))], (0x16141c, 0x9a55ff)),
    ]),
    'titan': dict(model='titan', size=dict(F=(0.44, 0.68)), wheels=[
        W('stock', 'STOCK', OFF(lugs=22), (GUN, 0.3), (BLACKCHROME, 0.3), CYAN,
          [('spokes', dict(n=6, w=(0.05, 0.045), depth=0.06)), ('beadlock', dict(bolts=18)), ('hub', dict(sides=8))], (0x2c2e32, 0x19e6ff)),
        W('w01', 'WARLORD', OFF(lugs=20), (GUN, 0.35), (OLIVE, 0.6), AMBER,
          [('spokes', dict(n=6, w=(0.05, 0.06), depth=0.06)), ('armour', dict(n=6, gap=0.12)), ('ring', dict(r=0.8, kind='dash', n=6, w=0.014)), ('hub', dict(sides=8))], (0x2c3020, 0xffa020)),
        W('w02', 'OLIVE EIGHT', OFF(lugs=24), (OLIVE, 0.55), (GUN, 0.4), None,
          [('spokes', dict(n=8, w=(0.035, 0.05), depth=0.08)), ('beadlock', dict(bolts=20, mat='acc')), ('hub', dict(sides=8))], (0x3a4022, 0x6a7040)),
        W('w03', 'CYAN SIX', OFF(lugs=20), (CHROME, 0.15), (BLACKCHROME, 0.3), CYAN,
          [('spokes', dict(n=6, split=0.08, w=(0.03, 0.03), depth=0.06)), ('ring', dict(r=0.84, w=0.016)), ('hub', dict(sides=8))], (0xc0c4cc, 0x19e6ff)),
        W('w04', 'DESERT HAZARD', OFF(lugs=22), (BLACKCHROME, 0.3), (ORANGE, 0.45), None,
          [('spokes', dict(n=6, w=(0.05, 0.05), depth=0.06, inserts=True)), ('beadlock', dict(bolts=18, mat='tan')), ('hub', dict(sides=8))], (0x8a6a40, 0xff6a14)),
        W('w05', 'CHROME CRUSHER', OFF(lugs=20), (CHROME, 0.1), (BLACKCHROME, 0.3), None,
          [('spokes', dict(n=6, w=(0.07, 0.04), sweep=0.25, depth=0.06)), ('beadlock', dict(bolts=20)), ('hub', dict(sides=8))], (0xd0d4dc, 0x5a5e64)),
        W('w06', 'STEALTH OPS', OFF(lugs=24, hexblock=True), (BLACKCHROME, 0.35), (GUN, 0.4), CYAN,
          [('spokes', dict(n=6, w=(0.06, 0.07), depth=0.05)), ('ring', dict(r=0.95, w=0.016)), ('hub', dict(sides=8))], (0x141518, 0x19e6ff)),
        W('w07', 'BRONZE RANGER', OFF(lugs=22, hexblock=True), (BRONZE, 0.3), (BLACKCHROME, 0.3), None,
          [('spokes', dict(n=8, split=0.05, w=(0.022, 0.02), depth=0.06)), ('beadlock', dict(bolts=20)), ('hub', dict(sides=8))], (0x6a4a2a, 0x2a2420)),
        W('w08', 'HAZARD ORANGE', OFF(lugs=20), (ORANGE, 0.4), (BLACKCHROME, 0.3), None,
          [('spokes', dict(n=6, w=(0.06, 0.07), depth=0.07)), ('beadlock', dict(bolts=18)), ('hub', dict(sides=8, mat='acc'))], (0xff6a14, 0x1a1a1e)),
        W('w09', 'OLIVE BEACON', OFF(lugs=22), (OLIVE, 0.55), (GUN, 0.4), AMBER,
          [('spokes', dict(n=5, w=(0.06, 0.08), depth=0.06)), ('ring', dict(r=0.9, kind='dots', n=24)), ('hub', dict(sides=8))], (0x3a4022, 0xffa020)),
        W('w10', 'DUNE BEACON', OFF(lugs=24), (GUN, 0.35), (TAN, 0.5), AMBER,
          [('spokes', dict(n=6, split=0.06, w=(0.026, 0.024), depth=0.06)), ('beadlock', dict(bolts=20, mat='acc')), ('ring', dict(r=0.8, kind='dots', n=18)), ('hub', dict(sides=8))], (0x8a6a40, 0xffa020)),
    ]),
}


# ---------------- materials ----------------
def material(name, color, metal, rough, emit=None):
    m = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    m.use_nodes = True
    b = m.node_tree.nodes['Principled BSDF']
    b.inputs['Base Color'].default_value = (*color, 1)
    b.inputs['Metallic'].default_value = metal
    b.inputs['Roughness'].default_value = rough
    if emit:
        b.inputs['Emission Color'].default_value = (*emit, 1)
        b.inputs['Emission Strength'].default_value = 1.2
    return m


def wheel_materials(car, w):
    """[tyre, rim, acc, glow, dark] for this wheel; the stock wheel's names stay short (one car file, one set)."""
    sfx = car if w['code'] == 'stock' else f"{car}_{w['code']}"
    kind, t = w['tyre']
    tyre = (material(f'wheel_tyre_{sfx}', t['color'], 1.0, t['rough']) if kind == 'iron'
            else material(f'wheel_tyre_{car}', (0.022, 0.022, 0.025), 0.0, 0.86))
    return [tyre,
            material(f'wheel_rim_{sfx}', w['rim'][0], 1.0, w['rim'][1]),
            material(f'wheel_acc_{sfx}', w['acc'][0], 0.0 if w['acc'][0] in (ORANGE, OLIVE, TAN) else 1.0, w['acc'][1]),
            material(f'wheel_glow_{sfx}', (0.02, 0.02, 0.02), 0.0, 0.4, emit=w['glow'] or (0.0, 0.0, 0.0)),
            material('wheel_dark', (0.012, 0.012, 0.014), 0.3, 0.6)]


T, RIM, ACC, GLOW, DARK = range(5)


# ---------------- geometry helpers (axle along X; (y, z) the wheel's plane) ----------------
def pol(r, a):
    return r * math.cos(a), r * math.sin(a)


def lathe(bm, prof, seg, mat, a0=0.0, a1=TAU, caps=False):
    """Revolves a (x, r) profile around X; a full turn closes, an arc gets end caps if asked."""
    full = abs(a1 - a0 - TAU) < 1e-6
    n = seg if full else seg + 1
    rings = []
    for i in range(n):
        a = a0 + (a1 - a0) * i / seg
        rings.append([bm.verts.new((x, *pol(r, a))) for x, r in prof])
    for i in range(seg if full else seg):
        A, B = rings[i], rings[(i + 1) % n]
        for j in range(len(prof) - 1):
            f = bm.faces.new((A[j], B[j], B[j + 1], A[j + 1]))
            f.material_index = mat
    if caps and not full:
        for ring, rev in ((rings[0], True), (rings[-1], False)):
            f = bm.faces.new(ring[::-1] if rev else ring)
            f.material_index = mat
    return rings


def solid(bm, x0, x1, r0, r1, a0, a1, seg, mat):
    """A closed ring sector (box in polar space): front face x0, back x1, radii r0..r1, angles a0..a1."""
    lathe(bm, [(x0, r0), (x0, r1), (x1, r1), (x1, r0), (x0, r0)], seg, mat, a0, a1, caps=True)


def prism(bm, cy, cz, r, sides, x0, x1, mat, rot=0.0, ri=None):
    """An n-sided prism along X centred at (cy, cz) (a nut, a bolt); with ri a hollow frame."""
    outer0 = [bm.verts.new((x0, cy + r * math.cos(rot + TAU * k / sides), cz + r * math.sin(rot + TAU * k / sides))) for k in range(sides)]
    outer1 = [bm.verts.new((x1, v.co.y, v.co.z)) for v in outer0]
    faces = []
    if ri is None:
        faces.append(bm.faces.new(outer0[::-1]))
        faces.append(bm.faces.new(outer1))
    else:
        in0 = [bm.verts.new((x0, cy + ri * math.cos(rot + TAU * k / sides), cz + ri * math.sin(rot + TAU * k / sides))) for k in range(sides)]
        in1 = [bm.verts.new((x1, v.co.y, v.co.z)) for v in in0]
        for k in range(sides):
            k2 = (k + 1) % sides
            faces.append(bm.faces.new((outer0[k], in0[k], in0[k2], outer0[k2])))
            faces.append(bm.faces.new((in0[k], in1[k], in1[k2], in0[k2])))
    for k in range(sides):
        k2 = (k + 1) % sides
        faces.append(bm.faces.new((outer0[k], outer0[k2], outer1[k2], outer1[k])))
    for f in faces:
        f.material_index = mat


def spoke(bm, face_x, depth, r0, r1, w0, w1, ang, sweep, twist, mat, steps):
    """A spoke / blade from r0 to r1: half-widths w0..w1, swept by `sweep` rad at the rim, pitched by `twist` rad."""
    rows = []
    for i in range(steps + 1):
        t = i / steps
        r = r0 + (r1 - r0) * t
        a = ang + sweep * t ** 1.4
        w = w0 + (w1 - w0) * t
        cy, cz = pol(r, a)
        ty, tz = -math.sin(a), math.cos(a)  # tangent
        p = twist * (0.4 + 0.6 * t)  # blade pitch
        row = []
        for sx, sw in ((-1, -1), (-1, 1), (1, 1), (1, -1)):  # front/back x, -/+ tangent
            u, v = sw * w, sx * depth / 2
            u, v = u * math.cos(p) - v * math.sin(p), u * math.sin(p) + v * math.cos(p)
            row.append(bm.verts.new((face_x + depth / 2 + v, cy + ty * u, cz + tz * u)))
        rows.append(row)
    for i in range(steps):
        A, B = rows[i], rows[i + 1]
        for k in range(4):
            f = bm.faces.new((A[k], A[(k + 1) % 4], B[(k + 1) % 4], B[k]))
            f.material_index = mat
    for row, rev in ((rows[0], False), (rows[-1], True)):
        f = bm.faces.new(row[::-1] if rev else row)
        f.material_index = mat


# ---------------- a wheel ----------------
def tyre(bm, kind, o, R, Wd, seg, hi):
    """The tyre (material T): iron band, low-profile rubber or off-road lugs. Returns the bead radius (rim lip)."""
    x0, x1 = -Wd / 2, Wd / 2
    if kind == 'iron':
        rb = R - 0.085
        prof = [(x1 - 0.008, rb), (x1, rb + 0.01), (x1, R - 0.02), (x1 - 0.014, R - 0.003)]
        n = 10 if hi else 2
        prof += [(x1 - 0.03 - (Wd - 0.06) * k / n, R) for k in range(n + 1)]
        prof += [(x0 + 0.014, R - 0.003), (x0, R - 0.02), (x0, rb + 0.01), (x0 + 0.008, rb)]
        rings = lathe(bm, prof, seg, T)
        if hi and (o.get('grooves') or o.get('knurl')):
            g = o.get('grooves', 0)
            for i, ring in enumerate(rings):
                for j in range(4, 4 + n + 1):
                    k = (j - 4) / n
                    cut = 0
                    if g and any(abs(k - (c + 1) / (g + 1)) < 0.045 for c in range(g)):
                        cut = 0.012
                    if o.get('knurl') and i % 2 == 0 and 0.1 < k < 0.9:
                        cut = max(cut, 0.006)
                    if cut:
                        ring[j].co.y *= (R - cut) / R
                        ring[j].co.z *= (R - cut) / R
        if o.get('segmented'):  # steel cleats round the band
            n_ = o['segmented']
            for i in range(n_):
                a = TAU * i / n_
                solid(bm, x0 + 0.03, x1 - 0.03, R - 0.002, R + 0.016, a, a + TAU / n_ * 0.55, 2 if hi else 1, T)
        return rb
    if kind == 'low':  # low-profile performance tyre: short sidewall, square shoulders, grooves and sipes
        S = 0.075
        rb = R - S
        prof = [(x1 - 0.01, rb), (x1 + 0.012, rb + S * 0.45), (x1 + 0.006, R - 0.025), (x1 - 0.02, R - 0.003)]
        n = 10 if hi else 2
        prof += [(x1 - 0.04 - (Wd - 0.08) * k / n, R) for k in range(n + 1)]
        prof += [(x0 + 0.02, R - 0.003), (x0 - 0.006, R - 0.025), (x0 - 0.012, rb + S * 0.45), (x0 + 0.01, rb)]
        rings = lathe(bm, prof, seg, T)
        if hi:
            for i, ring in enumerate(rings):
                for j in range(4, 4 + n + 1):
                    k = (j - 4) / n
                    cut = 0.012 if any(abs(k - c) < 0.05 for c in (0.3, 0.7)) else (0.006 if i % 4 == 0 and (k < 0.22 or k > 0.78) else 0)
                    if cut:
                        ring[j].co.y *= (R - cut) / R
                        ring[j].co.z *= (R - cut) / R
        return rb
    # off-road: a carcass with big staggered lugs that wrap onto the shoulders
    S = 0.15
    rb = R - S
    L = 0.045  # lug height
    Rc = R - L
    prof = [(x1 - 0.01, rb), (x1 + 0.03, rb + S * 0.4), (x1 + 0.02, Rc - 0.02), (x1 - 0.03, Rc), (x0 + 0.03, Rc),
            (x0 - 0.02, Rc - 0.02), (x0 - 0.03, rb + S * 0.4), (x0 + 0.01, rb)]
    lathe(bm, prof, seg, T)
    n = o.get('lugs', 22)
    for i in range(n):
        a = TAU * i / n
        da = TAU / n * (0.42 if o.get('hexblock') else 0.5)
        for side in (0, 1):  # two staggered rows; each lug runs from the centre over its shoulder
            off = (0.5 / n * TAU) if side else 0
            if side == 0:
                xa, xb = -0.02, x1 + 0.025
            else:
                xa, xb = x0 - 0.025, 0.02
            if hi:
                solid(bm, xa, xb, Rc - 0.004, R, a + off, a + off + da, 2, T)
                # shoulder step: the lug comes down the sidewall
                xs = xb - 0.04 if side == 0 else xa
                solid(bm, xs, xs + 0.04, Rc - 0.05, Rc, a + off, a + off + da * 0.8, 1, T)
                if o.get('hexblock'):  # a small centre block between the rows
                    solid(bm, -0.05, 0.05, Rc - 0.004, R - 0.012, a + off + da * 1.05, a + off + da * 1.6, 1, T)
            else:
                solid(bm, xa, xb, Rc - 0.004, R, a + off, a + off + da, 1, T)
    return rb


def wheel(car, w, R, Wd, lod):
    hi = lod == 'hi'
    seg = 64 if hi else 24
    bm = bmesh.new()
    kind, o = w['tyre']
    x0, x1 = -Wd / 2, Wd / 2
    rb = tyre(bm, kind, o, R, Wd, seg, hi)
    # rim: lip, barrel and a dark back wall
    rl = rb - 0.004
    face_x = x0 + 0.045
    lathe(bm, [(x1 - 0.02, rl), (x0 + 0.012, rl), (x0 + 0.002, rl + 0.012), (x0 - 0.006, rl - 0.008), (face_x - 0.01, rl - 0.03)], seg, RIM)
    lathe(bm, [(x1 - 0.02, rl), (x1 - 0.02, 0.06)], seg, DARK)
    rr = rl - 0.03  # inside of the lip: the face features reach this radius
    steps = 6 if hi else 2
    hub_r = 0.1
    for kindf, f in w['feats']:
        if kindf == 'spokes':
            n, (w0, w1) = f['n'], f['w']
            fx = face_x + f.get('x', 0.0)
            pairs = [(-f['split'] / 2, f['split'] / 2)] if f.get('split') else [(0, 0)]
            for i in range(n):
                a = TAU * i / n
                for p0, p1 in pairs:
                    if f.get('split'):  # twin spokes: together at the hub, apart at the rim (a Y when y=True)
                        if f.get('y'):
                            spoke(bm, fx, f.get('depth', 0.03), hub_r * 0.9, rr * 0.55, w0, w0 * 0.8, a, f.get('sweep', 0) * 0.5, 0, RIM, steps)
                            spoke(bm, fx, f.get('depth', 0.03) * 0.9, rr * 0.5, rr + 0.004, w1, w1, a + 0.0, (p1 * 4) + f.get('sweep', 0) * 0.5, 0, RIM, steps)
                            spoke(bm, fx, f.get('depth', 0.03) * 0.9, rr * 0.5, rr + 0.004, w1, w1, a + 0.0, (p0 * 4) + f.get('sweep', 0) * 0.5, 0, RIM, steps)
                            break
                        spoke(bm, fx, f.get('depth', 0.03), hub_r * 0.85, rr + 0.004, w0, w1, a + p0 * 0.3, (p0 - p0 * 0.3) + f.get('sweep', 0), f.get('twist', 0), RIM, steps)
                    else:
                        spoke(bm, fx, f.get('depth', 0.03), hub_r * 0.85, rr + 0.004, w0, w1, a, f.get('sweep', 0), f.get('twist', 0), RIM, steps)
                    if f.get('inserts'):  # a coloured insert along the spoke
                        spoke(bm, fx - 0.006, 0.01, hub_r * 1.6, rr * 0.8, w0 * 0.45, w1 * 0.45, a, 0, 0, ACC, 2)
        elif kindf == 'disc':
            n, gap = f['n'], f.get('gap', 0)
            r0, r1, dome = f['r0'] * rr, f['r1'] * rr, f.get('dome', 0)
            for i in range(n):
                a0 = TAU * i / n + (gap / 2 if n > 1 else 0)
                a1 = TAU * (i + 1) / n - (gap / 2 if n > 1 else 0)
                prof = [(face_x + 0.01, r1), (face_x - dome * 0.5, (r0 + r1) / 2), (face_x - dome, r0), (face_x + 0.02, r0), (face_x + 0.03, r1), (face_x + 0.01, r1)]
                if n == 1:
                    lathe(bm, prof[:-1], seg, RIM)
                else:
                    lathe(bm, prof, max(4, int(seg * (a1 - a0) / TAU)), RIM, a0, a1, caps=True)
            lathe(bm, [(face_x + 0.04, r1 + 0.01), (face_x + 0.04, 0.0)], seg, DARK)  # behind the slots
        elif kindf == 'backdisc':
            lathe(bm, [(face_x + 0.035, rr + 0.005), (face_x + 0.035, hub_r)], seg, ACC if f['mat'] == 'acc' else DARK)
            if f.get('hexes') and hi:  # honeycomb bosses on the back disc
                for ring_r, cnt in ((0.45, 10), (0.7, 16), (0.88, 22)):
                    for k in range(cnt):
                        cy, cz = pol(rr * ring_r, TAU * (k + 0.5 * (ring_r > 0.6)) / cnt)
                        prism(bm, cy, cz, rr * 0.075, 6, face_x + 0.03, face_x + 0.022, ACC, ri=rr * 0.05)
        elif kindf == 'hex':
            for k in range(f['n']):
                cy, cz = pol(rr * f['at'], TAU * k / f['n'])
                s = rr * f['size']
                prism(bm, cy, cz, s, 6, face_x - 0.005, face_x + 0.03, RIM, rot=TAU * k / f['n'], ri=s * 0.78)
                prism(bm, cy, cz, s * 0.78, 6, face_x + 0.03, face_x + 0.031, DARK, rot=TAU * k / f['n'])
            lathe(bm, [(face_x - 0.004, hub_r * 1.3), (face_x - 0.004, rr * 0.25), (face_x + 0.03, rr * 0.25)], seg, RIM)
        elif kindf == 'ring':
            r = f['r'] * rr
            wd = f.get('w', 0.012)
            k = f.get('kind', 'solid')
            if k == 'solid':
                lathe(bm, [(face_x - 0.004, r + wd), (face_x - 0.008, r + wd * 0.6), (face_x - 0.008, r - wd * 0.6), (face_x - 0.004, r - wd)], seg, GLOW)
            else:
                n = f['n']
                for i in range(n):
                    a = TAU * i / n
                    if k == 'dash':
                        solid(bm, face_x - 0.01, face_x, r - wd, r + wd, a, a + TAU / n * 0.6, max(2, seg // n), GLOW)
                    elif k == 'bars':
                        solid(bm, face_x - 0.012, face_x, r - 0.05, r + 0.03, a - 0.05, a + 0.05, 1, GLOW)
                    else:  # dots
                        cy, cz = pol(r, a)
                        prism(bm, cy, cz, 0.011, 6, face_x - 0.012, face_x, GLOW)
        elif kindf == 'beadlock':  # a bolted outer ring over the bead
            m = {'acc': ACC, 'tan': ACC}.get(f.get('mat'), RIM)
            lathe(bm, [(x0 - 0.016, rl + 0.022), (x0 - 0.016, rl - 0.055), (x0 + 0.012, rl - 0.055)], seg, m)
            bolts = f['bolts'] if hi else f['bolts'] // 2
            for i in range(bolts):
                cy, cz = pol(rl - 0.016, TAU * (i + 0.5) / bolts)
                prism(bm, cy, cz, 0.012, 6 if hi else 4, x0 - 0.03, x0 - 0.014, RIM if m == ACC else DARK)
        elif kindf == 'armour':  # armour plates round the face with gaps (glow shows in the gaps)
            n, gap = f['n'], f['gap']
            for i in range(n):
                a = TAU * i / n
                solid(bm, face_x - 0.03, face_x - 0.005, rr * 0.72, rr + 0.004, a + gap / 2, a + TAU / n - gap / 2, max(2, seg // n), ACC)
        elif kindf == 'hub':
            hub_r = f.get('r', 0.14) if f.get('r') else hub_r
    # hub: a cap (round, or an n-sided nut) with a glowing centre
    hub = next((f for k_, f in w['feats'] if k_ == 'hub'), {})
    hm = ACC if hub.get('mat') == 'acc' else RIM
    if hub.get('sides'):
        prism(bm, 0, 0, 0.13, hub['sides'], face_x - 0.03, face_x + 0.04, hm, rot=math.pi / hub['sides'])
        prism(bm, 0, 0, 0.07, 6, face_x - 0.045, face_x - 0.03, DARK)
    else:
        hr = hub.get('r', 0.11) * (rr / 0.3 if hub.get('r') else 1)
        hr = min(hr, rr * 0.4)
        lathe(bm, [(face_x - 0.025, 0.0), (face_x - 0.025, hr * 0.55), (face_x - 0.04, hr * 0.8), (face_x - 0.015, hr), (x1 + 0.1, hr * 0.7), (x1 + 0.1, 0.0)], max(16, seg // 2), hm)
    if w['glow']:
        lathe(bm, [(face_x - 0.042, 0.022), (face_x - 0.048, 0.03), (face_x - 0.042, 0.038)], max(12, seg // 3), GLOW)
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-5)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    me = bpy.data.meshes.new(f"{car}_{w['code']}_{lod}")
    bm.to_mesh(me)
    bm.free()
    for m in wheel_materials(car, w):
        me.materials.append(m)
    return me


def finish(o, hi):
    """Polish: bevelled hard edges (hi only), smooth by angle, weighted normals; then everything applied."""
    bpy.context.view_layer.objects.active = o
    o.select_set(True)
    if hi:
        b = o.modifiers.new('bevel', 'BEVEL')
        b.width, b.segments, b.limit_method, b.angle_limit = 0.0025, 1, 'ANGLE', math.radians(50)
        b.harden_normals = True
    if hasattr(o.data, 'shade_smooth_by_angle'):
        o.data.shade_smooth_by_angle(angle=math.radians(35))
    if hi:
        o.modifiers.new('wn', 'WEIGHTED_NORMAL').keep_sharp = True
    for m in list(o.modifiers):
        bpy.ops.object.modifier_apply(modifier=m.name)
    o.select_set(False)


def mirror(me, name):
    r = me.copy()
    r.name = name
    r.transform(Matrix.Diagonal((-1, 1, 1, 1)))
    r.flip_normals()
    return r


def tris(o):
    return sum(len(p.vertices) - 2 for p in o.data.polygons)


def build(car, C, w, prefix=''):
    """The wheel's objects: <prefix><lod>_<L|R>[_rear]."""
    objs = []
    for axle, (R, Wd) in C['size'].items():
        sfx = '_rear' if axle == 'R' else ''
        for lod in ('hi', 'lo'):
            left = wheel(car, w, R, Wd, lod)
            ol = bpy.data.objects.new(f'{prefix}{lod}_L{sfx}', left)
            bpy.context.scene.collection.objects.link(ol)
            finish(ol, lod == 'hi')
            orr = bpy.data.objects.new(f'{prefix}{lod}_R{sfx}', mirror(ol.data, f"{car}_{w['code']}_{lod}_R{sfx}"))
            bpy.context.scene.collection.objects.link(orr)
            objs += [ol, orr]
    return objs


def export(path, objs):
    bpy.ops.object.select_all(action='DESELECT')
    for o in objs:
        o.select_set(True)
    bpy.ops.export_scene.gltf(filepath=path, export_format='GLB', use_selection=True, export_yup=True, export_normals=True)


def thumb(o, path):
    """A three-quarter render of one hi L wheel on a dark studio background (the shop chip)."""
    sc = bpy.context.scene
    engines = [e.identifier for e in bpy.types.RenderSettings.bl_rna.properties['engine'].enum_items]
    sc.render.engine = 'BLENDER_EEVEE' if 'BLENDER_EEVEE' in engines else 'BLENDER_EEVEE_NEXT'
    sc.render.resolution_x = sc.render.resolution_y = 320
    sc.render.film_transparent = True
    sc.view_settings.view_transform = 'AgX' if 'AgX' in [v.identifier for v in sc.view_settings.bl_rna.properties['view_transform'].enum_items] else 'Filmic'
    for x in sc.objects:
        x.hide_render = x is not o and x.type == 'MESH'
    if not sc.world:
        sc.world = bpy.data.worlds.new('w')
    sc.world.use_nodes = True
    bg = sc.world.node_tree.nodes['Background']
    bg.inputs['Color'].default_value = (0.5, 0.52, 0.58, 1)
    bg.inputs['Strength'].default_value = 0.22
    if 'cam' not in bpy.data.objects:
        cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam'))
        sc.collection.objects.link(cam)
        for i, (loc, e, s) in enumerate((((-2.2, -1.6, 2.4), 300, 1.2), ((1.5, 2.5, 0.6), 160, 1.6), ((-2.5, 2.0, -0.6), 80, 2.0))):
            L = bpy.data.objects.new(f'L{i}', bpy.data.lights.new(f'L{i}', 'AREA'))
            L.data.energy, L.data.size = e, s
            L.location = loc
            L.rotation_euler = (-Vector(loc)).to_track_quat('-Z', 'Y').to_euler()
            sc.collection.objects.link(L)
    cam = bpy.data.objects['cam']
    sc.camera = cam
    d = Vector((-1.0, -0.42, 0.28)).normalized()
    R = max(o.dimensions) / 2
    cam.location = d * R * 5.2
    cam.rotation_euler = d.to_track_quat('Z', 'Y').to_euler()
    cam.data.lens = 70
    sc.render.filepath = path
    bpy.ops.render.render(write_still=True)


bpy.ops.wm.read_factory_settings(use_empty=True)
for car, C in CARS.items():
    if ONLY and car not in ONLY:
        continue
    for o in list(bpy.data.objects):
        bpy.data.objects.remove(o)
    os.makedirs(os.path.join(OUT, car), exist_ok=True)
    for w in C['wheels']:
        objs = build(car, C, w)
        export(os.path.join(HERE, '..', 'wheels', f"{C['model']}_wheels.glb") if w['code'] == 'stock'
               else os.path.join(OUT, car, f"{w['code']}.glb"), objs)
        hiL = next(o for o in objs if o.name.endswith('hi_L'))
        loL = next(o for o in objs if o.name.endswith('lo_L'))
        print('TIRE', car, w['code'], w['name'], 'hi', tris(hiL), 'lo', tris(loL))
        if THUMBS:
            thumb(hiL, os.path.join(OUT, 'thumbs', f"{car}-{w['code']}.png"))
        for o in list(objs):  # names are reused by the next wheel
            bpy.data.objects.remove(o)
    print('WROTE', car)

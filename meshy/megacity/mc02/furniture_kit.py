"""Living Megacity MC02: the furniture kit for real 3D interiors (near the camera).

  blender -b --factory-startup --python furniture_kit.py -- <out.glb>

Every piece is low poly (boxes and cylinders), coloured by vertex colour with a little lighting baked in (top faces
bright, sides mid, undersides dark, darker towards the floor), so the game draws thousands of instances with one
unlit material and no lights. Parts that glow (screens, LEDs, lamps) are a second mesh '<piece>__glow' drawn with the
glow material. Origin: the piece's footprint centre on the floor. Facing: the window is -Y in Blender (+Z in the game):
screens and chairs face the glass, so you see glowing screens from outside.
"""
import bpy, bmesh, sys, os, math, json

OUT = os.path.abspath(sys.argv[sys.argv.index('--') + 1])
bpy.ops.wm.read_factory_settings(use_empty=True)


def lin(c):
    return tuple(pow(v, 2.2) for v in c)


class Piece:
    def __init__(self, name):
        self.name = name
        self.parts = {False: [], True: []}   # glow -> [(verts, faces, colour)]

    def box(self, x0, x1, y0, y1, z0, z1, col, glow=False):
        v = [(x, y, z) for z in (z0, z1) for y in (y0, y1) for x in (x0, x1)]
        f = [(0, 2, 3, 1), (4, 5, 7, 6), (0, 1, 5, 4), (2, 6, 7, 3), (0, 4, 6, 2), (1, 3, 7, 5)]
        self.parts[glow].append((v, f, col))

    def cyl(self, x, y, z0, z1, r, col, n=8, glow=False):
        v = [(x + r * math.cos(2 * math.pi * k / n), y + r * math.sin(2 * math.pi * k / n), z) for z in (z0, z1) for k in range(n)]
        f = [tuple(range(n - 1, -1, -1)), tuple(range(n, 2 * n))] + [(k, (k + 1) % n, n + (k + 1) % n, n + k) for k in range(n)]
        self.parts[glow].append((v, f, col))

    def build(self):
        out = []
        for glow in (False, True):
            if not self.parts[glow]:
                continue
            bm = bmesh.new()
            ca = bm.loops.layers.float_color.new('Col')
            for verts, faces, col in self.parts[glow]:
                vs = [bm.verts.new(p) for p in verts]
                for fi in faces:
                    face = bm.faces.new([vs[i] for i in fi])
                    face.normal_update()
                    n = face.normal
                    shade = 1.0 if glow else (1.0 if n.z > 0.5 else 0.42 if n.z < -0.5 else 0.66 + 0.12 * abs(n.x))
                    for lp in face.loops:
                        ao = 1.0 if glow else 0.72 + 0.28 * min(1.0, lp.vert.co.z / 0.6)
                        c = lin(col)
                        lp[ca] = (c[0] * shade * ao, c[1] * shade * ao, c[2] * shade * ao, 1.0)
            me = bpy.data.meshes.new(self.name + ('__glow' if glow else ''))
            bm.to_mesh(me)
            bm.free()
            me.color_attributes.active_color = me.color_attributes['Col']
            me.color_attributes.render_color_index = 0
            o = bpy.data.objects.new(me.name, me)
            bpy.context.scene.collection.objects.link(o)
            out.append(o)
        return out


DARK, METAL, WOOD, LWOOD, WHITE = (0.07, 0.07, 0.08), (0.35, 0.36, 0.38), (0.42, 0.3, 0.2), (0.72, 0.62, 0.48), (0.86, 0.86, 0.84)
SCREEN, SCREEN2, RGB1, RGB2, WARM = (0.45, 0.65, 1.0), (0.55, 0.95, 0.85), (1.0, 0.2, 0.75), (0.2, 0.9, 1.0), (1.0, 0.75, 0.45)
P = {}


def piece(name):
    p = Piece(name)
    P[name] = p
    return p


def chair(p, x, y, col=DARK, face=-1, gaming=False):
    p.box(x - 0.25, x + 0.25, y - 0.25, y + 0.25, 0.42, 0.5, col)
    by = y - 0.24 * face
    p.box(x - 0.25, x + 0.25, by - 0.04, by + 0.04, 0.5, 1.25 if gaming else 0.98, col)
    if gaming:
        p.box(x - 0.26, x + 0.26, by - 0.045, by + 0.045, 0.95, 1.0, RGB1)
    p.cyl(x, y, 0.05, 0.42, 0.03, METAL, 6)
    p.box(x - 0.28, x + 0.28, y - 0.03, y + 0.03, 0.0, 0.05, METAL)


def desk(p, x, y, w=1.6, d=0.8, col=WHITE, h=0.74):
    p.box(x - w / 2, x + w / 2, y - d / 2, y + d / 2, h - 0.03, h, col)
    for sx in (-1, 1):
        p.box(x + sx * (w / 2 - 0.04) - 0.025, x + sx * (w / 2 - 0.04) + 0.025, y - d / 2 + 0.05, y + d / 2 - 0.05, 0, h - 0.03, METAL)


def monitor(p, x, y, w=0.6, h0=0.8, col=SCREEN):
    p.box(x - w / 2, x + w / 2, y, y + 0.03, h0, h0 + w * 0.58, DARK)
    p.box(x - w / 2 + 0.02, x + w / 2 - 0.02, y - 0.005, y, h0 + 0.02, h0 + w * 0.58 - 0.02, col, glow=True)
    p.box(x - 0.03, x + 0.03, y + 0.03, y + 0.07, 0.74, h0, METAL)


# ---- offices
p = piece('workstation'); desk(p, 0, 0.1); monitor(p, 0, 0.25); chair(p, 0, -0.55)
p = piece('call_pod'); desk(p, 0, 0.1, 1.2, 0.7)
p.box(-0.62, -0.58, -0.25, 0.45, 0, 1.3, (0.3, 0.38, 0.45)); p.box(0.58, 0.62, -0.25, 0.45, 0, 1.3, (0.3, 0.38, 0.45))
p.box(-0.6, 0.6, 0.43, 0.47, 0.74, 1.3, (0.3, 0.38, 0.45)); monitor(p, -0.15, 0.2, 0.5, col=SCREEN2); chair(p, 0, -0.5, (0.12, 0.2, 0.32))
p = piece('meeting_table'); desk(p, 0, 0, 2.6, 1.1, WOOD)
for k in (-0.9, 0, 0.9):
    chair(p, k, -0.85, face=1); chair(p, k, 0.85, face=-1)
p = piece('plant')
p.cyl(0, 0, 0, 0.45, 0.24, (0.22, 0.2, 0.19)); p.cyl(0, 0, 0.45, 1.5, 0.42, (0.12, 0.3, 0.1), 7); p.cyl(0, 0, 1.0, 1.75, 0.3, (0.16, 0.36, 0.13), 7)
# ---- classroom
p = piece('school_desk'); desk(p, 0, 0.1, 1.2, 0.55, LWOOD, 0.72); chair(p, -0.3, -0.45, (0.2, 0.35, 0.6), face=-1); chair(p, 0.3, -0.45, (0.2, 0.35, 0.6), face=-1)
p = piece('teacher_desk'); desk(p, 0, 0, 1.8, 0.8, WOOD); p.box(-0.85, 0.85, 0.3, 0.38, 0, 0.7, WOOD); monitor(p, 0.5, -0.1, 0.45); chair(p, 0, 0.75, face=1)
p = piece('board')   # stands against the core wall, faces the window
p.box(-2.0, 2.0, -0.04, 0.0, 0.9, 2.2, (0.15, 0.32, 0.22)); p.box(-2.05, 2.05, -0.06, 0.0, 0.85, 0.9, METAL)
# ---- living / tv
p = piece('sofa')
for c, (x0, x1, y0, y1, z0, z1) in [((0.3, 0.32, 0.4), (-1.1, 1.1, -0.45, 0.45, 0.12, 0.45)), ((0.3, 0.32, 0.4), (-1.1, 1.1, 0.25, 0.45, 0.45, 0.9)),
                                   ((0.27, 0.29, 0.36), (-1.2, -1.05, -0.45, 0.45, 0.12, 0.65)), ((0.27, 0.29, 0.36), (1.05, 1.2, -0.45, 0.45, 0.12, 0.65))]:
    p.box(x0, x1, y0, y1, z0, z1, c)
p.box(-1.2, 1.2, -0.45, 0.45, 0, 0.12, DARK)
p = piece('tv_wall')  # TV and low cabinet against the core wall (screen faces the window)
p.box(-1.3, 1.3, -0.4, 0.0, 0, 0.5, (0.12, 0.1, 0.09)); p.box(-1.0, 1.0, -0.06, 0.0, 0.9, 2.05, DARK)
p.box(-0.97, 0.97, -0.065, -0.06, 0.93, 2.02, (0.4, 0.55, 0.95), glow=True)
p = piece('coffee_table'); desk(p, 0, 0, 1.1, 0.6, WOOD, 0.4); p.box(-1.4, 1.4, -1.0, 1.0, 0, 0.012, (0.45, 0.2, 0.16))
p = piece('bean_bag'); p.cyl(0, 0, 0, 0.45, 0.42, (0.85, 0.35, 0.15), 8); p.cyl(0, 0, 0.45, 0.6, 0.3, (0.85, 0.35, 0.15), 8)
p = piece('floor_lamp'); p.cyl(0, 0, 0, 0.03, 0.18, METAL); p.cyl(0, 0, 0, 1.55, 0.015, METAL, 5); p.cyl(0, 0, 1.55, 1.85, 0.2, WARM, 8, glow=True)
# ---- gaming
p = piece('gaming_desk'); desk(p, 0, 0.1, 1.7, 0.8, DARK)
monitor(p, -0.45, 0.25, 0.6, col=RGB2); monitor(p, 0.25, 0.25, 0.7, col=(0.7, 0.4, 1.0))
p.box(-0.85, 0.85, -0.31, -0.29, 0.66, 0.69, RGB1, glow=True)   # LED strip under the desk edge
p.box(0.6, 0.8, 0.0, 0.4, 0.74, 1.2, DARK); p.box(0.6, 0.8, -0.005, 0.0, 0.8, 1.15, RGB2, glow=True)  # PC tower glass
chair(p, 0, -0.6, (0.12, 0.12, 0.14), gaming=True)
p = piece('arcade'); p.box(-0.35, 0.35, -0.35, 0.35, 0, 1.75, (0.15, 0.05, 0.25)); p.box(-0.3, 0.3, -0.36, -0.35, 1.0, 1.45, (0.4, 1.0, 0.6), glow=True)
p.box(-0.35, 0.35, -0.55, -0.35, 0.85, 0.95, DARK); p.box(-0.36, 0.36, -0.36, 0.36, 1.65, 1.75, RGB1, glow=True)
# ---- gym
p = piece('treadmill'); p.box(-0.4, 0.4, -0.9, 0.9, 0, 0.2, DARK); p.box(-0.35, 0.35, -0.85, 0.85, 0.2, 0.22, (0.15, 0.15, 0.16))
p.box(-0.4, -0.35, 0.6, 0.7, 0.2, 1.3, METAL); p.box(0.35, 0.4, 0.6, 0.7, 0.2, 1.3, METAL); p.box(-0.4, 0.4, 0.6, 0.75, 1.25, 1.45, DARK)
p.box(-0.2, 0.2, 0.59, 0.6, 1.3, 1.42, SCREEN2, glow=True)
p = piece('weight_bench'); p.box(-0.15, 0.15, -0.6, 0.6, 0.4, 0.48, (0.6, 0.1, 0.1)); p.cyl(0, 0, 0, 0.4, 0.04, METAL)
for sx in (-0.6, 0.6):
    p.box(sx - 0.03, sx + 0.03, 0.5, 0.56, 0, 1.2, METAL)
p.cyl(-0.9, 0.53, 1.05, 1.15, 0.22, DARK); p.cyl(0.9, 0.53, 1.05, 1.15, 0.22, DARK); p.box(-0.9, 0.9, 0.51, 0.55, 1.08, 1.12, METAL)
p = piece('dumbbell_rack'); p.box(-1.0, 1.0, -0.25, 0.25, 0, 0.7, METAL)
for k in range(8):
    p.cyl(-0.85 + k * 0.24, 0, 0.7, 0.8, 0.08, DARK)
# ---- server hall
p = piece('server_rack'); p.box(-0.3, 0.3, -0.5, 0.5, 0, 2.1, (0.08, 0.08, 0.1))
for k in range(9):
    p.box(-0.25, 0.25, -0.51, -0.5, 0.2 + k * 0.2, 0.22 + k * 0.2, (0.3, 1.0, 0.45) if k % 3 else (0.3, 0.6, 1.0), glow=True)
# ---- restaurant
p = piece('table4'); desk(p, 0, 0, 0.9, 0.9, (0.85, 0.82, 0.76), 0.75)
for (x, y, f) in ((0, -0.7, 1), (0, 0.7, -1)):
    chair(p, x, y, (0.45, 0.2, 0.12), face=f)
p.cyl(0, 0, 0.75, 0.9, 0.05, WARM, 6, glow=True)
p = piece('bar_counter'); p.box(-2.0, 2.0, -0.35, 0.35, 0, 1.05, (0.25, 0.14, 0.1)); p.box(-2.05, 2.05, -0.4, 0.4, 1.05, 1.1, (0.75, 0.72, 0.66))
p.box(-2.0, 2.0, -0.36, -0.35, 0.05, 0.1, WARM, glow=True)
for k in range(5):
    p.cyl(-1.6 + k * 0.8, -0.65, 0, 0.75, 0.18, (0.4, 0.2, 0.12), 8)
# ---- library
p = piece('bookshelf')
p.box(-1.0, 1.0, -0.2, 0.2, 0, 0.03, WOOD)
import random
rnd = random.Random(7)
for k in range(5):
    z = 0.03 + k * 0.42
    p.box(-1.0, 1.0, -0.2, 0.2, z + 0.4, z + 0.42, WOOD)
    x = -0.97
    while x < 0.9:
        w = rnd.uniform(0.1, 0.25)
        p.box(x, x + w - 0.01, -0.17, 0.17, z, z + rnd.uniform(0.26, 0.37), rnd.choice([(0.55, 0.12, 0.1), (0.12, 0.22, 0.45), (0.7, 0.6, 0.38), (0.15, 0.35, 0.18), (0.82, 0.8, 0.74)]))
        x += w
p.box(-1.02, -0.98, -0.2, 0.2, 0, 2.15, WOOD); p.box(0.98, 1.02, -0.2, 0.2, 0, 2.15, WOOD)
p = piece('reading_table'); desk(p, 0, 0, 2.2, 0.9, WOOD); p.cyl(-0.5, 0, 0.74, 1.15, 0.015, METAL, 5); p.cyl(-0.5, 0, 1.1, 1.18, 0.12, (0.2, 0.5, 0.25))
p.cyl(-0.5, 0, 1.09, 1.1, 0.11, WARM, 8, glow=True)
for k in (-0.6, 0.6):
    chair(p, k, -0.75, (0.35, 0.22, 0.14), face=1); chair(p, k, 0.75, (0.35, 0.22, 0.14), face=-1)
# ---- fit-out / storage
p = piece('crates')
for (x, y, s, h) in ((-0.5, 0, 0.6, 0.6), (0.2, 0.1, 0.7, 0.5), (0.1, 0.05, 0.5, 1.0), (-0.45, 0.4, 0.45, 0.95)):
    p.box(x - s / 2, x + s / 2, y - s / 2, y + s / 2, 0 if h < 0.9 else 0.5, h if h < 0.9 else h + 0.4, (0.58, 0.44, 0.27))
p = piece('pingpong'); desk(p, 0, 0, 2.74, 1.52, (0.1, 0.32, 0.22), 0.76); p.box(-0.01, 0.01, -0.8, 0.8, 0.76, 0.91, WHITE)
# ---- homes (BLD-05 apartments)
FABRIC = [(0.55, 0.52, 0.48), (0.3, 0.38, 0.5), (0.5, 0.3, 0.25)]
p = piece('bed')   # double bed, headboard against the core side
p.box(-0.8, 0.8, -1.0, 1.0, 0.0, 0.3, WOOD); p.box(-0.78, 0.78, -0.98, 0.98, 0.3, 0.5, (0.85, 0.84, 0.8))
p.box(-0.78, 0.78, -0.98, 0.2, 0.5, 0.56, (0.35, 0.45, 0.6)); p.box(-0.65, -0.1, 0.5, 0.85, 0.5, 0.65, WHITE); p.box(0.1, 0.65, 0.5, 0.85, 0.5, 0.65, WHITE)
p.box(-0.85, 0.85, 0.95, 1.05, 0.0, 1.1, WOOD)
for sx in (-1.15, 1.15):
    p.box(sx - 0.22, sx + 0.22, 0.6, 1.0, 0, 0.5, WOOD); p.cyl(sx, 0.8, 0.5, 0.75, 0.1, WARM, 6, glow=True)
p = piece('wardrobe'); p.box(-1.0, 1.0, -0.3, 0.3, 0, 2.2, (0.82, 0.8, 0.76)); p.box(-0.01, 0.01, -0.31, -0.3, 0.1, 2.1, METAL)
p = piece('kitchen')   # counter run with upper cabinets, against the core wall, faces the window
p.box(-1.6, 1.6, -0.3, 0.3, 0, 0.88, (0.85, 0.84, 0.8)); p.box(-1.62, 1.62, -0.32, 0.32, 0.88, 0.92, (0.2, 0.2, 0.22))
p.box(-1.6, 1.6, 0.0, 0.35, 1.5, 2.2, (0.85, 0.84, 0.8)); p.box(0.9, 1.6, -0.35, 0.3, 0, 2.0, (0.7, 0.71, 0.73))
p.box(-1.55, 0.85, -0.005, 0.0, 1.45, 1.5, WARM, glow=True)   # under-cabinet light
p = piece('dining'); desk(p, 0, 0, 1.4, 0.85, WOOD, 0.76)
for (x, y, f) in ((-0.4, -0.7, 1), (0.4, -0.7, 1), (-0.4, 0.7, -1), (0.4, 0.7, -1)):
    chair(p, x, y, (0.3, 0.22, 0.15), face=f)
p.cyl(0, 0, 1.5, 2.4, 0.01, DARK, 4); p.cyl(0, 0, 1.4, 1.55, 0.25, WARM, 8, glow=True)
p = piece('bunk_bed'); p.box(-0.5, 0.5, -1.0, 1.0, 0.25, 0.45, (0.4, 0.6, 0.85)); p.box(-0.5, 0.5, -1.0, 1.0, 1.35, 1.55, (0.9, 0.5, 0.5))
for x in (-0.5, 0.5):
    for y in (-1.0, 1.0):
        p.box(x - 0.04, x + 0.04, y - 0.04, y + 0.04, 0, 1.8, (0.95, 0.85, 0.3))
p = piece('toys')
for (x, y, s, c) in ((-0.3, 0, 0.3, (0.9, 0.2, 0.2)), (0.2, 0.2, 0.25, (0.2, 0.5, 0.9)), (0.1, -0.3, 0.2, (0.95, 0.8, 0.1)), (0.5, -0.1, 0.35, (0.2, 0.7, 0.3))):
    p.box(x - s / 2, x + s / 2, y - s / 2, y + s / 2, 0, s, c)
p.box(-1.2, 1.2, -0.9, 0.9, 0, 0.012, (0.3, 0.6, 0.85))
# ---- transport (BLD-08)
p = piece('console'); p.box(-1.4, 1.4, -0.35, 0.35, 0, 0.8, (0.12, 0.13, 0.15)); p.box(-1.45, 1.45, -0.4, 0.4, 0.8, 0.84, (0.25, 0.26, 0.28))
for x in (-0.9, 0, 0.9):
    monitor(p, x, 0.25, 0.75, h0=0.9, col=(0.25, 0.85, 0.6) if x else (0.95, 0.6, 0.2))
for x in (-0.9, 0.0, 0.9):
    chair(p, x, -0.75, (0.1, 0.12, 0.16))
p = piece('wall_screen')   # video wall on the core wall, faces the window
p.box(-2.5, 2.5, -0.1, 0.0, 0.9, 3.0, DARK)
for i in range(4):
    for j in range(2):
        p.box(-2.45 + i * 1.235, -2.45 + i * 1.235 + 1.2, -0.105, -0.1, 0.95 + j * 1.02, 0.95 + j * 1.02 + 0.99,
              [(0.15, 0.45, 0.95), (0.2, 0.85, 0.55), (0.95, 0.55, 0.2), (0.3, 0.6, 1.0)][(i + j) % 4], glow=True)
p = piece('bench_row')
for x in (-1.2, -0.4, 0.4, 1.2):
    p.box(x - 0.36, x + 0.36, -0.25, 0.25, 0.42, 0.47, (0.35, 0.37, 0.4)); p.box(x - 0.36, x + 0.36, 0.2, 0.25, 0.47, 0.88, (0.35, 0.37, 0.4))
p.box(-1.6, 1.6, -0.05, 0.05, 0.0, 0.42, METAL)
p = piece('machine'); p.box(-0.8, 0.8, -0.5, 0.5, 0, 1.25, (0.25, 0.36, 0.32)); p.box(-0.6, 0.6, -0.4, 0.4, 1.25, 1.6, (0.3, 0.32, 0.34))
p.box(-0.15, 0.15, -0.51, -0.5, 0.9, 1.05, (0.2, 1.0, 0.35), glow=True); p.box(-0.82, 0.82, -0.6, -0.5, 0, 0.08, (0.95, 0.75, 0.1))
p = piece('workbench'); desk(p, 0, 0, 2.0, 0.8, (0.45, 0.4, 0.32), 0.9); p.box(-1.0, 1.0, 0.35, 0.4, 0.9, 1.8, (0.3, 0.3, 0.32))
for k in range(6):
    p.box(-0.9 + k * 0.32, -0.8 + k * 0.32, 0.3, 0.35, 1.3, 1.5, (0.6, 0.6, 0.62))
p = piece('lockers')
for k in range(5):
    p.box(-1.25 + k * 0.5, -0.77 + k * 0.5, -0.25, 0.25, 0, 1.9, (0.2, 0.35, 0.5) if k % 2 else (0.3, 0.3, 0.33))
p.box(-1.0, 1.0, -0.9, -0.6, 0.4, 0.45, WOOD)
# ---- hotel (BLD-04)
p = piece('armchair')   # faces the room (its back to the window)
p.box(-0.4, 0.4, -0.38, 0.38, 0.1, 0.42, (0.55, 0.42, 0.3)); p.box(-0.4, 0.4, -0.38, -0.22, 0.42, 0.9, (0.55, 0.42, 0.3))
p.box(-0.45, -0.35, -0.38, 0.38, 0.1, 0.62, (0.48, 0.36, 0.26)); p.box(0.35, 0.45, -0.38, 0.38, 0.1, 0.62, (0.48, 0.36, 0.26))
p.box(-0.45, 0.45, -0.38, 0.38, 0.0, 0.1, DARK)
p = piece('twin_beds')   # two singles, headboards against the core side, a nightstand between them
for x in (-0.75, 0.75):
    p.box(x - 0.48, x + 0.48, -1.0, 1.0, 0.0, 0.3, WOOD); p.box(x - 0.46, x + 0.46, -0.98, 0.98, 0.3, 0.5, (0.86, 0.85, 0.81))
    p.box(x - 0.46, x + 0.46, -0.98, 0.1, 0.5, 0.55, (0.55, 0.22, 0.17)); p.box(x - 0.3, x + 0.3, 0.5, 0.85, 0.5, 0.64, WHITE)
    p.box(x - 0.5, x + 0.5, 0.95, 1.05, 0.0, 1.1, WOOD)
p.box(-0.2, 0.2, 0.6, 1.0, 0, 0.5, WOOD); p.cyl(0, 0.8, 0.5, 0.75, 0.1, WARM, 6, glow=True)
p = piece('hotel_desk')   # vanity desk on a partition, mirror, chair and a desk lamp
p.box(-0.7, 0.7, -0.27, 0.27, 0.71, 0.75, WOOD); p.box(-0.7, -0.62, -0.27, 0.27, 0, 0.71, WOOD); p.box(0.62, 0.7, -0.27, 0.27, 0, 0.71, WOOD)
p.box(-0.5, 0.5, 0.25, 0.28, 1.05, 1.75, (0.55, 0.6, 0.65)); chair(p, 0, -0.5, (0.45, 0.2, 0.15), face=1)
p.cyl(0.5, 0.05, 0.75, 1.15, 0.012, METAL, 5); p.cyl(0.5, 0.05, 1.1, 1.25, 0.12, WARM, 8, glow=True)
# ---- the building itself (scaled per instance in the game)
p = piece('panel')   # one ceiling light panel 1.2 x 0.6, glows
p.box(-0.6, 0.6, -0.3, 0.3, -0.02, 0.0, (1.0, 1.0, 1.0), glow=True)
p = piece('unit')    # unit cube, origin bottom centre: slabs, columns, core walls (scaled), lit by its baked shade
p.box(-0.5, 0.5, -0.5, 0.5, 0.0, 1.0, (1.0, 1.0, 1.0))

objs = []
for name, pc in P.items():
    objs += pc.build()
bpy.ops.object.select_all(action='DESELECT')
for o in objs:
    o.select_set(True)
bpy.ops.export_scene.gltf(filepath=OUT, export_format='GLB', use_selection=True, export_vertex_color='ACTIVE',
                          export_all_vertex_colors=False, export_active_vertex_color_when_no_material=True, export_materials='NONE', export_yup=True)
tris = {o.name: sum(len(p.vertices) - 2 for p in o.data.polygons) for o in objs}
with open(OUT.replace('.glb', '.json'), 'w') as fh:
    json.dump({'pieces': list(P), 'triangles': tris}, fh, indent=1)
print('KIT', len(P), 'pieces', sum(tris.values()), 'triangles')

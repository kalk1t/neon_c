"""Living Megacity, the new city's leisure and country props (Blender 5.2, headless). Each prop is one GLB, metres,
origin = its centre on the ground, Blender Z up (glTF Y up; Blender (x, y) -> game (x, -z)). The sport kits match the
lines the ground shader paints on a 90 m lot (game/src/newcity.js GROUND_GLSL):

  PROP-tennis    two courts at x = +-12 (long axis y): nets, posts, perimeter fence with windscreen, 4 light masts
  PROP-basket    two courts at x = +-10: hoops at y = +-12.4, fence, benches, 4 light masts
  PROP-pool      50 x 21 pool at y = +6 (game z -6), kids' pool at (-22, -25): loungers, parasols, lifeguard chair,
                 low glass fence, pool house, lights
  PROP-football  pitch 50 x 80: goals at y = +-40, a small stand, 4 light masts
  PROP-stadium   the bowl around the painted track (rounded rectangle 70 x 88, r 18): 16 tiers, roof ring,
                 lit concourse, 4 light towers (55 m)
  PROP-barn  PROP-silo  PROP-house  PROP-fence (10 m)  PROP-hay   the farmstead
  PROP-club  PROP-flag  PROP-cart                                    the golf club

  blender -b --factory-startup --python props.py -- <out_dir>
"""
import bpy, bmesh, sys, os, json, math, random
from mathutils import Vector

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L
from lib_bld import block, cyl

OUT = os.path.abspath(sys.argv[sys.argv.index('--') + 1])
os.makedirs(OUT, exist_ok=True)
L.start()
L.common_mats()
S = L.S
for a in [
    ('net_dark', (0.03, 0.03, 0.035), 0.8, 0.0), ('windscreen', (0.03, 0.12, 0.07), 0.85, 0.0),
    ('post_green', (0.05, 0.2, 0.1), 0.5, 0.6), ('backboard', (0.85, 0.87, 0.9), 0.3, 0.0),
    ('rim_orange', (0.9, 0.3, 0.05), 0.4, 0.7), ('net_white', (0.9, 0.9, 0.9), 0.7, 0.0),
    ('seat_blue', (0.05, 0.16, 0.5), 0.5, 0.0), ('seat_red', (0.55, 0.06, 0.06), 0.5, 0.0),
    ('seat_white', (0.8, 0.8, 0.82), 0.5, 0.0), ('canvas', (0.92, 0.9, 0.84), 0.8, 0.0),
    ('canvas_teal', (0.05, 0.45, 0.5), 0.8, 0.0), ('glass_rail', (0.5, 0.7, 0.75), 0.1, 0.0),
    ('barn_red', (0.42, 0.06, 0.04), 0.8, 0.0), ('trim_white', (0.85, 0.83, 0.78), 0.7, 0.0),
    ('shingle', (0.12, 0.11, 0.11), 0.9, 0.0), ('silo_metal', (0.6, 0.62, 0.64), 0.35, 0.9),
    ('hay', (0.6, 0.47, 0.18), 0.95, 0.0), ('fence_wood', (0.4, 0.3, 0.2), 0.9, 0.0),
    ('house_white', (0.82, 0.8, 0.74), 0.8, 0.0), ('cart_white', (0.88, 0.88, 0.86), 0.4, 0.0),
    ('flag_red', (0.8, 0.05, 0.05), 0.6, 0.0), ('tyre', (0.03, 0.03, 0.03), 0.9, 0.0),
    ('window_dark', (0.04, 0.05, 0.07), 0.15, 0.3),
]:
    L.mat(*a)
L.mat('flood', (1.0, 1.0, 1.0), 0.3, 0.0, (0.9, 0.95, 1.0), 6.0)
L.mat('window_warm', (1.0, 0.8, 0.5), 0.4, 0.0, (1.0, 0.72, 0.42), 2.2)
L.mat('pool_light', (0.4, 0.9, 1.0), 0.3, 0.0, (0.3, 0.85, 1.0), 2.5)
L.mat('concourse', (1.0, 0.9, 0.7), 0.4, 0.0, (1.0, 0.82, 0.55), 2.0)
rnd = random.Random(31)


def mesh(name, verts, faces, m, c, mats=None):
    me = bpy.data.meshes.new(name)
    me.from_pydata(verts, [], faces)
    me.update()
    me.materials.append(L.M[m])
    for extra in (mats or []):
        me.materials.append(L.M[extra])
    bm = bmesh.new()
    bm.from_mesh(me)
    uv = bm.loops.layers.uv.new('UVMap')
    for f in bm.faces:
        for l in f.loops:
            l[uv].uv = (0.02, 0.02)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    bm.to_mesh(me)
    bm.free()
    o = bpy.data.objects.new(name, me)
    c.objects.link(o)
    return o


def gable(name, x0, x1, y0, y1, z0, z1, m, c, over=0.4):
    """A pitched roof: ridge along y, eaves at z0, ridge at z1, overhanging by over."""
    x0 -= over; x1 += over; y0 -= over; y1 += over
    xm = (x0 + x1) / 2
    v = [(x0, y0, z0), (x1, y0, z0), (xm, y0, z1), (x0, y1, z0), (x1, y1, z0), (xm, y1, z1)]
    f = [(0, 1, 2), (3, 5, 4), (0, 2, 5, 3), (1, 4, 5, 2), (0, 3, 4, 1)]
    return mesh(name, v, f, m, c)


def mast(parts, x, y, h, c, face=0.0):
    """A floodlight mast: pole and a lamp head turned to face angle (radians, 0 = +y)."""
    parts.append(cyl('mast', x, y, 0, h, 0.22, 'steel_rail', 8, coll=c))
    hx, hy = math.sin(face) * 0.6, math.cos(face) * 0.6
    parts.append(block('head', x + hx - 1.4, x + hx + 1.4, y + hy - 0.35, y + hy + 0.35, h - 1.2, h + 0.4, 'steel_dark', coll=c))
    parts.append(block('lamp', x + hx * 1.7 - 1.25, x + hx * 1.7 + 1.25, y + hy * 1.7 - 0.06, y + hy * 1.7 + 0.06, h - 1.05, h + 0.25, 'flood', coll=c))


def fence(parts, x0, x1, y0, y1, h, c, post='post_green', screen='windscreen', step=3.0, gate=None):
    """A sports fence: posts every step, top and mid rails, a dark windscreen on the lower 2 m."""
    for (a0, b0, a1, b1) in [(x0, y0, x1, y0), (x0, y1, x1, y1), (x0, y0, x0, y1), (x1, y0, x1, y1)]:
        L_ = math.hypot(a1 - a0, b1 - b0)
        n = max(1, int(L_ / step))
        for k in range(n + 1):
            px, py = a0 + (a1 - a0) * k / n, b0 + (b1 - b0) * k / n
            parts.append(cyl('post', px, py, 0, h, 0.05, post, 6, coll=c))
        t = 0.04
        xa, xb, ya, yb = min(a0, a1) - t, max(a0, a1) + t, min(b0, b1) - t, max(b0, b1) + t
        parts.append(block('rail', xa, xb, ya, yb, h - 0.06, h, post, coll=c))
        parts.append(block('rail', xa, xb, ya, yb, 1.95, 2.0, post, coll=c))
        parts.append(block('screen', xa + 0.02, xb - 0.02, ya + 0.02, yb - 0.02, 0.0, 1.9, screen, coll=c))


def bench(parts, x, y, c, rot=False, w=3.0):
    if rot:
        parts.append(block('bench', x - 0.25, x + 0.25, y - w / 2, y + w / 2, 0.42, 0.48, 'wood_deck', coll=c))
    else:
        parts.append(block('bench', x - w / 2, x + w / 2, y - 0.25, y + 0.25, 0.42, 0.48, 'wood_deck', coll=c))


# ---------------------------------------------------------------------------------------------------------------- sport

def tennis(c):
    p = []
    for sx in (-12, 12):
        p.append(block('net', sx - 6.4, sx + 6.4, -0.02, 0.02, 0.0, 0.95, 'net_dark', coll=c))
        p.append(block('tape', sx - 6.4, sx + 6.4, -0.03, 0.03, 0.91, 1.0, 'net_white', coll=c))
        for px in (sx - 6.4, sx + 6.4):
            p.append(cyl('npost', px, 0, 0, 1.07, 0.05, 'post_green', 8, coll=c))
        p.append(cyl('umpire', sx + 7.6, 0, 0, 1.8, 0.07, 'post_green', 6, coll=c))
        p.append(block('uchair', sx + 7.2, sx + 8.0, -0.4, 0.4, 1.8, 2.3, 'post_green', coll=c))
        bench(p, sx + 8.0, 3.0, c, rot=True, w=2.0)
    fence(p, -21, 21, -18, 18, 3.6, c)
    for x, y in [(-21, -18), (21, -18), (-21, 18), (21, 18)]:
        mast(p, x * 1.04, y * 1.04, 12, c, math.atan2(-x, -y))
    return p


def basket(c):
    p = []
    for sx in (-10, 10):
        for sy in (-1, 1):
            y = sy * 14.9
            p.append(cyl('pole', sx, y, 0, 3.4, 0.09, 'steel_dark', 8, coll=c))
            p.append(block('arm', sx - 0.08, sx + 0.08, min(y, y - sy * 1.25), max(y, y - sy * 1.25), 3.3, 3.42, 'steel_dark', coll=c))
            yb = sy * 13.65
            p.append(block('board', sx - 0.9, sx + 0.9, yb - 0.03, yb + 0.03, 2.9, 3.95, 'backboard', coll=c))
            # rim: a thin ring of 8 small boxes
            yr = sy * 13.4
            for k in range(8):
                a = k * math.pi / 4
                p.append(block('rim', sx + math.cos(a) * 0.23 - 0.04, sx + math.cos(a) * 0.23 + 0.04,
                               yr + math.sin(a) * 0.23 - 0.04, yr + math.sin(a) * 0.23 + 0.04, 3.03, 3.07, 'rim_orange', coll=c))
            p.append(cyl('netc', sx, yr, 2.65, 3.03, 0.2, 'net_white', 8, coll=c))
        bench(p, sx + sx * 0.9, 0, c, rot=True, w=4)
    fence(p, -19, 19, -17, 17, 4.0, c)
    for x, y in [(-19, -17), (19, -17), (-19, 17), (19, 17)]:
        mast(p, x * 1.04, y * 1.04, 11, c, math.atan2(-x, -y))
    return p


def pool(c):
    p = []
    # coping around the big pool (game z -6 = Blender y +6) and the kids' pool, lane-end blocks, ladders
    x0, x1, y0, y1 = -25, 25, 6 - 10.5, 6 + 10.5
    for (a, b, cc, d) in [(x0 - 0.5, x1 + 0.5, y0 - 0.5, y0), (x0 - 0.5, x1 + 0.5, y1, y1 + 0.5), (x0 - 0.5, x0, y0, y1), (x1, x1 + 0.5, y0, y1)]:
        p.append(block('coping', a, b, cc, d, 0.0, 0.12, 'pool_tile', coll=c))
    for k in range(9):
        y = y0 + 1.25 + k * 2.5
        if k < 8:
            p.append(block('startblock', x0 - 1.0, x0 - 0.3, y - 0.3, y + 0.3, 0.0, 0.7, 'trim_white', coll=c))
    for k in range(1, 8):                                   # lane ropes: floats as thin lit strips
        y = y0 + k * 2.625
        p.append(block('lane', x0, x1, y - 0.05, y + 0.05, 0.03, 0.09, 'seat_red' if k % 2 else 'seat_blue', coll=c))
    for x in (-15, 0, 15):
        p.append(block('light', x - 0.4, x + 0.4, y0 - 0.02, y0 + 0.02, -0.2, 0.0, 'pool_light', coll=c))
    for x in (x0 + 2, x1 - 2):
        for y in (y0 - 0.2, y1 + 0.2):
            p.append(cyl('ladder', x, y, 0, 1.0, 0.03, 'steel_rail', 6, coll=c))
            p.append(cyl('ladder', x + 0.5, y, 0, 1.0, 0.03, 'steel_rail', 6, coll=c))
    p.append(cyl('kidring', -22, -25, 0, 0.12, 7.4, 'pool_tile', 24, coll=c))
    # loungers and parasols on the deck (game z +8..+20 = Blender y -8..-20) and along the long side
    for k in range(10):
        x = -22 + k * 4.6
        for y in (-12.5, -18.0):
            if y < -15 and x < -12:                      # the kids' pool (game (-22, +25), r 7)
                continue
            p.append(block('lounger', x - 0.35, x + 0.35, y - 1.0, y + 1.0, 0.3, 0.42, 'trim_white', coll=c))
            p.append(block('back', x - 0.35, x + 0.35, y + 0.6, y + 1.0, 0.42, 0.9, 'trim_white', coll=c))
        if k % 2 == 0:
            p.append(cyl('pstem', x + 2.3, -15.2, 0, 2.4, 0.04, 'steel_rail', 6, coll=c))
            cone = bmesh.new()
            bmesh.ops.create_cone(cone, cap_ends=False, segments=8, radius1=1.6, radius2=0.05, depth=0.5)
            for v in cone.verts:
                v.co = Vector((v.co.x + x + 2.3, v.co.y - 15.2, v.co.z + 2.4))
            me = bpy.data.meshes.new('parasol'); cone.to_mesh(me); cone.free()
            me.materials.append(L.M['canvas_teal' if k % 4 else 'canvas'])
            o = bpy.data.objects.new('parasol', me); c.objects.link(o); p.append(o)
    # lifeguard chair, pool house, low glass fence, lights
    p.append(block('lg_legs', 26.5, 27.5, 5.5, 6.5, 0.0, 2.2, 'trim_white', coll=c))
    p.append(block('lg_seat', 26.3, 27.7, 5.3, 6.7, 2.2, 2.4, 'flag_red', coll=c))
    p.append(block('house', 28, 40, 18, 30, 0.0, 3.6, 'house_white', coll=c))
    p.append(block('house_win', 27.95, 28.0, 20, 28, 0.9, 2.6, 'window_warm', coll=c))
    p.append(block('house_roof', 27.6, 40.4, 17.6, 30.4, 3.6, 3.9, 'steel_dark', coll=c))
    for (a, b, cc, d) in [(-40, 40, -40, -39.9), (-40, 40, 39.9, 40), (-40, -39.9, -40, 40), (39.9, 40, -40, 40)]:
        p.append(block('glass', a, b, cc, d, 0.0, 1.1, 'glass_rail', coll=c))
    for x, y in [(-38, -38), (38, -38), (-38, 38), (38, 38), (0, -38), (0, 38)]:
        p.append(cyl('lpole', x, y, 0, 6, 0.08, 'steel_dark', 6, coll=c))
        p.append(block('lhead', x - 0.3, x + 0.3, y - 0.3, y + 0.3, 6.0, 6.3, 'window_warm', coll=c))
    return p


def football(c):
    p = []
    for sy in (-1, 1):
        y = sy * 40
        for x in (-3.66, 3.66):
            p.append(cyl('gpost', x, y, 0, 2.44, 0.06, 'trim_white', 8, coll=c))
        p.append(block('bar', -3.72, 3.72, y - 0.06, y + 0.06, 2.38, 2.5, 'trim_white', coll=c))
        p.append(block('gnet', -3.66, 3.66, y + sy * 1.8 - 0.01, y + sy * 1.8 + 0.01, 0.0, 2.2, 'net_white', coll=c))
        p.append(block('gtop', -3.66, 3.66, min(y, y + sy * 1.8), max(y, y + sy * 1.8), 2.42, 2.44, 'net_white', coll=c))
    # a small stand on the east side: 5 steps
    for k in range(5):
        p.append(block('step', 28 + k * 0.9, 29 + k * 0.9, -20, 20, 0.0, 0.5 + k * 0.5, 'concrete_pale', coll=c))
        p.append(block('seats', 28.1 + k * 0.9, 28.6 + k * 0.9, -19.5, 19.5, 0.5 + k * 0.5, 0.75 + k * 0.5, 'seat_blue' if k % 2 else 'seat_white', coll=c))
    p.append(block('dugout', -30, -28, -6, 6, 0.0, 2.2, 'steel_dark', coll=c))
    for x, y in [(-30, -44), (30, -44), (-30, 44), (30, 44)]:
        mast(p, x, y, 22, c, math.atan2(-x, -y))
    return p


def rr_loop(hx, hy, r, z, n=10):
    """A rounded rectangle (half sizes hx, hy, corner radius r) at height z: counter-clockwise points."""
    pts = []
    for cx, cy, a0 in [(hx - r, hy - r, 0), (-(hx - r), hy - r, math.pi / 2), (-(hx - r), -(hy - r), math.pi), (hx - r, -(hy - r), 1.5 * math.pi)]:
        for k in range(n + 1):
            a = a0 + (math.pi / 2) * k / n
            pts.append((cx + math.cos(a) * r, cy + math.sin(a) * r, z))
    return pts


def loft(name, loops, m, c, mat_per_band=None):
    """Quad strips between consecutive loops (same point count); mat_per_band: material index per band."""
    verts, faces, fm = [], [], []
    n = len(loops[0])
    for lp in loops:
        verts += lp
    for b in range(len(loops) - 1):
        for k in range(n):
            a0, a1 = b * n + k, b * n + (k + 1) % n
            faces.append((a0, a1, a1 + n, a0 + n))
            fm.append(mat_per_band[b] if mat_per_band else 0)
    mats = sorted(set(fm))
    names = [m] if not mat_per_band else None
    o = mesh(name, verts, faces, m if not mat_per_band else mat_per_band_names[mats[0]], c,
             None if not mat_per_band else [mat_per_band_names[i] for i in mats[1:]])
    if mat_per_band:
        idx = {mm: i for i, mm in enumerate(mats)}
        for f, mm in zip(o.data.polygons, fm):
            f.material_index = idx[mm]
    return o


mat_per_band_names = ['seat_blue', 'concrete_pale', 'seat_red', 'seat_white', 'concrete_struct', 'steel_dark', 'concourse']


def stadium(c):
    p = []
    HX, HY, R = 70.0, 88.0, 18.0          # the track's outer edge (the ground shader's oval)
    loops, bands = [], []
    o, z = 2.0, 1.2
    loops.append(rr_loop(HX + o, HY + o, R + o, 0.0))
    loops.append(rr_loop(HX + o, HY + o, R + o, z)); bands.append(4)          # the pitch wall
    for k in range(16):                                                          # tiers: tread then riser
        o2 = o + 3.2
        loops.append(rr_loop(HX + o2, HY + o2, R + o2, z)); bands.append(0 if (k // 4) % 2 == 0 else 2)
        z2 = z + 1.6
        loops.append(rr_loop(HX + o2, HY + o2, R + o2, z2)); bands.append(1)
        o, z = o2, z2
    top_o, top_z = o, z
    loops.append(rr_loop(HX + o + 2, HY + o + 2, R + o + 2, z)); bands.append(4)   # walkway at the top
    loops.append(rr_loop(HX + o + 2, HY + o + 2, R + o + 2, z + 1.2)); bands.append(4)
    p.append(loft('bowl', loops, 'seat_blue', c, bands))
    # outer wall down to the ground, with a lit concourse band
    oo = top_o + 2
    wall = [rr_loop(HX + oo, HY + oo, R + oo, z + 1.2), rr_loop(HX + oo, HY + oo, R + oo, 11.0),
            rr_loop(HX + oo, HY + oo, R + oo, 9.0), rr_loop(HX + oo, HY + oo, R + oo, 0.0)]
    p.append(loft('wall', wall, 'concrete_struct', c, [4, 6, 4]))
    # the roof ring: a thin canopy over the upper tiers, on columns
    rz = top_z + 9.0
    roof = [rr_loop(HX + oo + 1, HY + oo + 1, R + oo + 1, rz), rr_loop(HX + oo - 26, HY + oo - 26, R + oo - 26, rz + 1.5),
            rr_loop(HX + oo - 26, HY + oo - 26, R + oo - 26, rz + 0.9), rr_loop(HX + oo + 1, HY + oo + 1, R + oo + 1, rz - 0.6)]
    p.append(loft('roof', roof + [roof[0]], 'steel_dark', c, [5, 5, 5, 5]))
    for (x, y, zz) in rr_loop(HX + oo, HY + oo, R + oo, 0.0, 3)[::2]:
        p.append(cyl('col', x, y, z, rz, 0.5, 'steel_dark', 8, coll=c))
    # a lit strip under the roof edge (the bowl's own floodlights)
    inner = [rr_loop(HX + oo - 25.5, HY + oo - 25.5, R + oo - 25.5, rz + 0.85), rr_loop(HX + oo - 25.5, HY + oo - 25.5, R + oo - 25.5, rz + 0.55)]
    p.append(loft('rooflight', inner, 'flood', c))
    # four light towers on the diagonals
    for sx in (-1, 1):
        for sy in (-1, 1):
            x, y = sx * (HX + oo + 6), sy * (HY + oo + 6)
            p.append(cyl('tower', x, y, 0, 55, 0.9, 'steel_rail', 10, coll=c))
            fx, fy = -sx * 0.7, -sy * 0.7
            p.append(block('rack', x - 4, x + 4, y - 4, y + 4, 53, 54, 'steel_dark', coll=c))
            for k in range(3):
                for j in range(4):
                    p.append(block('fl', x + fx * 4 - 3.4 + j * 2.2, x + fx * 4 - 1.6 + j * 2.2, y + fy * 4 - 0.1, y + fy * 4 + 0.1,
                                   54.5 + k * 1.6, 55.8 + k * 1.6, 'flood', coll=c))
    # gates: four lit portals in the wall
    for (x, y) in [(HX + oo, 0), (-(HX + oo), 0), (0, HY + oo), (0, -(HY + oo))]:
        if x:
            p.append(block('gate', x - 0.3, x + 0.3, -6, 6, 0.0, 6.0, 'concourse', coll=c))
        else:
            p.append(block('gate', -6, 6, y - 0.3, y + 0.3, 0.0, 6.0, 'concourse', coll=c))
    return p


# ---------------------------------------------------------------------------------------------------------------- farm

def barn(c):
    p = [block('walls', -7, 7, -11, 11, 0, 7, 'barn_red', coll=c)]
    p.append(gable('roof', -7, 7, -11, 11, 7, 12.5, 'shingle', c))
    for y in (-11.02, 11.02):
        p.append(block('door', -2.6, 2.6, y - 0.04, y + 0.04, 0, 5.2, 'trim_white', coll=c))
        p.append(block('doorin', -2.2, 2.2, y - 0.06, y + 0.06, 0.2, 4.8, 'barn_red', coll=c))
        p.append(block('loft', -1.0, 1.0, y - 0.06, y + 0.06, 8.0, 9.6, 'window_warm', coll=c))
    for x in (-7.02, 7.02):
        for y in (-6, 0, 6):
            p.append(block('win', x - 0.05, x + 0.05, y - 0.8, y + 0.8, 3.0, 4.4, 'window_warm', coll=c))
    p.append(block('trim', -7.05, 7.05, -11.05, 11.05, 6.9, 7.1, 'trim_white', coll=c))
    return p


def silo(c):
    p = [cyl('silo', 0, 0, 0, 16, 3.0, 'silo_metal', 20, coll=c)]
    for z in (4, 8, 12):
        p.append(cyl('band', 0, 0, z, z + 0.25, 3.05, 'steel_dark', 20, coll=c))
    dome = bmesh.new()
    bmesh.ops.create_uvsphere(dome, u_segments=20, v_segments=8, radius=3.0)
    bmesh.ops.delete(dome, geom=[v for v in dome.verts if v.co.z < -0.01], context='VERTS')
    for v in dome.verts:
        v.co.z = v.co.z * 0.6 + 16
    me = bpy.data.meshes.new('dome'); dome.to_mesh(me); dome.free()
    me.materials.append(L.M['silo_metal'])
    o = bpy.data.objects.new('dome', me); c.objects.link(o); p.append(o)
    p.append(block('ladder', 2.95, 3.1, -0.3, 0.3, 0.5, 16, 'steel_dark', coll=c))
    return p


def house(c):
    p = [block('walls', -6, 6, -4.5, 4.5, 0, 6.0, 'house_white', coll=c)]
    p.append(gable('roof', -6, 6, -4.5, 4.5, 6.0, 9.0, 'shingle', c))   # ridge along y
    p.append(block('porch', -6, 6, -7.0, -4.5, 0, 0.4, 'wood_deck', coll=c))
    p.append(block('porchroof', -6.2, 6.2, -7.2, -4.5, 2.8, 3.0, 'shingle', coll=c))
    for x in (-5.8, -2, 2, 5.8):
        p.append(cyl('pcol', x, -7.0, 0.4, 2.8, 0.1, 'trim_white', 6, coll=c))
    p.append(block('door', -0.6, 0.6, -4.55, -4.5, 0.4, 2.5, 'wood_deck', coll=c))
    for x in (-3.8, 3.8):
        for z in (1.2, 4.0):
            p.append(block('win', x - 0.7, x + 0.7, -4.56, -4.5, z, z + 1.3, 'window_warm', coll=c))
    p.append(block('chimney', 3, 4, 1, 2, 7, 10.0, 'brick_cornice' if 'brick_cornice' in L.M else 'barn_red', coll=c))
    return p


def fence_seg(c):
    """10 m of wooden fence along x: posts every 2.5 m, two rails."""
    p = []
    for k in range(5):
        x = -5 + k * 2.5
        p.append(block('post', x - 0.08, x + 0.08, -0.08, 0.08, 0, 1.3, 'fence_wood', coll=c))
    for z in (0.55, 1.05):
        p.append(block('rail', -5, 5, -0.04, 0.04, z, z + 0.12, 'fence_wood', coll=c))
    return p


def hay(c):
    p = []
    b = bmesh.new()
    bmesh.ops.create_cone(b, cap_ends=True, segments=14, radius1=0.8, radius2=0.8, depth=1.3)
    for v in b.verts:
        v.co = Vector((v.co.x, v.co.z, v.co.y + 0.8))          # lying on its side (axis along y)
    me = bpy.data.meshes.new('bale'); b.to_mesh(me); b.free()
    me.materials.append(L.M['hay'])
    o = bpy.data.objects.new('bale', me); c.objects.link(o); p.append(o)
    return p


# ---------------------------------------------------------------------------------------------------------------- golf

def club(c):
    p = [block('ground', -18, 18, -9, 9, 0, 4.2, 'limestone', coll=c)]
    p.append(block('glass', -17.9, 17.9, -9.05, -8.95, 0.4, 3.8, 'window_warm', coll=c))
    for x in range(-16, 17, 4):
        p.append(block('mull', x - 0.12, x + 0.12, -9.15, -8.9, 0, 4.2, 'bronze', coll=c))
    p.append(block('upper', -14, 14, -7, 9, 4.2, 7.8, 'house_white', coll=c))
    p.append(block('glass2', -13.9, 13.9, -7.05, -6.95, 4.8, 7.4, 'window_warm', coll=c))
    p.append(block('terrace', -18.5, 18.5, -9.5, 9.5, 4.2, 4.45, 'wood_deck', coll=c))
    p.append(block('rail', -18.5, 18.5, -9.6, -9.45, 4.45, 5.4, 'glass_rail', coll=c))
    p.append(block('roof', -14.6, 14.6, -7.6, 9.6, 7.8, 8.2, 'steel_dark', coll=c))
    p.append(block('canopy', -4, 4, -14, -9, 3.4, 3.7, 'steel_dark', coll=c))
    for x in (-3.8, 3.8):
        p.append(cyl('ccol', x, -13.8, 0, 3.4, 0.12, 'bronze', 8, coll=c))
    p.append(block('sign', -4, 4, -14.05, -13.95, 3.7, 4.4, 'concourse', coll=c))
    return p


def flag(c):
    return [cyl('pin', 0, 0, 0, 2.4, 0.02, 'trim_white', 6, coll=c),
            block('flag', 0.02, 0.62, -0.01, 0.01, 1.95, 2.35, 'flag_red', coll=c),
            cyl('cup', 0, 0, -0.01, 0.01, 0.06, 'tyre', 8, coll=c)]


def cart(c):
    p = [block('body', -0.6, 0.6, -1.2, 1.2, 0.35, 0.95, 'cart_white', coll=c)]
    p.append(block('seat', -0.55, 0.55, -0.1, 0.5, 0.95, 1.05, 'steel_dark', coll=c))
    p.append(block('back', -0.55, 0.55, 0.45, 0.55, 1.05, 1.5, 'steel_dark', coll=c))
    p.append(block('roof', -0.65, 0.65, -0.9, 1.0, 1.9, 1.96, 'cart_white', coll=c))
    for x, y in [(-0.55, -0.85), (0.55, -0.85), (-0.55, 0.95), (0.55, 0.95)]:
        p.append(cyl('strut', x, y, 0.95, 1.9, 0.025, 'steel_rail', 6, coll=c))
    p.append(block('bag', -0.3, 0.3, 0.75, 1.15, 0.95, 1.7, 'flag_red', coll=c))
    for x, y in [(-0.6, -0.8), (0.6, -0.8), (-0.6, 0.8), (0.6, 0.8)]:
        w = bmesh.new()
        bmesh.ops.create_cone(w, cap_ends=True, segments=10, radius1=0.22, radius2=0.22, depth=0.18)
        for v in w.verts:
            v.co = Vector((v.co.z + x, v.co.y + y, v.co.x + 0.22))
        me = bpy.data.meshes.new('wheel'); w.to_mesh(me); w.free()
        me.materials.append(L.M['tyre'])
        o = bpy.data.objects.new('wheel', me); c.objects.link(o); p.append(o)
    return p


# ---------------------------------------------------------------------------------------------------------------- animals
# low-poly stand-ins (a body, a neck and head, four legs), facing +y; seen from a car they are a few pixels tall

def ellip(name, x, y, z, rx, ry, rz, m, c, seg=8):
    b = bmesh.new()
    bmesh.ops.create_uvsphere(b, u_segments=seg, v_segments=max(4, seg // 2), radius=1.0)
    for v in b.verts:
        v.co = Vector((v.co.x * rx + x, v.co.y * ry + y, v.co.z * rz + z))
    me = bpy.data.meshes.new(name); b.to_mesh(me); b.free()
    me.materials.append(L.M[m])
    o = bpy.data.objects.new(name, me); c.objects.link(o)
    return o


def beast(c, L_, W, H_, leg, body, head, hl, neck=0.0, legm=None, extra=None):
    """L_ body length, W width, H_ body height (its centre at leg + H_/2), leg height, head size hl."""
    p = [ellip('body', 0, 0, leg + H_ / 2, W / 2, L_ / 2, H_ / 2, body, c)]
    for sx in (-1, 1):
        for sy in (-1, 1):
            p.append(cyl('leg', sx * W * 0.3, sy * L_ * 0.33, 0, leg + H_ * 0.3, max(0.04, W * 0.09), legm or body, 6, coll=c))
    hy, hz = L_ / 2 + hl * 0.3, leg + H_ * 0.75 + neck
    if neck:
        p.append(cyl('neck', 0, L_ / 2 - hl * 0.1, leg + H_ / 2, hz, W * 0.18, body, 6, coll=c))
    p.append(ellip('head', 0, hy, hz, hl * 0.32, hl * 0.6, hl * 0.36, head, c, 6))
    if extra:
        p += extra
    return p


for a in [('horse_bay', (0.25, 0.12, 0.06), 0.6, 0.0), ('horse_dark', (0.06, 0.04, 0.03), 0.6, 0.0),
          ('cow_white', (0.8, 0.78, 0.74), 0.8, 0.0), ('cow_black', (0.05, 0.05, 0.05), 0.8, 0.0),
          ('wool', (0.85, 0.83, 0.76), 0.95, 0.0), ('sheep_face', (0.08, 0.07, 0.07), 0.8, 0.0),
          ('hen', (0.62, 0.32, 0.12), 0.8, 0.0), ('comb', (0.8, 0.08, 0.05), 0.6, 0.0), ('beak', (0.9, 0.6, 0.1), 0.6, 0.0)]:
    L.mat(*a)


def horse(c):
    return beast(c, 2.1, 0.62, 0.8, 1.0, 'horse_bay', 'horse_bay', 0.9, neck=0.55, legm='horse_dark',
                 extra=[cyl('tail', 0, -1.1, 1.0, 1.6, 0.08, 'horse_dark', 5, coll=c),
                        block('mane', -0.05, 0.05, 0.8, 1.25, 1.75, 2.15, 'horse_dark', coll=c)])


def cow(c):
    return beast(c, 2.0, 0.8, 0.95, 0.75, 'cow_white', 'cow_black', 0.75, neck=0.1, legm='cow_white',
                 extra=[ellip('patch', 0.25, 0.2, 1.35, 0.2, 0.5, 0.35, 'cow_black', c, 6),
                        ellip('patch', -0.3, -0.5, 1.2, 0.15, 0.35, 0.3, 'cow_black', c, 6)])


def sheep(c):
    return beast(c, 1.1, 0.6, 0.6, 0.4, 'wool', 'sheep_face', 0.42, legm='sheep_face')


def chicken(c):
    p = [ellip('body', 0, 0, 0.28, 0.13, 0.19, 0.15, 'hen', c, 6),
         ellip('head', 0, 0.16, 0.45, 0.07, 0.08, 0.08, 'hen', c, 6),
         block('comb', -0.015, 0.015, 0.13, 0.21, 0.5, 0.56, 'comb', coll=c),
         block('beak', -0.02, 0.02, 0.23, 0.28, 0.43, 0.46, 'beak', coll=c),
         block('tail', -0.04, 0.04, -0.24, -0.14, 0.3, 0.45, 'hen', coll=c)]
    for sx in (-0.05, 0.05):
        p.append(cyl('leg', sx, 0.0, 0.0, 0.15, 0.012, 'beak', 4, coll=c))
    return p


PROPS = {'tennis': tennis, 'basket': basket, 'pool': pool, 'football': football, 'stadium': stadium,
         'barn': barn, 'silo': silo, 'house': house, 'fence': fence_seg, 'hay': hay,
         'club': club, 'flag': flag, 'cart': cart, 'horse': horse, 'cow': cow, 'sheep': sheep, 'chicken': chicken}
report = {}
for name, fn in PROPS.items():
    c = bpy.data.collections.new(f'PROP-{name}')
    S.collection.children.link(c)
    o = L.join(fn(c), f'PROP-{name}')
    report[name] = sum(len(f.vertices) - 2 for f in o.data.polygons)
    for c2 in bpy.data.collections:
        c2.hide_render = c2.hide_viewport = c2 is not c
    bpy.ops.object.select_all(action='DESELECT')
    o.select_set(True)
    bpy.context.view_layer.objects.active = o
    bpy.ops.export_scene.gltf(filepath=os.path.join(OUT, f'PROP-{name}.glb'), export_format='GLB', use_selection=True,
                              export_yup=True, export_apply=True, export_image_format='NONE', export_animations=False,
                              export_extras=False)

# review: a contact sheet, each prop rendered on its own
S.render.engine = 'CYCLES'
S.cycles.samples = 32
w = bpy.data.worlds.new('neutral'); w.use_nodes = True
w.node_tree.nodes['Background'].inputs['Color'].default_value = (0.5, 0.55, 0.62, 1)
w.node_tree.nodes['Background'].inputs['Strength'].default_value = 0.8
S.world = w
sun = bpy.data.objects.new('key', bpy.data.lights.new('key', 'SUN'))
sun.data.energy = 3.5
sun.rotation_euler = (math.radians(50), 0, math.radians(-30))
S.collection.objects.link(sun)
cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam'))
S.collection.objects.link(cam)
S.camera = cam
cam.data.clip_end = 5000
S.render.resolution_x, S.render.resolution_y = 640, 420
VIEW = {'tennis': 48, 'basket': 44, 'pool': 85, 'football': 95, 'stadium': 330, 'barn': 32, 'silo': 30, 'house': 24,
        'fence': 12, 'hay': 5, 'club': 45, 'flag': 5, 'cart': 6, 'horse': 5, 'cow': 5, 'sheep': 3, 'chicken': 1}
for name in PROPS:
    for c2 in bpy.data.collections:
        c2.hide_render = c2.name != f'PROP-{name}'
    d = VIEW[name]
    cam.location = (d * 0.7, -d * 0.9, d * 0.55)
    cam.rotation_euler = (Vector((0, 0, d * 0.06)) - cam.location).to_track_quat('-Z', 'Y').to_euler()
    S.render.filepath = os.path.join(OUT, f'review_{name}.png')
    bpy.ops.render.render(write_still=True)
with open(os.path.join(OUT, 'PROPS.json'), 'w') as fh:
    json.dump({'triangles': report}, fh, indent=1)
print('PROPS', json.dumps(report))

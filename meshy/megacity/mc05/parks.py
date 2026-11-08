"""v0.3.6 parks and pools kit (Living Megacity): benches, bins, park lamps, a fountain, a playground (swings, slide,
climbing frame, sandpit border) and a pool kit (diving boards, water slide, loungers, umbrellas, lifeguard chair).
Low poly, origin = where it stands (the lot centre for the fountain, the playground's and the pool kit's own centre),
front = glTF +z (Blender -Y). 10 m = scale 1 in the game's Scatter (newcity.js buildProps).
  blender -b --factory-startup --python parks.py -- <out dir>
"""
import bpy, bmesh, sys, os, math, json
from mathutils import Vector, Matrix

OUT = sys.argv[sys.argv.index('--') + 1:][0]
os.makedirs(OUT, exist_ok=True)
MATS = {}

def mat(name, col, emit=0.0, rough=0.6, metal=0.0):
    if name in MATS: return MATS[name]
    m = bpy.data.materials.new(name); m.use_nodes = True
    b = m.node_tree.nodes['Principled BSDF']
    b.inputs['Base Color'].default_value = (*col, 1); b.inputs['Roughness'].default_value = rough; b.inputs['Metallic'].default_value = metal
    if emit:
        b.inputs['Emission Color'].default_value = (*col, 1); b.inputs['Emission Strength'].default_value = emit
    MATS[name] = m
    return m

def mats():
    return dict(
        wood=mat('park_wood', (0.36, 0.2, 0.09), rough=0.8), iron=mat('park_iron', (0.05, 0.06, 0.06), rough=0.5, metal=0.6),
        bin=mat('park_bin', (0.05, 0.16, 0.09), rough=0.6), glow=mat('park_glow', (1.0, 0.78, 0.5), emit=5.0),
        stone=mat('park_stone', (0.5, 0.48, 0.44), rough=0.85), water=mat('fountain_water', (0.25, 0.6, 0.85), emit=0.6, rough=0.05),
        jet=mat('fountain_jet', (0.75, 0.9, 1.0), emit=1.5, rough=0.1),
        red=mat('play_red', (0.75, 0.08, 0.06), rough=0.5), yellow=mat('play_yellow', (0.95, 0.7, 0.05), rough=0.5),
        blue=mat('play_blue', (0.06, 0.3, 0.75), rough=0.5), green=mat('play_green', (0.1, 0.55, 0.2), rough=0.5),
        steel=mat('play_steel', (0.6, 0.62, 0.64), rough=0.3, metal=0.8), rope=mat('play_rope', (0.2, 0.17, 0.13), rough=0.9),
        white=mat('pool_white', (0.9, 0.9, 0.88), rough=0.4), board=mat('pool_board', (0.1, 0.35, 0.75), rough=0.4),
        slide=mat('pool_slide', (0.95, 0.4, 0.05), rough=0.3), cloth=mat('pool_cloth', (0.9, 0.2, 0.25), rough=0.8),
        cloth2=mat('pool_cloth2', (0.95, 0.85, 0.2), rough=0.8), towel=mat('pool_towel', (0.2, 0.55, 0.8), rough=0.9),
    )

class B:
    def __init__(self):
        self.bm = bmesh.new(); self.m = []
    def mi(self, m):
        if m not in self.m: self.m.append(m)
        return self.m.index(m)
    def poly(self, vs, faces, m, T=Matrix()):
        v = [self.bm.verts.new(T @ Vector(p)) for p in vs]; k = self.mi(m)
        for f in faces:
            try: self.bm.faces.new([v[i] for i in f]).material_index = k
            except ValueError: pass
    def box(self, c, s, m, rz=0.0):
        a, b, h = s[0] / 2, s[1] / 2, s[2] / 2
        T = Matrix.Translation(c) @ Matrix.Rotation(rz, 4, 'Z')
        self.poly([(-a, -b, -h), (a, -b, -h), (a, b, -h), (-a, b, -h), (-a, -b, h), (a, -b, h), (a, b, h), (-a, b, h)],
                  [[0, 3, 2, 1], [4, 5, 6, 7], [0, 1, 5, 4], [1, 2, 6, 5], [2, 3, 7, 6], [3, 0, 4, 7]], m, T)
    def beam(self, p0, p1, t, m):
        """a square beam t thick from p0 to p1"""
        p0, p1 = Vector(p0), Vector(p1); d = p1 - p0
        R = d.normalized().to_track_quat('Z', 'Y').to_matrix().to_4x4()
        T = Matrix.Translation((p0 + p1) / 2) @ R
        a, h = t / 2, d.length / 2
        self.poly([(-a, -a, -h), (a, -a, -h), (a, a, -h), (-a, a, -h), (-a, -a, h), (a, -a, h), (a, a, h), (-a, a, h)],
                  [[0, 3, 2, 1], [4, 5, 6, 7], [0, 1, 5, 4], [1, 2, 6, 5], [2, 3, 7, 6], [3, 0, 4, 7]], m, T)
    def cyl(self, c, r, h, m, seg=10, r2=None, cap=True):
        r2 = r if r2 is None else r2; x, y, z = c
        v = [(x + math.cos(2 * math.pi * i / seg) * r, y + math.sin(2 * math.pi * i / seg) * r, z) for i in range(seg)]
        v += [(x + math.cos(2 * math.pi * i / seg) * r2, y + math.sin(2 * math.pi * i / seg) * r2, z + h) for i in range(seg)]
        f = [[i, (i + 1) % seg, seg + (i + 1) % seg, seg + i] for i in range(seg)]
        if cap: f += [list(range(seg))[::-1], list(range(seg, 2 * seg))]
        self.poly(v, f, m)
    def ring(self, c, r0, r1, h, m, seg=20):
        """a round wall: inner r0, outer r1, height h"""
        x, y, z = c; v = []
        for zz in (z, z + h):
            for r in (r0, r1):
                v += [(x + math.cos(2 * math.pi * i / seg) * r, y + math.sin(2 * math.pi * i / seg) * r, zz) for i in range(seg)]
        f = []
        for i in range(seg):
            j = (i + 1) % seg
            f += [[seg + i, seg + j, 3 * seg + j, 3 * seg + i], [j, i, 2 * seg + i, 2 * seg + j],   # outer, inner
                  [2 * seg + i, 3 * seg + i, 3 * seg + j, 2 * seg + j], [i, j, seg + j, seg + i]]  # top, bottom
        self.poly(v, f, m)
    def done(self, name):
        me = bpy.data.meshes.new(name); self.bm.normal_update(); self.bm.to_mesh(me); self.bm.free()
        for m in self.m: me.materials.append(m)
        ob = bpy.data.objects.new(name, me); bpy.context.scene.collection.objects.link(ob)
        bpy.context.view_layer.objects.active = ob; ob.select_set(True)
        bpy.ops.object.mode_set(mode='EDIT'); bpy.ops.mesh.select_all(action='SELECT'); bpy.ops.mesh.normals_make_consistent(inside=False)
        bpy.ops.object.mode_set(mode='OBJECT')
        return ob

def bench(M, b=None, at=(0, 0, 0), rz=0.0):
    b = b or B(); T = Matrix.Translation(at) @ Matrix.Rotation(rz, 4, 'Z')
    def bx(c, s, m): b.box(T @ Vector(c), s, m, rz)
    for x in (-0.75, 0.75):
        bx((x, 0, 0.22), (0.06, 0.45, 0.44), M['iron']); bx((x, 0.2, 0.62), (0.06, 0.06, 0.5), M['iron'])
    for k in range(3): bx((0, -0.15 + k * 0.13, 0.46), (1.8, 0.11, 0.04), M['wood'])
    for k in range(2): bx((0, 0.23, 0.62 + k * 0.17), (1.8, 0.04, 0.12), M['wood'])
    return b

def lamp(M, b=None, at=(0, 0, 0)):
    b = b or B(); x, y, z = at
    b.cyl((x, y, z), 0.14, 0.35, M['iron']); b.cyl((x, y, z + 0.35), 0.06, 3.4, M['iron'], seg=6)
    b.cyl((x, y, z + 3.75), 0.22, 0.38, M['glow'], seg=8, r2=0.26); b.cyl((x, y, z + 4.13), 0.3, 0.1, M['iron'], seg=8, r2=0.05)
    return b

def bin_(M, b=None, at=(0, 0, 0)):
    b = b or B(); x, y, z = at
    b.cyl((x, y, z), 0.28, 0.85, M['bin'], seg=8); b.cyl((x, y, z + 0.85), 0.3, 0.08, M['iron'], seg=8)
    return b

def fountain(M):
    b = B()
    b.ring((0, 0, 0), 4.0, 4.6, 0.65, M['stone'], seg=24)
    b.cyl((0, 0, 0.0), 4.0, 0.45, M['water'], seg=24)                     # the pool's water (its top 0.45 m up)
    b.cyl((0, 0, 0.45), 0.7, 1.2, M['stone'], seg=10, r2=0.5)            # the column
    b.cyl((0, 0, 1.65), 1.6, 0.25, M['stone'], seg=14, r2=1.8)           # the bowl
    b.cyl((0, 0, 1.9), 1.5, 0.05, M['water'], seg=14)
    b.cyl((0, 0, 1.9), 0.3, 1.0, M['stone'], seg=8, r2=0.2)
    b.cyl((0, 0, 2.9), 0.12, 1.6, M['jet'], seg=6, r2=0.03)               # the jet
    for i in range(8):                                                     # the bowl's spill: thin veils
        a = i * math.pi / 4
        b.box((math.cos(a) * 1.75, math.sin(a) * 1.75, 1.1), (0.05, 0.6, 1.1), M['jet'], a)
    for i in range(6):                                                     # benches round it
        a = i * math.pi / 3 + math.pi / 6
        bench(M, b, (math.cos(a) * 7.2, math.sin(a) * 7.2, 0), a - math.pi / 2)
    return b

def playground(M):
    b = B()
    # the sandpit border (the sand is painted on the ground, newcity.js park()): 6 x 5 m at (-4, 4)
    for (cx, cy, sx, sy) in [(0, 2.5, 6.2, 0.25), (0, -2.5, 6.2, 0.25), (3.0, 0, 0.25, 5.0), (-3.0, 0, 0.25, 5.0)]:
        b.box((cx - 4, cy + 4, 0.15), (sx, sy, 0.3), M['wood'])
    # swings: an A-frame each end, a top bar, two seats on ropes
    SY = 4.5                                                               # the swings: x 0.8 .. 6.2 at y 4.5
    for x in (0.8, 6.2):
        b.beam((x, SY - 0.9, 0), (x, SY, 2.6), 0.12, M['blue']); b.beam((x, SY + 0.9, 0), (x, SY, 2.6), 0.12, M['blue'])
    b.beam((0.8, SY, 2.6), (6.2, SY, 2.6), 0.12, M['red'])
    for x in (2.2, 4.8):
        for dx in (-0.25, 0.25): b.beam((x + dx, SY, 2.55), (x + dx, SY, 0.55), 0.03, M['rope'])
        b.box((x, SY, 0.52), (0.6, 0.25, 0.05), M['yellow'])
    # the slide: a tower with a roof, a ladder, the chute
    b.box((5.0, -4.0, 1.5), (1.4, 1.4, 0.1), M['wood'])
    for (x, y) in [(4.35, -4.65), (5.65, -4.65), (4.35, -3.35), (5.65, -3.35)]: b.beam((x, y, 0), (x, y, 2.8), 0.12, M['green'])
    b.poly([(4.2, -4.8, 2.8), (5.8, -4.8, 2.8), (5.8, -3.2, 2.8), (4.2, -3.2, 2.8), (5.0, -4.0, 3.6)],
           [[0, 1, 4], [1, 2, 4], [2, 3, 4], [3, 0, 4], [3, 2, 1, 0]], M['red'])
    for x in (4.45, 5.55): b.beam((x, -2.0, 0), (x, -3.3, 1.5), 0.08, M['steel'])
    for k in range(4): b.beam((4.45, -2.0 - k * 0.32, 0.35 + k * 0.37), (5.55, -2.0 - k * 0.32, 0.35 + k * 0.37), 0.05, M['steel'])
    b.poly([(4.5, -4.7, 1.55), (5.5, -4.7, 1.55), (5.5, -8.2, 0.25), (4.5, -8.2, 0.25),
            (4.5, -4.7, 1.75), (5.5, -4.7, 1.75), (5.5, -8.2, 0.45), (4.5, -8.2, 0.45)],
           [[0, 1, 2, 3], [4, 7, 6, 5], [0, 4, 5, 1], [1, 5, 6, 2], [2, 6, 7, 3], [3, 7, 4, 0]], M['yellow'])
    # a climbing frame (a dome of bars) and a see-saw
    for i in range(6):
        a = i * math.pi / 3
        b.beam((-4.5 + math.cos(a) * 1.8, -3.0 + math.sin(a) * 1.8, 0), (-4.5, -3.0, 2.2), 0.06, M['steel'])
    for h in (0.8, 1.5):
        r = 1.8 * (1 - h / 2.2)
        for i in range(6):
            a, a2 = i * math.pi / 3, (i + 1) * math.pi / 3
            b.beam((-4.5 + math.cos(a) * r, -3.0 + math.sin(a) * r, h), (-4.5 + math.cos(a2) * r, -3.0 + math.sin(a2) * r, h), 0.05, M['red'])
    b.box((0.5, -4.5, 0.25), (0.25, 0.25, 0.5), M['blue']); b.beam((0.5 - 1.8, -4.5, 0.25), (0.5 + 1.8, -4.5, 0.75), 0.12, M['yellow'])
    for x, y in [(-8.4, -0.5), (8.4, 1.0)]: bench(M, b, (x, y, 0), math.pi / 2 if x < 0 else -math.pi / 2)
    bin_(M, b, (-8.4, 1.4, 0))
    return b

def poolkit(M):
    """round the painted pools (newcity.js pool(): the big one 50 x 21 m at (0, -6) lot-local, z = Blender -y)"""
    b = B()
    # diving boards at the deep end (lot x = +25: Blender x 25)
    for y, h in [(-3.0, 1.0), (3.0, 3.0)]:
        yy = -(-6.0 + y)
        b.box((26.5, yy, h / 2), (1.0, 1.2, h), M['white'])
        b.box((24.6, yy, h + 0.08), (4.2, 0.55, 0.08), M['board'])
        b.beam((27.0, yy - 0.5, h), (27.0, yy - 0.5, h + 1.0), 0.05, M['steel']); b.beam((27.0, yy + 0.5, h), (27.0, yy + 0.5, h + 1.0), 0.05, M['steel'])
    # the water slide: a tower on the deck behind the kids' pool, a chute curling down into the big pool's shallow end
    tx, ty = -33.0, -(-22.0)
    for (x, y) in [(tx - 1, ty - 1), (tx + 1, ty - 1), (tx - 1, ty + 1), (tx + 1, ty + 1)]: b.beam((x, y, 0), (x, y, 6.0), 0.2, M['white'])
    b.box((tx, ty, 6.0), (2.6, 2.6, 0.2), M['white'])
    for k in range(10): b.box((tx + 1.4, ty - 1.0 + k * 0.0, 0.3 + k * 0.6), (0.6, 0.9, 0.08), M['steel'])
    pts = []
    for k in range(25):
        t = k / 24
        a = t * math.pi * 1.25
        pts.append(Vector((tx + math.sin(a) * 6.0 + t * 16.0, ty - (1 - math.cos(a)) * 5.0 - t * 4.0, 6.0 * (1 - t) + 0.4)))
    for p0, p1 in zip(pts, pts[1:]):
        d = (p1 - p0); side = Vector((-d.y, d.x, 0)).normalized() * 0.55
        b.poly([p0 - side, p0 + side, p1 + side, p1 - side, p0 - side + Vector((0, 0, 0.35)), p0 + side + Vector((0, 0, 0.35)),
                p1 + side + Vector((0, 0, 0.35)), p1 - side + Vector((0, 0, 0.35))],
               [[0, 1, 2, 3], [0, 4, 7, 3], [1, 2, 6, 5]], M['slide'])
        b.beam(p0 - Vector((0, 0, 0.2)), Vector((p0.x, p0.y, 0)), 0.1, M['white']) if pts.index(p0) % 6 == 3 else None
    # loungers in rows and umbrellas along the long sides
    for side in (-1,):                                                   # the far side only (the near deck has props.py loungers)
        yy = -(-6.0 + side * 14.0)
        for k in range(8):
            x = -21 + k * 6
            for dx in (-1.0, 1.0):
                b.box((x + dx, yy, 0.3), (0.7, 1.9, 0.12), M['white']); b.box((x + dx, yy - side * 0.95, 0.55), (0.7, 0.12, 0.5), M['white'])
                b.box((x + dx, yy, 0.38), (0.6, 1.5, 0.04), M['towel'])
            b.cyl((x, yy - side * 0.4, 0), 0.04, 2.3, M['steel'], seg=5)
            b.cyl((x, yy - side * 0.4, 2.05), 1.6, 0.4, M['cloth'] if k % 2 else M['cloth2'], seg=8, r2=0.05)
    # the lifeguard chair at the middle of the long side
    lx, ly = 0.0, -(-6.0 - 12.0)
    for (x, y) in [(lx - 0.6, ly - 0.6), (lx + 0.6, ly - 0.6), (lx - 0.6, ly + 0.6), (lx + 0.6, ly + 0.6)]: b.beam((x, y, 0), (lx + (x - lx) * 0.5, ly + (y - ly) * 0.5, 2.2), 0.1, M['white'])
    b.box((lx, ly, 2.25), (1.0, 0.8, 0.1), M['white']); b.box((lx, ly + 0.35, 2.7), (1.0, 0.1, 0.8), M['white'])
    b.cyl((lx, ly, 3.1), 0.03, 1.2, M['steel'], seg=4); b.cyl((lx, ly, 4.1), 1.0, 0.3, M['cloth'], seg=8, r2=0.05)
    return b

def export(name, build):
    bpy.ops.wm.read_factory_settings(use_empty=True); MATS.clear()
    M = mats(); ob = build(M).done(name)
    tris = sum(len(p.vertices) - 2 for p in ob.data.polygons)
    bpy.ops.export_scene.gltf(filepath=os.path.join(OUT, f'{name}.glb'), export_format='GLB', export_apply=True)
    return ob, tris

def review(ob, path, el=35):
    S = bpy.context.scene
    S.render.engine = 'BLENDER_EEVEE' if 'BLENDER_EEVEE' in [e.identifier for e in bpy.types.RenderSettings.bl_rna.properties['engine'].enum_items] else 'CYCLES'
    w = bpy.data.worlds.new('w'); w.use_nodes = True; w.node_tree.nodes['Background'].inputs['Color'].default_value = (0.5, 0.56, 0.62, 1); S.world = w
    sun = bpy.data.objects.new('sun', bpy.data.lights.new('sun', 'SUN')); sun.data.energy = 3; sun.rotation_euler = (0.8, 0, 0.5); S.collection.objects.link(sun)
    bpy.ops.mesh.primitive_plane_add(size=200, location=(0, 0, -0.01)); g = bpy.context.active_object
    gm = bpy.data.materials.new('g'); gm.diffuse_color = (0.12, 0.25, 0.1, 1); g.data.materials.append(gm)
    bb = [ob.matrix_world @ Vector(c) for c in ob.bound_box]
    lo = Vector(map(min, *[tuple(v) for v in bb])); hi = Vector(map(max, *[tuple(v) for v in bb])); ext = hi - lo
    cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam')); S.collection.objects.link(cam); S.camera = cam
    cam.data.type = 'ORTHO'; cam.data.ortho_scale = max(ext.x, ext.y, ext.z) * 1.3
    S.render.resolution_x, S.render.resolution_y = 700, 500
    d = Vector((0.8, -1.0, math.tan(math.radians(el)) * 1.3)).normalized()
    cam.location = (lo + hi) / 2 + d * 300; cam.rotation_euler = (-d).to_track_quat('-Z', 'Y').to_euler(); cam.data.clip_end = 1000
    S.render.filepath = path
    bpy.ops.render.render(write_still=True)

report = {}
for name, build in [('PROP-bench', lambda M: bench(M)), ('PROP-bin', lambda M: bin_(M)), ('PROP-parklamp', lambda M: lamp(M)),
                    ('PROP-fountain', fountain), ('PROP-playground', playground), ('PROP-poolkit', poolkit)]:
    ob, tris = export(name, build)
    report[name] = {'tris': tris, 'size_m': [round(v, 2) for v in ob.dimensions]}
    review(ob, os.path.join(OUT, f'review-{name}.png'))
json.dump(report, open(os.path.join(OUT, 'PARKS.json'), 'w'), indent=1)
print(json.dumps(report))

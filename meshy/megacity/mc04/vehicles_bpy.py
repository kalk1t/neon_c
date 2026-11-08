"""v0.3.6 city vehicles, built (the Meshy cars in mc04 decimate badly: their proportions are the reference):
sedan, hatch, van, taxi, bus; a hot-air balloon; a delivery drone and a camera drone.
Low poly, front = glTF +z (Blender -Y), standing on the origin. Materials by name (the game shares them):
  paint      white: each car's own colour comes from the instance colour (taxi_paint / bus_paint keep theirs)
  glass, trim, tyre, rim, head (emissive white), tail (emissive red), sign (emissive), led_r / led_g
  gore_a (white: each balloon's colour), gore_b, gore_c, basket, burner (emissive), rope
  blender -b --factory-startup --python vehicles_bpy.py -- <out dir>
"""
import bpy, bmesh, sys, os, math, json
from mathutils import Vector, Matrix

OUT = sys.argv[sys.argv.index('--') + 1:][0]
os.makedirs(OUT, exist_ok=True)
MATS = {}

def mat(name, col, emit=0.0, rough=0.5, metal=0.0, alpha=1.0):
    if name in MATS: return MATS[name]
    m = bpy.data.materials.new(name); m.use_nodes = True
    b = m.node_tree.nodes['Principled BSDF']
    b.inputs['Base Color'].default_value = (*col, 1); b.inputs['Roughness'].default_value = rough; b.inputs['Metallic'].default_value = metal
    if emit:
        b.inputs['Emission Color'].default_value = (*col, 1); b.inputs['Emission Strength'].default_value = emit
    if alpha < 1:
        b.inputs['Alpha'].default_value = alpha; m.blend_method = 'BLEND' if hasattr(m, 'blend_method') else None
    MATS[name] = m
    return m

def M():
    return {
        'paint': mat('paint', (0.9, 0.9, 0.9), rough=0.35, metal=0.3),
        'taxi_paint': mat('taxi_paint', (0.95, 0.68, 0.05), rough=0.35, metal=0.2),
        'bus_paint': mat('bus_paint', (0.92, 0.93, 0.95), rough=0.4),
        'bus_band': mat('bus_band', (0.05, 0.28, 0.7), rough=0.4),
        'glass': mat('glass', (0.03, 0.05, 0.08), rough=0.1, metal=0.6),
        'trim': mat('trim', (0.04, 0.04, 0.045), rough=0.6),
        'tyre': mat('tyre', (0.025, 0.025, 0.025), rough=0.9),
        'rim': mat('rim', (0.55, 0.56, 0.58), rough=0.3, metal=0.8),
        'head': mat('head', (1.0, 0.95, 0.85), emit=6.0),
        'tail': mat('tail', (1.0, 0.05, 0.03), emit=4.0),
        'sign': mat('sign', (1.0, 0.85, 0.3), emit=3.0),
        'led_r': mat('led_r', (1.0, 0.05, 0.05), emit=8.0),
        'led_g': mat('led_g', (0.1, 1.0, 0.2), emit=8.0),
        'gore_a': mat('gore_a', (0.92, 0.92, 0.92), rough=0.6),
        'gore_b': mat('gore_b', (0.95, 0.75, 0.1), rough=0.6),
        'gore_c': mat('gore_c', (0.1, 0.25, 0.65), rough=0.6),
        'basket': mat('basket', (0.42, 0.27, 0.12), rough=0.9),
        'burner': mat('burner', (1.0, 0.55, 0.15), emit=10.0),
        'rope': mat('rope', (0.25, 0.22, 0.18), rough=0.9),
        'drone': mat('drone', (0.88, 0.89, 0.9), rough=0.4),
        'drone_dark': mat('drone_dark', (0.06, 0.06, 0.07), rough=0.5),
        'parcel': mat('parcel', (0.55, 0.38, 0.2), rough=0.9),
        'rotor': mat('rotor', (0.1, 0.1, 0.1), rough=0.5),
    }

class Builder:
    def __init__(self):
        self.bm = bmesh.new(); self.mats = []
    def mi(self, m):
        if m not in self.mats: self.mats.append(m)
        return self.mats.index(m)
    def faces(self, verts, faces, m):
        vs = [self.bm.verts.new(v) for v in verts]
        k = self.mi(m)
        for f in faces:
            try:
                fc = self.bm.faces.new([vs[i] for i in f]); fc.material_index = k
            except ValueError: pass
    def prism(self, prof, w, m, y0=0.0):
        """a side profile [(s along the car, front +), z] extruded across the car (x = -w/2 .. w/2); front -> Blender -Y"""
        n = len(prof)
        verts = [(-w / 2, -s + y0, z) for s, z in prof] + [(w / 2, -s + y0, z) for s, z in prof]
        faces = [list(range(n))[::-1], list(range(n, 2 * n))]
        for i in range(n):
            j = (i + 1) % n
            faces.append([i, j, n + j, n + i])
        self.faces(verts, faces, m)
    def box(self, c, s, m):
        x, y, z = c; a, b, h = s[0] / 2, s[1] / 2, s[2] / 2
        v = [(x - a, y - b, z - h), (x + a, y - b, z - h), (x + a, y + b, z - h), (x - a, y + b, z - h),
             (x - a, y - b, z + h), (x + a, y - b, z + h), (x + a, y + b, z + h), (x - a, y + b, z + h)]
        self.faces(v, [[0, 3, 2, 1], [4, 5, 6, 7], [0, 1, 5, 4], [1, 2, 6, 5], [2, 3, 7, 6], [3, 0, 4, 7]], m)
    def cyl(self, c, r, l, m, axis='x', seg=8, cap=None):
        x, y, z = c; v = []
        for e in (-l / 2, l / 2):
            for i in range(seg):
                a = 2 * math.pi * i / seg; p, q = math.cos(a) * r, math.sin(a) * r
                v.append((x + e, y + p, z + q) if axis == 'x' else (x + p, y + q, z + e))
        f = [[i, (i + 1) % seg, seg + (i + 1) % seg, seg + i] for i in range(seg)]
        self.faces(v, f, m)
        self.faces(v, [list(range(seg))[::-1]], cap or m); self.faces(v, [list(range(seg, 2 * seg))], cap or m)
    def wheel(self, s, side, r, w, M):
        self.cyl((side, -s, r), r, w, M['tyre'], seg=10)
        self.cyl((side, -s, r), r * 0.58, w + 0.03, M['rim'], seg=8)
    def lathe(self, prof, seg, mats_of):
        """a solid of revolution round Blender Z: prof [(r, z)] bottom->top; mats_of(gore index) -> material"""
        rings = []
        for r, z in prof:
            rings.append([self.bm.verts.new((math.cos(2 * math.pi * i / seg) * r, math.sin(2 * math.pi * i / seg) * r, z)) for i in range(seg)])
        for k in range(len(rings) - 1):
            for i in range(seg):
                j = (i + 1) % seg
                f = self.bm.faces.new([rings[k][i], rings[k][j], rings[k + 1][j], rings[k + 1][i]])
                f.material_index = self.mi(mats_of(i))
    def done(self, name):
        me = bpy.data.meshes.new(name); self.bm.normal_update(); self.bm.to_mesh(me); self.bm.free()
        for m in self.mats: me.materials.append(m)
        ob = bpy.data.objects.new(name, me); bpy.context.scene.collection.objects.link(ob)
        bpy.context.view_layer.objects.active = ob
        ob.select_set(True)
        bpy.ops.object.mode_set(mode='EDIT'); bpy.ops.mesh.select_all(action='SELECT'); bpy.ops.mesh.normals_make_consistent(inside=False)
        bpy.ops.object.mode_set(mode='OBJECT')
        for p in me.polygons: p.use_smooth = False
        return ob

def car(kind, M):
    B = Builder()
    paint = M['taxi_paint'] if kind == 'taxi' else M['paint']
    if kind in ('sedan', 'taxi'):
        L, W, wb, r = 4.7, 1.84, 1.42, 0.33
        body = [(2.35, 0.32), (2.35, 0.72), (2.15, 0.86), (1.25, 0.98), (-1.9, 1.02), (-2.3, 0.92), (-2.35, 0.7), (-2.35, 0.32)]
        cab = [(1.3, 0.97), (0.45, 1.44), (-0.95, 1.46), (-1.95, 1.01)]
        roof = [(0.42, 1.43), (0.42, 1.49), (-0.92, 1.51), (-0.92, 1.45)]
    elif kind == 'hatch':
        L, W, wb, r = 4.0, 1.78, 1.25, 0.32
        body = [(2.0, 0.32), (2.0, 0.74), (1.75, 0.9), (1.1, 1.0), (-1.95, 1.04), (-2.0, 0.95), (-2.0, 0.32)]
        cab = [(1.15, 0.99), (0.3, 1.52), (-1.7, 1.54), (-1.93, 1.03)]
        roof = [(0.28, 1.51), (0.28, 1.58), (-1.72, 1.6), (-1.72, 1.53)]
    elif kind == 'van':
        L, W, wb, r = 5.2, 2.0, 1.75, 0.36
        body = [(2.6, 0.34), (2.6, 0.85), (2.35, 1.05), (1.6, 1.25), (0.85, 2.05), (-2.55, 2.1), (-2.6, 2.0), (-2.6, 0.34)]
        cab = None
        roof = None
    if kind == 'bus':
        L, W, r = 12.0, 2.55, 0.5
        B.prism([(6.0, 0.35), (6.0, 3.0), (5.85, 3.15), (-5.9, 3.15), (-6.0, 3.0), (-6.0, 0.35)], W, M['bus_paint'])
        B.prism([(6.02, 1.0), (6.02, 2.85), (5.6, 2.85), (5.6, 1.0)], W - 0.2, M['glass'])            # windscreen
        B.prism([(5.0, 1.35), (5.0, 2.65), (-5.6, 2.65), (-5.6, 1.35)], W + 0.03, M['glass'])         # side windows
        B.prism([(6.03, 0.5), (6.03, 0.95), (-6.03, 0.95), (-6.03, 0.5)], W + 0.04, M['bus_band'])   # a blue band
        B.box((0, 4.2, 1.55), (W + 0.05, 1.1, 2.3), M['glass'])                                     # the door
        for s in (4.0, -3.6):
            for x in (-W / 2 + 0.12, W / 2 - 0.12): B.wheel(s, x, r, 0.32, M)
        for x in (-0.95, 0.95):
            B.box((x, -6.02, 0.75), (0.35, 0.05, 0.22), M['head']); B.box((x, 6.02, 0.85), (0.3, 0.05, 0.35), M['tail'])
        B.box((0, -6.0, 2.98), (1.6, 0.05, 0.22), M['sign'])                                       # route sign
        return B
    B.prism(body, W, paint)
    if cab: B.prism(cab, W - 0.16, M['glass']); B.prism(roof, W - 0.14, paint)
    if kind == 'van':
        B.prism([(1.62, 1.28), (0.87, 2.03), (0.6, 2.03), (1.35, 1.28)], W - 0.2, M['glass'], 0.0)   # windscreen (proud)
        B.box((0, -0.25, 1.62), (W + 0.02, 0.9, 0.62), M['glass'])                                   # cab side windows
        B.box((0, 1.6, 1.62), (W + 0.02, 1.6, 0.55), M['glass'])
    B.prism([(L / 2 + 0.02, 0.32), (L / 2 + 0.02, 0.48), (-L / 2 - 0.02, 0.48), (-L / 2 - 0.02, 0.32)], W + 0.02, M['trim'])  # bumpers/sills
    for s in (wb, -wb):
        for x in (-W / 2 + 0.1, W / 2 - 0.1): B.wheel(s, x, r, 0.24, M)
    hz = 0.72 if kind != 'van' else 0.82
    for x in (-W / 2 + 0.3, W / 2 - 0.3):
        B.box((x, -L / 2 - 0.01, hz), (0.36, 0.04, 0.12), M['head'])
        B.box((x, L / 2 + 0.01, hz + 0.08), (0.34, 0.04, 0.12), M['tail'])
    if kind == 'taxi':
        B.box((0, 0.2, 1.6), (0.75, 0.3, 0.2), M['sign'])
    return B

def balloon(M):
    B = Builder()
    prof = [(0.4, 6.2), (2.2, 7.0), (4.6, 9.0), (6.6, 12.0), (7.7, 15.0), (8.0, 17.5), (7.4, 20.0), (5.6, 22.0), (3.0, 23.3), (0.01, 23.7)]
    B.lathe(prof, 16, lambda i: M['gore_a'] if i % 4 in (0, 1) else M['gore_b'] if i % 4 == 2 else M['gore_c'])
    B.box((0, 0, 0.55), (1.4, 1.4, 1.1), M['basket'])
    B.box((0, 0, 4.4), (0.35, 0.35, 0.35), M['burner'])
    for a in range(4):
        x, y = math.cos(a * math.pi / 2 + math.pi / 4) * 0.62, math.sin(a * math.pi / 2 + math.pi / 4) * 0.62
        top = Vector((x * 0.6 / 0.62 * 0.0 + math.cos(a * math.pi / 2 + math.pi / 4) * 0.4, math.sin(a * math.pi / 2 + math.pi / 4) * 0.4, 6.2))
        bot = Vector((x, y, 1.1)); mid = (top + bot) / 2; d = top - bot
        # a rope: a thin box along d
        b0 = Builder(); b0.box((0, 0, 0), (0.05, 0.05, d.length), M['rope'])
        me = bpy.data.meshes.new('r'); b0.bm.to_mesh(me); b0.bm.free()
        R = d.normalized().to_track_quat('Z', 'Y').to_matrix().to_4x4(); T = Matrix.Translation(mid) @ R
        for v in me.vertices:
            B.bm.verts.new(T @ v.co)
        B.bm.verts.ensure_lookup_table(); n = len(B.bm.verts)
        vs = B.bm.verts[n - 8:]
        k = B.mi(M['rope'])
        for f in [[0, 3, 2, 1], [4, 5, 6, 7], [0, 1, 5, 4], [1, 2, 6, 5], [2, 3, 7, 6], [3, 0, 4, 7]]:
            try: B.bm.faces.new([vs[i] for i in f]).material_index = k
            except ValueError: pass
        bpy.data.meshes.remove(me)
    return B

def drone(kind, M):
    B = Builder()
    B.box((0, 0, 0.5), (0.5, 0.7, 0.18), M['drone'])
    for a in range(4):
        ang = a * math.pi / 2 + math.pi / 4; x, y = math.cos(ang) * 0.75, math.sin(ang) * 0.75
        B.box((x / 2, y / 2, 0.52), (abs(x) + 0.06, abs(y) + 0.06, 0.06) if False else (0.08, 0.08, 0.06), M['drone'])
        # arm: a box from the body to the motor
        arm = Builder(); arm.box((0, 0, 0), (0.07, 0.07, 1.06), M['drone'])
        me = bpy.data.meshes.new('a'); arm.bm.to_mesh(me); arm.bm.free()
        T = Matrix.Translation((x / 2, y / 2, 0.52)) @ Vector((x, y, 0)).normalized().to_track_quat('Z', 'Y').to_matrix().to_4x4()
        for v in me.vertices: B.bm.verts.new(T @ v.co)
        B.bm.verts.ensure_lookup_table(); vs = B.bm.verts[len(B.bm.verts) - 8:]; k = B.mi(M['drone'])
        for f in [[0, 3, 2, 1], [4, 5, 6, 7], [0, 1, 5, 4], [1, 2, 6, 5], [2, 3, 7, 6], [3, 0, 4, 7]]:
            try: B.bm.faces.new([vs[i] for i in f]).material_index = k
            except ValueError: pass
        bpy.data.meshes.remove(me)
        B.cyl((x, y, 0.58), 0.08, 0.14, M['drone_dark'], axis='z')
        B.box((x, y, 0.67), (0.62, 0.05, 0.012), M['rotor'])                   # two-blade props
        B.box((x, y, 0.67), (0.05, 0.62, 0.012), M['rotor'])
        B.box((x, y, 0.47), (0.06, 0.06, 0.04), M['led_r'] if y < 0 else M['led_g'])
    for x in (-0.18, 0.18):
        B.box((x, 0, 0.22), (0.04, 0.5, 0.04), M['drone_dark']); B.box((x, 0.18, 0.33), (0.03, 0.03, 0.22), M['drone_dark']); B.box((x, -0.18, 0.33), (0.03, 0.03, 0.22), M['drone_dark'])
    if kind == 'delivery': B.box((0, 0, 0.27), (0.42, 0.42, 0.36), M['parcel'])
    else: B.cyl((0, -0.25, 0.33), 0.1, 0.12, M['drone_dark'], axis='x', seg=8); B.box((0, -0.33, 0.33), (0.08, 0.04, 0.08), M['glass'])
    return B

def export(name, build):
    bpy.ops.wm.read_factory_settings(use_empty=True); MATS.clear()
    Mx = M(); B = build(Mx); ob = B.done(name)
    tris = sum(len(p.vertices) - 2 for p in ob.data.polygons)
    bpy.ops.export_scene.gltf(filepath=os.path.join(OUT, f'{name}.glb'), export_format='GLB', export_apply=True)
    return ob, tris

def review(name, ob, path):
    S = bpy.context.scene
    S.render.engine = 'BLENDER_EEVEE' if 'BLENDER_EEVEE' in [e.identifier for e in bpy.types.RenderSettings.bl_rna.properties['engine'].enum_items] else 'CYCLES'
    w = bpy.data.worlds.new('w'); w.use_nodes = True; w.node_tree.nodes['Background'].inputs['Color'].default_value = (0.55, 0.6, 0.66, 1); S.world = w
    sun = bpy.data.objects.new('sun', bpy.data.lights.new('sun', 'SUN')); sun.data.energy = 3; sun.rotation_euler = (0.8, 0, 0.5); S.collection.objects.link(sun)
    bb = [ob.matrix_world @ Vector(c) for c in ob.bound_box]
    lo = Vector(map(min, *[tuple(v) for v in bb])); hi = Vector(map(max, *[tuple(v) for v in bb])); ext = hi - lo
    cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam')); S.collection.objects.link(cam); S.camera = cam
    cam.data.type = 'ORTHO'; cam.data.ortho_scale = max(ext) * 1.35
    S.render.resolution_x, S.render.resolution_y = 600, 450
    d = Vector((1.0, -1.3, 0.7)).normalized()
    cam.location = (lo + hi) / 2 + d * 100; cam.rotation_euler = (-d).to_track_quat('-Z', 'Y').to_euler(); cam.data.clip_end = 500
    S.render.filepath = path
    bpy.ops.render.render(write_still=True)

report = {}
items = [(f'VEH-{k}', (lambda k: lambda Mx: car(k, Mx))(k)) for k in ('sedan', 'hatch', 'van', 'taxi', 'bus')]
items += [('VEH-balloon', balloon), ('VEH-drone', lambda Mx: drone('delivery', Mx)), ('VEH-camdrone', lambda Mx: drone('camera', Mx))]
for name, build in items:
    ob, tris = export(name, build)
    report[name] = {'tris': tris, 'size_m': [round(v, 2) for v in ob.dimensions]}
    review(name, ob, os.path.join(OUT, f'review-{name}.png'))
json.dump(report, open(os.path.join(OUT, 'VEHICLES.json'), 'w'), indent=1)
print(json.dumps(report))

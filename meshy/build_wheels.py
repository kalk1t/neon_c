# v0.3.9: modelled wheels for the three shop cars, to replace the wheel cut out of each Meshy pod
# (game/tools/split-wheels.mjs puts them into the cars). Each wheel: a tyre with tread (lathed profile, tread blocks),
# a rim in the car's style, a hub and a neon accent ring. Axle along X, origin at the wheel's centre; the L wheel's outer
# face looks to -X (the car's left), the R wheel is its mirror. Two levels of detail (hi / lo).
# Usage: blender -b --python build_wheels.py -- [out dir]     (default meshy/wheels/)
#   writes <car>_wheels.glb with four objects: hi_L, hi_R, lo_L, lo_R (and <car>_preview.png)
import math, os, sys
import bpy, bmesh
from mathutils import Vector, Matrix

HERE = os.path.dirname(os.path.abspath(__file__))
argv = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else []
OUT = argv[0] if argv else os.path.join(HERE, 'wheels')
os.makedirs(OUT, exist_ok=True)

# R: tyre radius; W: tyre width; side: sidewall height (tyre profile); hub_w: how far the hub / axle reaches inward
# (hidden in the pod); rim: style; spokes; tread: style; accent: neon colour (linear RGB); metal: rim [albedo, rough]
CARS = {
    'roadster': dict(R=0.42, W=0.46, side=0.12, hub_w=0.38, rim='split', spokes=5, tread='sport',
                     accent=(1.0, 0.05, 0.75), metal=((0.32, 0.32, 0.35), 0.2)),
    'viper': dict(R=0.38, W=0.5, side=0.1, hub_w=0.3, rim='turbine', spokes=14, tread='slick',
                  accent=(1.0, 0.08, 0.6), metal=((0.62, 0.63, 0.66), 0.18)),
    'titan': dict(R=0.44, W=0.68, side=0.16, hub_w=0.06, rim='beadlock', spokes=6, tread='offroad',
                  accent=(0.05, 0.75, 1.0), metal=((0.2, 0.21, 0.23), 0.3)),
}


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


def lathe(bm, profile, seg, mat_index=0):
    """Revolves profile [(x, r)] around the X axis; returns the rings of verts (seg x len(profile))."""
    rings = []
    for i in range(seg):
        a = 2 * math.pi * i / seg
        rings.append([bm.verts.new((x, r * math.cos(a), r * math.sin(a))) for x, r in profile])
    for i in range(seg):
        A, B = rings[i], rings[(i + 1) % seg]
        for j in range(len(profile) - 1):
            f = bm.faces.new((A[j], A[j + 1], B[j + 1], B[j]))
            f.material_index = mat_index
            f.smooth = True
    return rings


def box(bm, center, size, rot_x, mat_index, smooth=False):
    """A box (size: x, y, z half extents) turned by rot_x about the X axis, centred at center (x, along-r, 0)."""
    res = bmesh.ops.create_cube(bm, size=1.0)
    vs = res['verts']
    M = Matrix.Rotation(rot_x, 4, 'X') @ Matrix.Translation(Vector(center)) @ Matrix.Diagonal((*[2 * s for s in size], 1))
    bmesh.ops.transform(bm, matrix=M, verts=vs)
    faces = {f for v in vs for f in v.link_faces}
    for f in faces:
        f.material_index = mat_index
        f.smooth = smooth
    return vs


def wheel(car, P, lod):
    hi = lod == 'hi'
    seg = 64 if hi else 24
    bm = bmesh.new()
    R, W, S = P['R'], P['W'], P['side']
    x0, x1 = -W / 2, W / 2  # tyre from x0 (outer, -X) to x1
    # ---- tyre (material 0): bead, sidewall bulge, shoulder, tread ----
    rb = R - S  # bead (where the rim holds it)
    bulge = 0.03 if P['tread'] == 'offroad' else 0.018
    prof = [(x1 - 0.01, rb), (x1 + bulge, rb + S * 0.45), (x1 + bulge * 0.4, R - 0.035), (x1 - 0.03, R - 0.004),
            (x1 - 0.06, R), (x0 + 0.06, R), (x0 + 0.03, R - 0.004), (x0 - bulge * 0.4, R - 0.035),
            (x0 - bulge, rb + S * 0.45), (x0 + 0.01, rb)]
    if hi:  # extra tread rows so blocks can be cut
        n = 8
        tread = [(x1 - 0.06 - (W - 0.12) * k / n, R) for k in range(n + 1)]
        prof = prof[:4] + tread + prof[6:]
    rings = lathe(bm, prof, seg, 0)
    if hi:
        # tread: grooves pressed into the running surface
        t0, t1 = 4, 4 + 8  # indices of the tread rows in the profile
        for i, ring in enumerate(rings):
            for j in range(t0, t1 + 1):
                k = (j - t0) / (t1 - t0)  # 0 inner .. 1 outer across the tread
                v = ring[j]
                if P['tread'] == 'sport':  # chevron grooves + a centre groove
                    groove = abs(k - 0.5) < 0.07 or ((i + int(abs(k - 0.5) * 8)) % 6 == 0 and abs(k - 0.5) > 0.12)
                    depth = 0.022
                elif P['tread'] == 'offroad':  # big alternating lugs
                    lug = (i // 2 + (0 if k < 0.5 else 1)) % 2 == 0
                    groove = not lug or abs(k - 0.5) < 0.05
                    depth = 0.035
                else:  # slick: two thin circumferential grooves
                    groove = abs(k - 0.3) < 0.05 or abs(k - 0.7) < 0.05
                    depth = 0.014
                if groove:
                    v.co.y *= (R - depth) / R
                    v.co.z *= (R - depth) / R
    # ---- rim barrel and face (material 1) ----
    rl = rb - 0.004  # rim lip radius
    face_x = x0 + 0.05  # the rim face sits a little inside the outer sidewall
    dish = 0.06 if P['rim'] != 'beadlock' else 0.02
    rim_prof = [(x1 - 0.02, rl), (x0 + 0.012, rl), (x0 + 0.004, rl + 0.012), (x0 - 0.004, rl - 0.01),
                (face_x - 0.01, rl - 0.03)]
    lathe(bm, rim_prof, seg, 1)
    # inner barrel back wall (so nothing is see-through)
    lathe(bm, [(x1 - 0.02, rl), (x1 - 0.02, 0.1)], seg, 3)  # dark, so the spokes stand out
    # hub
    hub_r = 0.11 if P['rim'] != 'beadlock' else 0.15
    hub = [(face_x + dish - 0.03, 0.0), (face_x + dish - 0.03, hub_r * 0.55), (face_x + dish - 0.045, hub_r * 0.8),
           (face_x + dish - 0.02, hub_r), (x1 + P['hub_w'], hub_r * 0.7), (x1 + P['hub_w'], 0.0)]
    lathe(bm, hub, max(16, seg // 2), 1)
    # accent ring (material 2): a thin glowing ring on the rim face, and the hub centre
    ring_r = rl - 0.035
    lathe(bm, [(face_x + 0.004, ring_r + 0.012), (face_x - 0.004, ring_r + 0.008), (face_x - 0.004, ring_r - 0.008),
               (face_x + 0.004, ring_r - 0.012)], seg, 2)
    lathe(bm, [(face_x + dish - 0.05, hub_r * 0.3), (face_x + dish - 0.056, hub_r * 0.38), (face_x + dish - 0.05, hub_r * 0.46)],
          max(12, seg // 3), 2)
    # spokes (material 1)
    n = P['spokes']
    inner, outer = hub_r * 0.85, rl - 0.025
    L = outer - inner
    for i in range(n):
        a = 2 * math.pi * i / n
        if P['rim'] == 'split':  # a twin spoke, the pair splaying toward the rim
            for off in (-0.055, 0.055):
                box(bm, (face_x + 0.012, inner + L / 2, off), (0.03, L / 2, 0.02), a + off * 0.6, 1)
        elif P['rim'] == 'turbine':  # many thin, twisted blades
            vs = box(bm, (face_x + 0.012, inner + L / 2, 0), (0.026, L / 2, 0.014), a, 1)
            for v in vs:  # twist along the blade
                r = math.hypot(v.co.y, v.co.z)
                tw = (r - inner) / L * 0.35
                c, s_ = math.cos(tw), math.sin(tw)
                v.co.y, v.co.z = v.co.y * c - v.co.z * s_, v.co.y * s_ + v.co.z * c
        else:  # beadlock: six thick spokes and a bolted outer ring
            box(bm, (face_x + 0.012, inner + L / 2, 0), (0.03, L / 2, 0.045), a, 1)
    if P['rim'] == 'beadlock':
        lathe(bm, [(x0 - 0.012, rl + 0.02), (x0 - 0.012, rl - 0.05), (x0 + 0.01, rl - 0.05)], seg, 1)
        bolts = 16 if hi else 8
        for i in range(bolts):
            box(bm, (x0 - 0.02, rl - 0.015, 0), (0.012, 0.012, 0.012), 2 * math.pi * (i + 0.5) / bolts, 1)
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-5)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    mesh = bpy.data.meshes.new(f'{car}_{lod}')
    bm.to_mesh(mesh)
    bm.free()
    for m in mats[car]:
        mesh.materials.append(m)
    return mesh


bpy.ops.wm.read_factory_settings(use_empty=True)
mats = {}
for car, P in CARS.items():
    mats[car] = [material(f'wheel_tyre', (0.025, 0.025, 0.028), 0.0, 0.82),
                 material(f'wheel_rim_{car}', P['metal'][0], 1.0, P['metal'][1]),
                 material(f'wheel_glow_{car}', (0.02, 0.02, 0.02), 0.0, 0.4, emit=P['accent']),
                 material('wheel_dark', (0.012, 0.012, 0.014), 0.3, 0.6)]

for car, P in CARS.items():
    for o in list(bpy.data.objects):
        bpy.data.objects.remove(o)
    for lod in ('hi', 'lo'):
        left = wheel(car, P, lod)
        ol = bpy.data.objects.new(f'{lod}_L', left)
        bpy.context.scene.collection.objects.link(ol)
        right = left.copy()
        right.name = f'{car}_{lod}_R'
        right.transform(Matrix.Diagonal((-1, 1, 1, 1)))
        right.flip_normals()
        orr = bpy.data.objects.new(f'{lod}_R', right)
        bpy.context.scene.collection.objects.link(orr)
        for o in (ol, orr):  # bevel-like smoothing: smooth by angle
            o.data.shade_smooth_by_angle(angle=math.radians(40)) if hasattr(o.data, 'shade_smooth_by_angle') else None
        print('WHEEL', car, lod, 'tris', sum(len(p.vertices) - 2 for p in left.polygons))
    path = os.path.join(OUT, f'{car}_wheels.glb')
    bpy.ops.export_scene.gltf(filepath=path, export_format='GLB', use_selection=False, export_yup=True)
    # preview: the hi L wheel, three-quarter from outside
    sc = bpy.context.scene
    sc.render.engine = 'BLENDER_EEVEE' if 'BLENDER_EEVEE' in [e.identifier for e in bpy.types.RenderSettings.bl_rna.properties['engine'].enum_items] else 'BLENDER_EEVEE_NEXT'
    sc.render.resolution_x = sc.render.resolution_y = 600
    for o in sc.objects:
        o.hide_render = o.name != 'hi_L'
    if not sc.world:
        sc.world = bpy.data.worlds.new('w')
    sc.world.use_nodes = True
    sc.world.node_tree.nodes['Background'].inputs['Color'].default_value = (0.35, 0.37, 0.42, 1)
    cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam'))
    sc.collection.objects.link(cam)
    sc.camera = cam
    d = Vector((-1.0, -0.55, 0.35)).normalized()
    cam.location = d * 2.4
    cam.rotation_euler = d.to_track_quat('Z', 'Y').to_euler()
    for i, (loc, e) in enumerate((((-2, -2, 3), 600), ((2, 2, 1), 200))):
        L = bpy.data.objects.new(f'L{i}', bpy.data.lights.new(f'L{i}', 'POINT'))
        L.data.energy = e
        L.location = loc
        sc.collection.objects.link(L)
    sc.render.filepath = os.path.join(OUT, f'{car}_preview.png')
    bpy.ops.render.render(write_still=True)
    print('WROTE', path)

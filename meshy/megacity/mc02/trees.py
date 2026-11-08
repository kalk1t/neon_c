"""Living Megacity: the city's trees (Blender 5.2, headless). Four species, each 10 m tall at scale 1 (the game scales
every copy to its own height), origin = foot of the trunk, Blender Z up:

  TREE-A  plane tree   broad crown of leaf clumps on a forked trunk (streets, plazas, parks)
  TREE-B  pine         tiers of ragged boughs (parks, golf club, the country)
  TREE-C  poplar       a tall narrow column (avenues, roofs, terraces)
  TREE-D  cherry       a low round crown in blossom (planters, courtyards, sky lobbies)

Each comes as _near (real branches and clumps, about 1-2.5k triangles) and _far (two or three blobs, about 100).
Materials: bark, leaf_a .. leaf_d (+ a lighter _lit shade per species for the sunny clumps), shipped as PBR factors.

  blender -b --factory-startup --python trees.py -- <out_dir>
"""
import bpy, bmesh, sys, os, json, math, random
from mathutils import Vector, Matrix, noise

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import lib_bld as L

OUT = os.path.abspath(sys.argv[sys.argv.index('--') + 1])
os.makedirs(OUT, exist_ok=True)
L.start()
S = L.S
L.common_mats()

L.mat('bark', (0.13, 0.1, 0.08), 0.9, 0.0)
L.mat('bark_pale', (0.42, 0.38, 0.32), 0.85, 0.0)
SPECIES_MATS = {
    'a': ((0.07, 0.19, 0.06), (0.16, 0.32, 0.08)),
    'b': ((0.03, 0.11, 0.07), (0.07, 0.19, 0.1)),
    'c': ((0.08, 0.2, 0.05), (0.18, 0.34, 0.07)),
    'd': ((0.62, 0.22, 0.36), (0.86, 0.48, 0.6)),
}
for k, (dark, lit) in SPECIES_MATS.items():
    L.mat(f'leaf_{k}', dark, 0.75, 0.0)
    L.mat(f'leaf_{k}_lit', lit, 0.7, 0.0)


def new_obj(name, bm, mats, coll):
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    for m in mats:
        me.materials.append(L.M[m])
    o = bpy.data.objects.new(name, me)
    coll.objects.link(o)
    return o


def limb(bm, p0, p1, r0, r1, seg=6, bend=0.0, rnd=None, mi=0, steps=3):
    """A tapered, slightly bent branch from p0 to p1 (radii r0 -> r1)."""
    p0, p1 = Vector(p0), Vector(p1)
    ax = (p1 - p0)
    up = Vector((0, 0, 1)) if abs(ax.normalized().z) < 0.95 else Vector((1, 0, 0))
    u = ax.cross(up).normalized()
    v = ax.cross(u).normalized()
    side = (u * (rnd.random() - 0.5) + v * (rnd.random() - 0.5)).normalized() * bend if rnd else Vector()
    rings = []
    for s in range(steps + 1):
        t = s / steps
        c = p0.lerp(p1, t) + side * math.sin(t * math.pi) * ax.length
        r = r0 + (r1 - r0) * t
        rings.append([bm.verts.new(c + (u * math.cos(a) + v * math.sin(a)) * r)
                      for a in [2 * math.pi * k / seg for k in range(seg)]])
    for a, b in zip(rings, rings[1:]):
        for k in range(seg):
            f = bm.faces.new((a[k], a[(k + 1) % seg], b[(k + 1) % seg], b[k]))
            f.material_index = mi
            f.smooth = True
    return p0.lerp(p1, 1.0) + side * 0.0


def clump(bm, c, rx, ry, rz, mi, rnd, sub=2, rough=0.28):
    """A leaf clump: an icosphere squashed to rx, ry, rz and roughened with noise (a ragged, leafy silhouette)."""
    g = bmesh.ops.create_icosphere(bm, subdivisions=sub, radius=1.0)
    seed = Vector((rnd.random() * 100, rnd.random() * 100, rnd.random() * 100))
    for vtx in g['verts']:
        n = vtx.co.normalized()
        k = 1.0 + rough * noise.noise(n * 2.3 + seed) + rough * 0.5 * noise.noise(n * 5.1 + seed)
        vtx.co = Vector((c[0] + n.x * rx * k, c[1] + n.y * ry * k, c[2] + n.z * rz * k))
    for f in {f for vtx in g['verts'] for f in vtx.link_faces}:
        f.material_index = mi
        f.smooth = True


MID = [False]                   # building the mid form: every clump at the lowest detail


def crown(bm, c, rx, ry, rz, n, size, rnd, lit_above=0.35, mats=(1, 2), rough=0.42):
    """A full crown: n small ragged clumps spread over an ellipsoid shell (plus a few inside to close it)."""
    for k in range(n):
        u, v = rnd.random() * 2 - 1, rnd.random() * 2 * math.pi
        sh = 0.55 if k < n * 0.15 else 0.82 + rnd.random() * 0.2      # a few deep ones fill the middle
        d = Vector((math.sqrt(1 - u * u) * math.cos(v), math.sqrt(1 - u * u) * math.sin(v), u))
        p = (c[0] + d.x * rx * sh, c[1] + d.y * ry * sh, c[2] + d.z * rz * sh)
        s = size * (0.75 + rnd.random() * 0.5)
        lit = d.z > lit_above or (d.x + d.y) > 0.9 or rnd.random() < 0.15
        clump(bm, p, s, s, s * 0.8, mats[0] if lit else mats[1], rnd, 1 if (s < 0.8 or MID[0]) else 2, rough)


def cone_tier(bm, z0, z1, r, mi, rnd, n=11):
    """A pine tier: a ragged star of boughs hanging down from z1 to z0."""
    top = bm.verts.new((0, 0, z1))
    ring = []
    for k in range(n * 2):
        a = 2 * math.pi * k / (n * 2) + rnd.random() * 0.1
        rr = r * (1.0 if k % 2 == 0 else 0.62) * (0.85 + rnd.random() * 0.3)
        ring.append(bm.verts.new((math.cos(a) * rr, math.sin(a) * rr, z0 + (0 if k % 2 == 0 else (z1 - z0) * 0.18) + rnd.random() * 0.15)))
    inner = [bm.verts.new((math.cos(2 * math.pi * k / n) * r * 0.25, math.sin(2 * math.pi * k / n) * r * 0.25, z0 + (z1 - z0) * 0.3)) for k in range(n)]
    for k in range(n * 2):
        f = bm.faces.new((top, ring[k], ring[(k + 1) % (n * 2)]))
        f.material_index = mi
        f.smooth = True
    for k in range(n):                       # underside: so a tier seen from below is closed
        a, b, c = ring[2 * k], ring[(2 * k + 2) % (n * 2)], inner[k]
        f = bm.faces.new((b, a, c)); f.material_index = mi
        f = bm.faces.new((inner[(k + 1) % n], c, b) if False else (c, inner[(k + 1) % n], b)); f.material_index = mi


def plane_tree(lod, rnd):
    bm = bmesh.new()
    if lod == 'near':
        limb(bm, (0, 0, -0.2), (0.15, 0.1, 3.6), 0.34, 0.22, 9, 0.04, rnd, 0, 4)
        tips = []
        for k in range(5):
            a = k * 2 * math.pi / 5 + rnd.random() * 0.5
            tip = Vector((math.cos(a) * 2.6, math.sin(a) * 2.6, 6.2 + rnd.random() * 1.6))
            limb(bm, (0.15, 0.1, 3.4), tip, 0.2, 0.07, 6, 0.08, rnd, 0, 3)
            tips.append(tip)
            for q in range(2):               # twigs into the clumps
                t2 = tip + Vector((rnd.random() - 0.5, rnd.random() - 0.5, 0.4 + rnd.random() * 0.6)) * 2.0
                limb(bm, tip, t2, 0.07, 0.03, 4, 0.0, None, 0, 1)
        crown(bm, (0.1, 0.1, 7.4), 3.9, 3.7, 2.5, 46, 1.05, rnd)
        for t in tips[:3]:                   # lobes: an uneven, real-tree outline
            crown(bm, (t.x * 1.15, t.y * 1.15, t.z + 0.3), 1.6, 1.6, 1.2, 8, 0.8, rnd)
        return bm, ['bark', 'leaf_a_lit', 'leaf_a']
    limb(bm, (0, 0, -0.2), (0, 0, 4.5), 0.34, 0.2, 5, 0, None, 0, 1)
    clump(bm, (0, 0, 7.3), 3.9, 3.9, 2.8, 1, rnd, 1, 0.15)
    return bm, ['bark', 'leaf_a']


def pine(lod, rnd):
    bm = bmesh.new()
    if lod == 'near':
        limb(bm, (0, 0, -0.2), (0, 0, 9.6), 0.28, 0.05, 8, 0.01, rnd, 0, 4)
        tiers = 9
        for k in range(tiers):
            z0 = 2.0 + k * 0.82
            r = 2.9 * (1 - k / (tiers + 0.6)) + 0.35
            cone_tier(bm, z0, z0 + 1.6, r, 1 if k % 2 else 2, rnd, 14)
        cone_tier(bm, 9.2, 10.0, 0.5, 1, rnd, 7)
        return bm, ['bark', 'leaf_b_lit', 'leaf_b']
    limb(bm, (0, 0, -0.2), (0, 0, 2.6), 0.28, 0.2, 5, 0, None, 0, 1)
    g = bmesh.ops.create_cone(bm, cap_ends=True, segments=8, radius1=3.0, radius2=0.0, depth=8.0)
    bmesh.ops.translate(bm, verts=g['verts'], vec=(0, 0, 6.0))
    for f in {f for vtx in g['verts'] for f in vtx.link_faces}:
        f.material_index = 1
    return bm, ['bark', 'leaf_b']


def poplar(lod, rnd):
    bm = bmesh.new()
    if lod == 'near':
        limb(bm, (0, 0, -0.2), (0, 0, 8.6), 0.22, 0.06, 7, 0.0, None, 0, 3)
        for k in range(4):
            a = k * math.pi / 2 + 0.4
            limb(bm, (0, 0, 2.2 + k * 0.9), (math.cos(a) * 0.6, math.sin(a) * 0.6, 7.5 + k * 0.4), 0.08, 0.03, 4, 0.0, None, 0, 1)
        crown(bm, (0, 0, 6.1), 1.3, 1.2, 3.7, 72, 0.66, rnd, 0.5)
        crown(bm, (0.1, 0, 9.4), 0.45, 0.45, 0.7, 5, 0.4, rnd)
        return bm, ['bark_pale', 'leaf_c_lit', 'leaf_c']
    limb(bm, (0, 0, -0.2), (0, 0, 2.5), 0.22, 0.15, 5, 0, None, 0, 1)
    clump(bm, (0, 0, 6.2), 1.45, 1.45, 3.9, 1, rnd, 1, 0.1)
    return bm, ['bark_pale', 'leaf_c']


def cherry(lod, rnd):
    bm = bmesh.new()
    if lod == 'near':
        limb(bm, (0, 0, -0.2), (0.3, 0, 2.8), 0.3, 0.2, 8, 0.06, rnd, 0, 3)
        tips = []
        for k in range(6):
            a = k * math.pi / 3 + rnd.random() * 0.4
            tip = Vector((0.3 + math.cos(a) * 3.0, math.sin(a) * 3.0, 4.6 + rnd.random() * 1.4))
            limb(bm, (0.3, 0, 2.6), tip, 0.16, 0.05, 5, 0.12, rnd, 0, 3)
            tips.append(tip)
        crown(bm, (0.3, 0, 6.2), 4.1, 3.9, 2.1, 52, 0.95, rnd, 0.3, rough=0.5)
        for t in tips[::2]:
            crown(bm, (t.x * 1.1, t.y * 1.1, t.z + 0.2), 1.3, 1.3, 0.9, 7, 0.7, rnd, rough=0.5)
        return bm, ['bark', 'leaf_d_lit', 'leaf_d']
    limb(bm, (0, 0, -0.2), (0.3, 0, 3.4), 0.3, 0.18, 5, 0, None, 0, 1)
    clump(bm, (0.3, 0, 6.2), 4.0, 4.0, 2.4, 1, rnd, 1, 0.18)
    return bm, ['bark', 'leaf_d']


def mid_of(fn, n_scale=0.22):
    def f(lod, rnd):
        if lod != 'mid':
            return fn(lod, rnd)
        MID[0] = True
        old = crown.__defaults__
        try:
            g = globals()['crown']
            def thin(bm, c, rx, ry, rz, n, size, rnd, *a, **k):
                return g(bm, c, rx, ry, rz, max(3, int(n * n_scale)), size * 1.7, rnd, *a, **k)
            globals()['crown'] = thin
            return fn('near', rnd)
        finally:
            globals()['crown'] = g
            MID[0] = False
    return f


SPECIES = [('TREE-A', mid_of(plane_tree)), ('TREE-B', mid_of(pine)), ('TREE-C', mid_of(poplar)), ('TREE-D', mid_of(cherry))]
report = {}
for i, (name, fn) in enumerate(SPECIES):
    report[name] = {}
    objs = {}
    for lod in ('near', 'mid', 'far'):
        c = bpy.data.collections.new(f'{name}_{lod}')
        S.collection.children.link(c)
        bm, mats = fn(lod, random.Random(7 + i))
        bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
        o = new_obj(f'{name}_{lod}', bm, mats, c)
        objs[lod] = (c, o)
        report[name][lod] = sum(len(p.vertices) - 2 for p in o.data.polygons)
    for lod, (c, o) in objs.items():
        bpy.ops.object.select_all(action='DESELECT')
        o.select_set(True)
        bpy.ops.export_scene.gltf(filepath=os.path.join(OUT, f'{name}_{lod}.glb'), export_format='GLB', use_selection=True,
                                  export_yup=True, export_apply=True, export_image_format='NONE', export_animations=False,
                                  export_extras=False)

# review: the four species in a row, near forms, neutral daylight
for c in bpy.data.collections:
    c.hide_render = not c.name.endswith('_near')
for i, (name, _) in enumerate(SPECIES):
    bpy.data.objects[f'{name}_near'].location.x = (i - 1.5) * 9.0
S.render.engine = 'CYCLES'
S.cycles.samples = 48
w = bpy.data.worlds.new('neutral'); w.use_nodes = True
w.node_tree.nodes['Background'].inputs['Color'].default_value = (0.5, 0.55, 0.62, 1)
w.node_tree.nodes['Background'].inputs['Strength'].default_value = 0.8
S.world = w
sun = bpy.data.objects.new('key', bpy.data.lights.new('key', 'SUN'))
sun.data.energy = 3.5
sun.rotation_euler = (math.radians(50), 0, math.radians(-30))
S.collection.objects.link(sun)
L.block('ground', -30, 30, -20, 20, -0.1, 0.0, 'soil', coll=S.collection)
cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam'))
S.collection.objects.link(cam)
cam.location = (0, -34, 7)
cam.rotation_euler = (Vector((0, 0, 5)) - cam.location).to_track_quat('-Z', 'Y').to_euler()
cam.data.lens = 35
S.camera = cam
S.render.resolution_x, S.render.resolution_y = 1400, 700
S.render.filepath = os.path.join(OUT, 'TREES_review.png')
bpy.ops.render.render(write_still=True)
with open(os.path.join(OUT, 'TREES.json'), 'w') as fh:
    json.dump({'height_m': 10.0, 'triangles': report}, fh, indent=1)
print('TREES', json.dumps(report))

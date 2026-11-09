# v0.2.27: rebuilds a production car (public/models/car*.glb) from its raw Meshy source with the SAME transform, parts,
# pivots and triangle budget, but without the defects measured in the v0.2.26 build (build_game_car.py):
#   1. shading: the decimated mesh gets its normals transferred from the dense source (custom split normals) instead of
#      normals recomputed from the decimated triangles with a 35 degree auto-smooth (crumpled panels, broken seams);
#      optionally (--bake) a tangent-space normal map baked from the dense source onto the game mesh, so detail the
#      decimation removed (door seams, vents) comes back as normal-map detail;
#   2. tangents are exported (MikkTSpace, as Blender computes them), instead of the browser guessing them per pixel;
#   3. material regions are authored into the texture set instead of guessed at run time:
#        emissive texture = the light lines only (base colour x a light mask), so the game no longer decides that every
#          saturated texel is a lamp; the mask is derived ONCE from the v0.2.26 rule (same lights as before), then cleaned
#          of single-texel speckle, and is now an ordinary editable image;
#        metal/roughness texture (glTF: G roughness, B metalness) + R = clear-coat weight (KHR_materials_clearcoat reads
#          R): paint = glossy coated paint, trim = metal, recesses = rough and dark, lights = no metal;
#      the Meshy maps were uniform (metalness 0 everywhere on ROADSTER/TITAN, 1 everywhere on VIPER).
# The pods are split off by the production file's own pod boxes (stable metadata) and keep the production pivots.
#
# Usage: blender -b --python refine_car.py -- <car id> [--bake] [--out <file.glb>]
#   car id: roadster | viper | titan  (sources and per-car material values: CARS below)
# Writes meshy/refined/<file>.glb, meshy/refined/<car>.blend, meshy/refined/<car>_report.json, and the authored maps
# (meshy/refined/<car>_*.png).
import json, math, os, sys, time
import bpy, bmesh
import numpy as np
from mathutils import Vector, Matrix

HERE = os.path.dirname(os.path.abspath(__file__))
GAME = os.path.normpath(os.path.join(HERE, '..', 'game'))
argv = sys.argv[sys.argv.index('--') + 1:]
CAR = argv[0]
BAKE = '--bake' in argv
OUT_DIR = os.path.join(HERE, 'refined')
os.makedirs(OUT_DIR, exist_ok=True)

# Per car: raw Meshy source (identified in the v0.2.27 audit by comparing textures: mean difference < 1/255), production
# file, and the material response of each region. paint: [metalness, roughness, coat]; VIPER keeps a metallic paint
# (its Meshy file was metal everywhere), the others a black lacquer.
CARS = {
    'roadster': dict(raw='car_neon_raw.glb', prod='car.glb', paint=(0.35, 0.34, 1.0), trim=(1.0, 0.22, 0.0),
                     pod=(0.45, 0.28)),  # pod: [linear albedo, roughness] (concept: chrome pods)
    'viper': dict(raw='car02/raw/model.glb', prod='car02.glb', paint=(0.7, 0.3, 1.0), trim=(1.0, 0.2, 0.0)),
    'titan': dict(raw='car03/retex2/model.glb', prod='car03.glb', paint=(0.35, 0.36, 1.0), trim=(1.0, 0.24, 0.0),
                  pod=(0.3, 0.3)),  # concept: dark gunmetal pods (VIPER's pods are already silver in its texture)
    # v0.3.9 ROADSTER pilot: a new body (meshy/roadster_v4: concept views from renders of the old car, then Meshy
    # multi-image-to-3D). No pods of its own: its sculpted wheels are cut away (wheels: per corner, the cylinder
    # [centre y, z (Blender, after normalising), radius, inner |x|, outer |x|], measured by roadster_v4/measure.py) and
    # each pod is an empty pivot on the axle, where tools/split-wheels.mjs hangs a modelled wheel. Its light lines came
    # out pale grey: `lines` = the colour given to thin bright lines (brighter than their 9x9 surroundings).
    # body: triangle budget per level of detail (the old body + pods budget, less the four modelled wheels)
    'roadster4': dict(raw='roadster_v4/model.glb', prod='car.glb', paint=(0.35, 0.34, 1.0), trim=(1.0, 0.22, 0.0),
                      wheels=dict(F=(1.522, -0.433, 0.384, 1.08, 1.645), R=(-1.72, -0.43, 0.41, 0.98, 1.64)),
                      prod_path='refined/v038/car.glb', wheel_w=dict(F=0.52, R=0.6), lines=(1.0, 0.1, 0.8), body=dict(hi=100000, lo=30000)),
}
C = CARS[CAR]
OUT = argv[argv.index('--out') + 1] if '--out' in argv else os.path.join(OUT_DIR, C['prod'])
TEX = 2048
LENGTH = 5.6
report = {'car': CAR, 'raw': C['raw'], 'prod': C['prod'], 'bake': BAKE, 'started': time.strftime('%Y-%m-%d %H:%M:%S')}
t0 = time.time()


def log(*a):
    print('[refine]', *a, flush=True)


def tris(me):
    return sum(len(p.vertices) - 2 for p in me.polygons)


# ---------------- 1. the production file: parts, pivots, triangle budget ----------------
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=os.path.join(HERE, C['prod_path']) if C.get('prod_path') else os.path.join(GAME, 'public', 'models', C['prod']))  # v0.3.4+: the game files are KTX2 (not importable): prod_path = an uncompressed master
prod = {}
for o in bpy.context.scene.objects:
    if o.type != 'MESH':
        continue
    o.data.transform(Matrix.Identity(4))
    w = [o.matrix_world @ v.co for v in o.data.vertices]
    lo = Vector(map(min, zip(*w)))
    hi = Vector(map(max, zip(*w)))
    prod[o.name] = dict(loc=tuple(o.matrix_world.translation), min=tuple(lo), max=tuple(hi), tris=tris(o.data))
report['prod_parts'] = prod
log('production parts', {k: v['tris'] for k, v in prod.items()})
for o in list(bpy.data.objects):
    bpy.data.objects.remove(o)
for m in list(bpy.data.meshes):
    bpy.data.meshes.remove(m)

# ---------------- 2. the raw source, turned and scaled exactly like build_game_car.py ----------------
bpy.ops.import_scene.gltf(filepath=os.path.join(HERE, C['raw']))
meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
bpy.context.view_layer.objects.active = meshes[0]
for o in meshes:
    o.select_set(True)
if len(meshes) > 1:
    bpy.ops.object.join()
src = bpy.context.view_layer.objects.active
for o in list(bpy.context.scene.objects):
    if o is not src:
        bpy.data.objects.remove(o)
src.data.transform(src.matrix_world)
src.matrix_world = Matrix.Identity(4)
src.data.transform(Matrix.Rotation(math.radians(-90), 4, 'Z'))
co = np.empty(len(src.data.vertices) * 3)
src.data.vertices.foreach_get('co', co)
co = co.reshape(-1, 3)
lo, hi = co.min(0), co.max(0)
if hi[1] - lo[1] < hi[0] - lo[0]:
    src.data.transform(Matrix.Rotation(math.radians(90), 4, 'Z'))
    src.data.vertices.foreach_get('co', co.reshape(-1))
    lo, hi = co.min(0), co.max(0)
s = LENGTH / (hi[1] - lo[1])
src.data.transform(Matrix.Scale(s, 4) @ Matrix.Translation(-Vector((lo + hi) / 2)))
src.name = 'source'
log('source', len(src.data.polygons), 'faces, size', ((hi - lo) * s).round(3).tolist())
report['source_tris'] = tris(src.data)
# glTF -> Blender: glTF (x, y, z) = Blender (x, -z... ) handled by the importer; production values above are Blender space

# v0.3.9 (roadster4): the sculpted wheels go (faces whose centre lies in a wheel's cylinder, 2 cm grown); the pods
# become empty pivots on the axles, the tyre's outer face where the sculpted one was
POD_PIVOTS = {}
if C.get('wheels'):
    bm = bmesh.new(); bm.from_mesh(src.data)
    drop = []
    for f in bm.faces:
        c = f.calc_center_median()
        for fr, (cy, cz, r, xin, xout) in C['wheels'].items():
            if xin - 0.02 <= abs(c.x) <= xout + 0.03 and math.hypot(c.y - cy, c.z - cz) < r + 0.02:
                drop.append(f); break
    bmesh.ops.delete(bm, geom=drop, context='FACES')
    bmesh.ops.delete(bm, geom=[v for v in bm.verts if not v.link_faces], context='VERTS')
    bm.to_mesh(src.data); bm.free()
    log('wheels cut', len(drop), 'faces')
    report['wheels_cut'] = len(drop)
    for fr, (cy, cz, r, xin, xout) in C['wheels'].items():
        for lr, sx in (('L', -1), ('R', 1)):
            POD_PIVOTS[fr + lr] = Vector((sx * (xout - C['wheel_w'][fr] / 2), cy, cz))

# ---------------- 3. split the source into body + four pods by the production pod boxes ----------------
def part_boxes(lod):
    """Pod boxes for one level of detail: the UNION of the hi and lo production pod boxes (the same physical pod; the
    decimated lo pod is a little smaller, and its box alone left pod fragments in lo_body), pivot from this level."""
    out = {}
    if POD_PIVOTS:
        return {}
    m = Vector((0.005, 0.005, 0.005))
    for key in ('FL', 'FR', 'RL', 'RR'):
        p = prod.get(f'{lod}_pod_{key}')
        if p:
            both = [q for q in (prod.get(f'hi_pod_{key}'), prod.get(f'lo_pod_{key}')) if q]
            lo = Vector([min(q['min'][i] for q in both) for i in range(3)])
            hi = Vector([max(q['max'][i] for q in both) for i in range(3)])
            out[key] = (lo - m, hi + m, Vector(p['loc']))
    return out


def split_source(boxes):
    """Copies of the source: body and one object per pod (faces whose centre lies in that pod's production box)."""
    parts = {}
    bm_all = bmesh.new()
    bm_all.from_mesh(src.data)
    owner = {}
    for f in bm_all.faces:
        c = f.calc_center_median()
        for key, (a, b, _) in boxes.items():
            if a.x <= c.x <= b.x and a.y <= c.y <= b.y and a.z <= c.z <= b.z:
                owner[f.index] = key
                break
    bm_all.free()
    for key in ['body', *boxes.keys()]:
        bm = bmesh.new()
        bm.from_mesh(src.data)
        drop = [f for f in bm.faces if owner.get(f.index, 'body') != key]
        bmesh.ops.delete(bm, geom=drop, context='FACES')
        bmesh.ops.delete(bm, geom=[v for v in bm.verts if not v.link_faces], context='VERTS')
        me = bpy.data.meshes.new(f'src_{key}')
        bm.to_mesh(me)
        bm.free()
        o = bpy.data.objects.new(f'src_{key}', me)
        bpy.context.scene.collection.objects.link(o)
        parts[key] = o
    return parts


# ---------------- 4. textures: the authored maps (numpy) ----------------
def image_pixels(img):
    a = np.empty(img.size[0] * img.size[1] * 4, dtype=np.float32)
    img.pixels.foreach_get(a)
    return a.reshape(img.size[1], img.size[0], 4)


def resized(img, px):
    c = img.copy()
    c.scale(px, px)
    return c


src_mat = src.data.materials[0]
nodes = src_mat.node_tree.nodes
tex_nodes = [n for n in nodes if n.type == 'TEX_IMAGE']
def linked_image(socket_name):
    bsdf = next(n for n in nodes if n.type == 'BSDF_PRINCIPLED')
    stack = [bsdf.inputs[socket_name]]
    while stack:
        s = stack.pop()
        for l in s.links:
            n = l.from_node
            if n.type == 'TEX_IMAGE':
                return n.image
            stack.extend(i for i in n.inputs if i.is_linked)
    return None
# v0.3 (A2): --tex <dir> = a Meshy retexture of this same raw model with its original UVs kept (enable_original_uv):
# its texture_0.png (paint, baked lighting removed) and texture_0_normal.png replace the raw file's, geometry unchanged
TEXDIR = os.path.join(HERE, argv[argv.index('--tex') + 1]) if '--tex' in argv else None
if TEXDIR:
    for fname, sock in (('texture_0.png', 'Base Color'), ('texture_0_normal.png', 'Normal')):
        old = linked_image(sock)
        new = bpy.data.images.load(os.path.join(TEXDIR, fname))
        new.colorspace_settings.name = old.colorspace_settings.name
        for n in nodes:
            if n.type == 'TEX_IMAGE' and n.image == old:
                n.image = new
    report['tex'] = TEXDIR
    log('retextured source', TEXDIR)
base_img = linked_image('Base Color')
normal_img = linked_image('Normal')
log('source images', base_img and base_img.size[:], normal_img and normal_img.size[:])

base_small = resized(base_img, TEX)
base_small.name = f'{CAR}_base'
B = image_pixels(base_small)  # Blender stores sRGB images' pixels as the file's values (not linearised) in .pixels
srgb = B[..., :3]
lin = np.where(srgb <= 0.04045, srgb / 12.92, ((srgb + 0.055) / 1.055) ** 2.4)
tmax = lin.max(-1)
tmin = lin.min(-1)
sat = np.where(tmax > 0, (tmax - tmin) / np.maximum(tmax, 1e-6), 0)
def smoothstep(a, b, x):
    t = np.clip((x - a) / (b - a), 0, 1)
    return t * t * (3 - 2 * t)
# the v0.2.26 rule (car.js NEON_MASK), evaluated once on the same linear values the shader saw
neon = smoothstep(0.45, 0.8, sat) * smoothstep(0.12, 0.4, tmax)
# cleaning: drop isolated specks (fewer than 3 lit texels in the 3x3 around it): compression noise, not a light line
lit = (neon > 0.3).astype(np.float32)
nb = sum(np.roll(np.roll(lit, dy, 0), dx, 1) for dy in (-1, 0, 1) for dx in (-1, 0, 1))
neon = np.where(nb >= 3, neon, 0)
lum = 0.2126 * lin[..., 0] + 0.7152 * lin[..., 1] + 0.0722 * lin[..., 2]
if C.get('lines'):
    # thin bright lines: much brighter than their 9x9 surroundings, not saturated (the cyan lamps already are lights)
    l9 = lum.copy()
    l9 = sum(np.roll(l9, d, 0) for d in range(-4, 5)) / 9
    l9 = sum(np.roll(l9, d, 1) for d in range(-4, 5)) / 9
    line = smoothstep(0.1, 0.25, lum - l9) * smoothstep(0.15, 0.35, lum) * (sat < 0.35)
    lit_ = (line > 0.3).astype(np.float32)
    nb_ = sum(np.roll(np.roll(lit_, dy, 0), dx, 1) for dy in (-1, 0, 1) for dx in (-1, 0, 1))
    line = np.where(nb_ >= 3, line, 0)
    lc = np.array(C['lines'], dtype=np.float32)
    to_srgb_ = lambda v: np.where(v <= 0.0031308, v * 12.92, 1.055 * np.power(v, 1 / 2.4) - 0.055)
    B[..., :3] = np.where(line[..., None] > 0.02, to_srgb_(lc * np.maximum(line, 0.5)[..., None]), B[..., :3])
    base_small.pixels.foreach_set(B.reshape(-1))
    srgb = B[..., :3]
    neon = np.maximum(neon, line)
    log('line texels', round(float((line > 0.02).mean()), 4))
light = neon > 0.02
# trim: light, neutral texels (silver / brushed metal painted by Meshy: only the VIPER has them; on the ROADSTER and
# TITAN 99% of the non-light texels lie between sRGB 14 and 45, so no chrome can be found in their textures);
# dark: the darkest texels (glass, vents, recesses: below sRGB ~20), glossy without a clear coat
trim = (~light) & (lum > 0.12) & (sat < 0.25)
recess = (~light) & (~trim) & (lum < 0.0055)


# pods (C['pod'], v0.2.27): the approved concepts and the brief ("four metallic anti-gravity pods") have metal pods; Meshy
# painted them like the body (dark, metal 0), so they read as black plastic. Their UV islands, found from the source
# triangles inside the production pod boxes, become polished metal of the given brightness (linear albedo), keeping the
# texture's shading detail (+-40 %), its light lines and its dark recesses.
def uv_mask(tri_sel, Hh, Ww):
    """Texels covered by the selected source triangles (a grid of barycentric samples on each)."""
    t = uv_tri[tri_sel]  # (n, 3, 2)
    m = np.zeros((Hh, Ww), dtype=bool)
    K = 8  # barycentric grid: 45 samples per triangle
    for w in [(i / K, j / K, (K - i - j) / K) for i in range(K + 1) for j in range(K + 1 - i)]:
        p = t[:, 0] * w[0] + t[:, 1] * w[1] + t[:, 2] * w[2]
        x = np.clip((p[:, 0] % 1.0 * Ww).astype(int), 0, Ww - 1)
        y = np.clip((p[:, 1] % 1.0 * Hh).astype(int), 0, Hh - 1)  # Blender image rows run bottom-up, like UV v
        m[y, x] = True
    return m


pod_mask = np.zeros_like(trim)
if C.get('pod'):
    me_src = src.data
    me_src.calc_loop_triangles()
    nt_ = len(me_src.loop_triangles)
    lt_loops = np.empty(nt_ * 3, dtype=np.int32); me_src.loop_triangles.foreach_get('loops', lt_loops)
    lt_verts = np.empty(nt_ * 3, dtype=np.int32); me_src.loop_triangles.foreach_get('vertices', lt_verts)
    uvl = np.empty(len(me_src.loops) * 2, dtype=np.float32); me_src.uv_layers.active.data.foreach_get('uv', uvl)
    uv_tri = uvl.reshape(-1, 2)[lt_loops].reshape(-1, 3, 2)
    vco = np.empty(len(me_src.vertices) * 3); me_src.vertices.foreach_get('co', vco)  # final (scaled) source space
    cen = vco.reshape(-1, 3)[lt_verts].reshape(-1, 3, 3).mean(1)
    in_pod = np.zeros(nt_, dtype=bool)
    for key in ('FL', 'FR', 'RL', 'RR'):
        a, b, _ = part_boxes('hi')[key]
        in_pod |= np.all((cen >= np.array(a)) & (cen <= np.array(b)), axis=1)
    Hh, Ww = trim.shape
    grow = lambda m: m | np.roll(m, 1, 0) | np.roll(m, -1, 0) | np.roll(m, 1, 1) | np.roll(m, -1, 1)
    shrink = lambda m: ~grow(~m)
    pm_ = shrink(shrink(grow(grow(uv_mask(in_pod, Hh, Ww)))))  # closed: no pinholes between samples
    body_m = uv_mask(~in_pod, Hh, Ww) & ~uv_mask(in_pod, Hh, Ww)  # texels only the body uses stay body
    # on the pods a recess is where the texture is dark on average (9x9): Meshy's dark texels are noisy, the threshold
    # alone left a pepper of metal specks in black areas and blotches of black on the metal
    l9 = lum.copy()  # 9x9 box blur, separable
    l9 = sum(np.roll(l9, d, 0) for d in range(-4, 5)) / 9
    l9 = sum(np.roll(l9, d, 1) for d in range(-4, 5)) / 9
    # v0.3: dark against the pod's own typical brightness, never stricter than the v0.2.27 0.0055: a retexture that
    # paints the whole pod near-black (ROADSTER variant B) would otherwise turn every pod texel into a black recess
    thr = min(0.0055, 0.35 * float(np.median(l9[pm_]))) if pm_.any() else 0.0055
    log('pod recess threshold', round(thr, 5))
    rp = grow(grow(shrink(shrink(l9 < thr))))  # opened (radius 2): coherent dark areas only
    pm_ = grow(grow(pm_)) & ~body_m  # and 2 texels into the gutters (no paint seams when mipmapped)
    recess = np.where(pm_, rp, recess)
    pod_mask = pm_ & ~light & ~recess
    albedo, rough = C['pod']
    # shading detail from the texture, blurred (5x5): Meshy's dark texels are noisy, metal would show every speck
    lb = sum(np.roll(np.roll(lum, dy, 0), dx, 1) for dy in range(-2, 3) for dx in range(-2, 3)) / 25
    detail = np.clip(lb / max(float(lb[pod_mask].mean()), 1e-4), 0.6, 1.4) if pod_mask.any() else 1
    silver = np.clip(albedo * detail, 0, 1)
    to_srgb = lambda v: np.where(v <= 0.0031308, v * 12.92, 1.055 * np.power(v, 1 / 2.4) - 0.055)
    B[..., :3] = np.where(pod_mask[..., None], to_srgb(silver)[..., None], B[..., :3])
    base_small.pixels.foreach_set(B.reshape(-1))
    srgb = B[..., :3]
    trim = trim | pod_mask
    log('pod metal texels', round(float(pod_mask.mean()), 4), 'pod triangles', int(in_pod.sum()), '/', nt_)
paint = ~(light | trim | recess)
regions = {'paint': float(paint.mean()), 'trim': float(trim.mean()), 'dark': float(recess.mean()), 'light': float(light.mean())}
log('regions (share of texels)', {k: round(v, 4) for k, v in regions.items()})
report['regions'] = regions
report['pod_metal_texels'] = float(pod_mask.mean())

H, W = neon.shape
def new_image(name, arr, non_color):
    img = bpy.data.images.new(name, W, H, alpha=False)
    img.colorspace_settings.name = 'Non-Color' if non_color else 'sRGB'
    rgba = np.ones((H, W, 4), dtype=np.float32)
    rgba[..., :3] = arr
    img.pixels.foreach_set(rgba.reshape(-1))
    img.file_format = 'PNG'
    img.filepath_raw = os.path.join(OUT_DIR, f'{name}.png')
    img.save()
    return img

# emissive: the base colour where it is a light, black elsewhere (sRGB values, like the base colour)
emis = np.where(light[..., None], srgb * neon[..., None], 0)
emis_img = new_image(f'{CAR}_emissive', emis, False)
# metal/roughness (+ coat in R): per region
mr = np.zeros((H, W, 3), dtype=np.float32)
pm, pr, pc = C['paint']
tm, tr, tc = C['trim']
for mask, (m, r, c) in ((paint, (pm, pr, pc)), (trim, (tm, tr, tc)), (recess, (0.0, 0.16, 0.0)), (light, (0.0, 0.35, 0.4))):
    mr[mask] = (c, r, m)
if C.get('pod'):
    mr[pod_mask] = (0.0, C['pod'][1], 1.0)  # polished metal, no coat
mr_img = new_image(f'{CAR}_orm', mr, True)
# region preview (for the audit; not used by the game): paint grey, trim white, recess black, light magenta
prev = np.zeros((H, W, 3), dtype=np.float32)
prev[paint] = (0.25, 0.25, 0.28); prev[trim] = (0.9, 0.9, 0.9); prev[pod_mask] = (0.3, 0.7, 1.0); prev[recess] = (0, 0, 0); prev[light] = (1, 0.1, 0.8)
new_image(f'{CAR}_regions_preview', prev, False)
normal_small = resized(normal_img, TEX)
normal_small.name = f'{CAR}_normal_src'
normal_small.colorspace_settings.name = 'Non-Color'


def game_material():
    m = bpy.data.materials.new(f'{CAR}_car')
    m.use_nodes = True
    nt = m.node_tree
    n = nt.nodes
    bsdf = n['Principled BSDF']
    def tex(img, non_color=False):
        t = n.new('ShaderNodeTexImage')
        t.image = img
        if non_color:
            t.image.colorspace_settings.name = 'Non-Color'
        return t
    tb, tmr, te = tex(base_small), tex(mr_img, True), tex(emis_img)
    nt.links.new(tb.outputs['Color'], bsdf.inputs['Base Color'])
    sep = n.new('ShaderNodeSeparateColor')
    nt.links.new(tmr.outputs['Color'], sep.inputs['Color'])
    nt.links.new(sep.outputs['Green'], bsdf.inputs['Roughness'])
    nt.links.new(sep.outputs['Blue'], bsdf.inputs['Metallic'])
    nt.links.new(sep.outputs['Red'], bsdf.inputs['Coat Weight'])
    bsdf.inputs['Coat Roughness'].default_value = 0.08
    nt.links.new(te.outputs['Color'], bsdf.inputs['Emission Color'])
    bsdf.inputs['Emission Strength'].default_value = 1.0
    nm = n.new('ShaderNodeNormalMap')
    tn = tex(normal_small, True)
    nt.links.new(tn.outputs['Color'], nm.inputs['Color'])
    nt.links.new(nm.outputs['Normal'], bsdf.inputs['Normal'])
    m.use_backface_culling = False
    return m, tn

mat, normal_node = game_material()


# ---------------- 5. game meshes: decimate per part to the production budget, normals from the source ----------------
def game_part(srcpart, name, target, pivot):
    o = srcpart.copy()
    o.data = srcpart.data.copy()
    o.name = name
    o.data.name = name
    bpy.context.scene.collection.objects.link(o)
    n = tris(o.data)
    if target < n:
        d = o.modifiers.new('decimate', 'DECIMATE')
        d.ratio = target / n
        d.use_collapse_triangulate = True
        bpy.context.view_layer.objects.active = o
        bpy.ops.object.modifier_apply(modifier=d.name)
    # normals: transferred from the dense source part (its smooth, flat panels), per face corner
    dt = o.modifiers.new('normals', 'DATA_TRANSFER')
    dt.object = srcpart
    dt.use_loop_data = True
    dt.data_types_loops = {'CUSTOM_NORMAL'}
    dt.loop_mapping = 'POLYINTERP_NEAREST'
    bpy.context.view_layer.objects.active = o
    bpy.ops.object.modifier_apply(modifier=dt.name)
    o.data.materials.clear()
    o.data.materials.append(mat)
    if pivot is not None:
        o.data.transform(Matrix.Translation(-pivot))
        o.location = pivot
    return o


created = []
for lod in ('hi', 'lo'):
    boxes = part_boxes(lod)
    parts = split_source(boxes)
    body_target = C['body'][lod] if C.get('body') else prod[f'{lod}_body']['tris']
    created.append(game_part(parts['body'], f'{lod}_body', body_target, None))
    for key, pivot in POD_PIVOTS.items():  # empty pivots (Blender empties export as glTF nodes)
        e = bpy.data.objects.new(f'{lod}_pod_{key}', None)
        e.location = pivot
        bpy.context.scene.collection.objects.link(e)
    for key, (_, _, pivot) in boxes.items():
        created.append(game_part(parts[key], f'{lod}_pod_{key}', prod[f'{lod}_pod_{key}']['tris'], pivot))
    for o in parts.values():
        if lod == 'lo' or not BAKE:
            bpy.data.objects.remove(o)
        else:
            o.hide_render = False
    if lod == 'hi' and BAKE:
        hi_sources = parts

# ---------------- 6. optional: bake a tangent-space normal map from the dense source onto the game mesh ----------------
if BAKE:
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.samples = 1
    scene.cycles.device = 'CPU'
    baked = bpy.data.images.new(f'{CAR}_normal', TEX, TEX, alpha=False, float_buffer=False)
    baked.colorspace_settings.name = 'Non-Color'
    # the source keeps its own material (with the Meshy normal map), so the bake carries geometry + micro detail
    for o in hi_sources.values():
        o.data.materials.clear()
        o.data.materials.append(src_mat)
    bake_node = mat.node_tree.nodes.new('ShaderNodeTexImage')
    bake_node.image = baked
    mat.node_tree.nodes.active = bake_node
    for o in created:
        if not o.name.startswith('hi_'):
            continue
        srcpart = hi_sources['body' if o.name == 'hi_body' else o.name.split('_')[-1]]
        bpy.ops.object.select_all(action='DESELECT')
        srcpart.select_set(True)
        o.select_set(True)
        bpy.context.view_layer.objects.active = o
        bpy.ops.object.bake(type='NORMAL', normal_space='TANGENT', use_selected_to_active=True, cage_extrusion=0.03,
                            max_ray_distance=0.06, margin=4, use_clear=(o.name == 'hi_body'))
        log('baked', o.name)
    baked.filepath_raw = os.path.join(OUT_DIR, f'{CAR}_normal.png')
    baked.file_format = 'PNG'
    baked.save()
    mat.node_tree.nodes.remove(bake_node)
    normal_node.image = baked
    for o in hi_sources.values():
        bpy.data.objects.remove(o)

bpy.data.objects.remove(src)
report['parts'] = {o.name: dict(tris=tris(o.data), loc=tuple(o.location)) for o in created}
for o in created:
    log('part', o.name, tris(o.data), tuple(round(v, 4) for v in o.location))

# ---------------- 7. export (WebP 90, tangents, the authored maps) and keep the editable .blend ----------------
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT_DIR, f'{CAR}.blend'))
bpy.ops.export_scene.gltf(filepath=OUT, export_format='GLB', export_image_format='WEBP', export_image_quality=90,
                          export_tangents=True, export_normals=True, use_selection=False)
report['out'] = OUT
report['seconds'] = round(time.time() - t0, 1)
json.dump(report, open(os.path.join(OUT_DIR, f'{CAR}_report.json'), 'w'), indent=1)
log('EXPORTED', OUT, f'{report["seconds"]} s')

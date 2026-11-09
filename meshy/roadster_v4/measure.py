# Normalises roadster_v4/model.glb exactly like refine_car.py (rotate -90 Z, length 5.6 along Y, centred) and finds its
# four wheels: per corner, the vertices of the low outer region; prints each wheel's bounds (centre, radius, x faces).
import sys, math, bpy, numpy as np
from mathutils import Matrix, Vector
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=sys.argv[sys.argv.index('--') + 1])
o = [o for o in bpy.context.scene.objects if o.type == 'MESH'][0]
o.data.transform(o.matrix_world); o.matrix_world = Matrix.Identity(4)
o.data.transform(Matrix.Rotation(math.radians(-90), 4, 'Z'))
co = np.empty(len(o.data.vertices) * 3); o.data.vertices.foreach_get('co', co); co = co.reshape(-1, 3)
lo, hi = co.min(0), co.max(0)
if hi[1] - lo[1] < hi[0] - lo[0]: print('ROTATE 90 needed')
s = 5.6 / (hi[1] - lo[1]); co = (co - (lo + hi) / 2) * s
print('size', (co.max(0) - co.min(0)).round(3), 'zmin', co[:, 2].min().round(3))
# side profile of the lowest points along y (5 cm bins): wheels are where the underside dips lowest
z0 = co[:, 2].min()
for y in np.arange(-2.8, 2.8, 0.1):
    m = (co[:, 1] >= y) & (co[:, 1] < y + 0.1)
    if m.any(): print(f'y {y:+.1f} zmin {co[m, 2].min():+.3f} xmax {np.abs(co[m, 0]).max():.3f}')
for name, (ya, yb) in (('F', (0.9, 2.1)), ('R', (-2.4, -1.0))):
    for side in (-1, 1):
        for xa in (1.2, 1.3, 1.45, 1.55):
            m = (co[:, 1] > ya) & (co[:, 1] < yb) & (co[:, 0] * side > xa) & (co[:, 2] < 0.2)
            if m.sum() < 20: continue
            p = co[m]; print(f'{name}{"L" if side < 0 else "R"} |x|>{xa}: n {m.sum()} y {p[:,1].min():+.3f}..{p[:,1].max():+.3f} z {p[:,2].min():+.3f}..{p[:,2].max():+.3f} xmax {np.abs(p[:,0]).max():.3f}')
def fit(p):  # least-squares circle in (y, z)
    A = np.c_[2 * p[:, 0], 2 * p[:, 1], np.ones(len(p))]; b = (p ** 2).sum(1)
    cy, cz, c = np.linalg.lstsq(A, b, rcond=None)[0]; return cy, cz, math.sqrt(c + cy * cy + cz * cz)
for name, (ya, yb) in (('F', (0.9, 2.1)), ('R', (-2.4, -1.0))):
    for side in (-1, 1):
        m = (co[:, 1] > ya) & (co[:, 1] < yb) & (co[:, 0] * side > 1.3) & (co[:, 2] < -0.45)
        p = co[m][:, 1:3]
        # the tyre's outline: per 2 deg angle around a first centre, the farthest point
        cy, cz, r = fit(p)
        ang = np.degrees(np.arctan2(p[:, 1] - cz, p[:, 0] - cy)); d = np.hypot(p[:, 0] - cy, p[:, 1] - cz)
        rim = np.array([p[(ang >= a) & (ang < a + 2)][np.argmax(d[(ang >= a) & (ang < a + 2)])] for a in range(-180, 0, 2) if ((ang >= a) & (ang < a + 2)).any()])
        cy, cz, r = fit(rim)
        xs = co[m][:, 0] * side
        print(f'{name}{"L" if side < 0 else "R"} centre y {cy:+.3f} z {cz:+.3f} r {r:.3f} x {xs.min():.3f}..{xs.max():.3f}')
for name, (cy, cz, r) in (('F', (1.522, -0.433, 0.384)), ('R', (-1.72, -0.43, 0.41))):
    d = np.hypot(co[:, 1] - cy, co[:, 2] - cz)
    for band in ((0, 0.6), (0.6, 0.9), (0.9, 1.02)):
        m = (d >= r * band[0]) & (d < r * band[1]) & (co[:, 0] > 0)
        h, e = np.histogram(co[m, 0], bins=np.arange(0.6, 1.75, 0.05))
        print(name, band, ' '.join(f'{x:.2f}:{c}' for x, c in zip(e, h) if c))

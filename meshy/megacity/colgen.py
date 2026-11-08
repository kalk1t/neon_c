"""Living Megacity collision shapes from the models themselves (v0.3.6): a model is voxelised (pitch m), every closed
cross-section filled (a wall ring makes a solid floor; an open wind floor or a gap between towers stays open), and the
solid voxels are merged into as few boxes as possible. Boxes, not triangles: a car at 190 m/s moves 3 m a frame, and a
box pushes it out from inside too, so nothing is tunnelled through.

Output: Blender frame boxes [x0, x1, y0, y1, z0, z1] (m; glTF (x, y, z) -> Blender (x, -z, y)), written into the
building's JSON as "colliders" (game: megacity.js stageColliders uses them before the stages) or, for props, into
public/megacity/props/PROPS-colliders.json.

  python colgen.py <model.glb> [--json <meta.json>] [--pitch 1.5] [--key name --props <file>]
"""
import sys, json, argparse
import numpy as np
import trimesh
from scipy import ndimage

ap = argparse.ArgumentParser()
ap.add_argument('glb')
ap.add_argument('--json')
ap.add_argument('--props')
ap.add_argument('--key')
ap.add_argument('--fill', default='slices', choices=['auto', 'slices', '3d', 'closed'])
ap.add_argument('--close', type=int, default=1, help='closing iterations after the slice fill (pits)')
ap.add_argument('--seal', type=int, default=0, help='2D closing (voxels) of each cross-section before its fill: seals facade gaps (a hollow tower)')
ap.add_argument('--out', help='also write the boxes alone to this file (review)')
ap.add_argument('--pitch', type=float, default=1.5)
ap.add_argument('--min', type=float, default=0.0, help='drop boxes thinner than this (m) in every axis but one')
a = ap.parse_args()

scene = trimesh.load(a.glb, force='scene')
mesh = scene.to_geometry() if hasattr(scene, 'to_geometry') else scene.dump(concatenate=True)
P = a.pitch
lo = mesh.bounds[0] - P
# surface voxels: points sampled densely on every triangle (spacing < pitch / 2)
tri = mesh.triangles
area = mesh.area_faces
n = np.clip((area / (P * P * 0.12)).astype(int) + 3, 3, 2_000_000)   # no low cap: a 200 m wall needs ~10^5 points
pts = [tri.mean(axis=1), tri[:, 0], tri[:, 1], tri[:, 2]]
rng = np.random.default_rng(1)
rep = np.repeat(np.arange(len(tri)), n)
u = rng.random((len(rep), 2))
flip = u.sum(axis=1) > 1
u[flip] = 1 - u[flip]
A, B, Cc = tri[rep, 0], tri[rep, 1], tri[rep, 2]
samp = A + (B - A) * u[:, :1] + (Cc - A) * u[:, 1:]
pts = np.concatenate([samp] + pts)
idx = np.floor((pts - lo) / P).astype(int)
shape = idx.max(axis=0) + 2
vox = np.zeros(shape, dtype=bool)
vox[idx[:, 0], idx[:, 1], idx[:, 2]] = True
# solid = every voxel the outside air can't reach (3D): rooms and the inside of walls fill, a courtyard open to the
# sky or an open wind floor stays open. If the surface has a gap the air leaks in; then fall back to filling each
# horizontal cross-section (glTF y is up: slices along axis 1), which is never too thin.
shell = vox.copy()
closed = ndimage.binary_closing(np.pad(shell, 2), structure=np.ones((3, 3, 3), bool), iterations=1)[2:-2, 2:-2, 2:-2]
vox3 = ndimage.binary_fill_holes(closed | shell)     # slits up to ~2 voxels (louvres, balcony gaps) are sealed first
vox2 = shell.copy()
for j in range(shape[1]):
    s = shell[:, j, :]
    if s.any():
        if a.seal:   # gaps in the wall ring (open corners, louvres) up to ~2 x seal voxels: closed before the fill
            k = a.seal + 1
            s = s | ndimage.binary_closing(np.pad(s, k), structure=np.ones((3, 3), bool), iterations=a.seal)[k:-k, k:-k]
        vox2[:, j, :] = ndimage.binary_fill_holes(s)
vox3o = ndimage.binary_fill_holes(shell)
ratio = vox3.sum() / max(1, vox2.sum())
vox = {'slices': vox2, '3d': vox3o, 'closed': vox3}.get(a.fill, vox3 if ratio > 0.8 else vox2)
if a.fill == 'slices':        # pits where the sampling missed a voxel: a light closing of the solid (gaps > ~3 m stay open)
    vox = vox | ndimage.binary_closing(np.pad(vox, a.close + 1), structure=np.ones((3, 3, 3), bool), iterations=a.close)[a.close + 1:-a.close - 1, a.close + 1:-a.close - 1, a.close + 1:-a.close - 1]
print(f'  3D fill keeps {ratio:.0%} of the slice fill -> {"3D" if ratio > 0.8 else "slices"}')

# greedy merge: rectangles per slice, then stack equal rectangles of consecutive slices into boxes
def rects(s):
    s = s.copy()
    out = []
    X, Z = s.shape
    for x in range(X):
        z = 0
        while z < Z:
            if not s[x, z]:
                z += 1
                continue
            z1 = z
            while z1 < Z and s[x, z1]:
                z1 += 1
            x1 = x + 1
            while x1 < X and s[x1, z:z1].all():
                x1 += 1
            s[x:x1, z:z1] = False
            out.append((x, x1, z, z1))
            z = z1
    return out

boxes = []
open_ = {}
for j in range(shape[1] + 1):
    cur = set(rects(vox[:, j, :])) if j < shape[1] else set()
    nxt = {}
    for r in cur:
        nxt[r] = open_.pop(r, j)
    for r, j0 in open_.items():
        boxes.append((r, j0, j))
    open_ = nxt

out = []
for (x0, x1, z0, z1), j0, j1 in boxes:
    gx0, gx1 = lo[0] + x0 * P, lo[0] + x1 * P
    gy0, gy1 = lo[1] + j0 * P, lo[1] + j1 * P
    gz0, gz1 = lo[2] + z0 * P, lo[2] + z1 * P
    # Blender frame: x = gx, y = -gz, z = gy
    out.append([round(gx0, 2), round(gx1, 2), round(-gz1, 2), round(-gz0, 2), round(max(gy0, 0.0), 2), round(gy1, 2)])
vol_vox = vox.sum() * P ** 3
print(f'{a.glb}: {len(tri)} tris, grid {tuple(shape)}, solid {vol_vox:.0f} m3, {len(out)} boxes')

if a.json:
    meta = json.load(open(a.json, encoding='utf8'))
    meta['colliders'] = out
    meta['colliders_pitch'] = P
    json.dump(meta, open(a.json, 'w', encoding='utf8'), separators=(',', ':'))
if a.props:
    try:
        allp = json.load(open(a.props, encoding='utf8'))
    except FileNotFoundError:
        allp = {}
    allp[a.key] = out
    json.dump(allp, open(a.props, 'w', encoding='utf8'), separators=(',', ':'))
if a.out:
    json.dump(out, open(a.out, 'w'))

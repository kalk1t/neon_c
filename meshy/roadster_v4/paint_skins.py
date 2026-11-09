# v0.3.9 ROADSTER pilot: the 10 shop skins (r01-r10), repainted on the new roadster4 body. The old skins were painted
# on the old body's UV atlas (Assets/01_Vehicle/skins/roadster); the new body has another, so each skin is painted again:
# its colours and finish measured on the old skin (paint median colour, region map clear coat / roughness / metal),
# its pattern (stripes, carbon weave, camo, aurora gradient) drawn in 3D from the body's baked position and normal
# (bake_pos.py). Only the paint region changes (region map metal 0.1-0.85); trim, glass, recesses and lights stay the
# car's own. The panel shading of the car's paint (its dark and light panels) is kept as light and dark tones.
# Usage: python paint_skins.py <car base png/webp> <car orm png> <pos dir (pos.npy, nrm.npy)> <cover png> <out dir>
# Writes <out>/r01_base.png, r01_orm.png, ... (2048, the car atlas); game/tools/extract-skins.mjs --png packs them.
import os, sys
import numpy as np
from PIL import Image

base_p, orm_p, pos_d, cover_p, out = sys.argv[1:6]
os.makedirs(out, exist_ok=True)
base = np.asarray(Image.open(base_p).convert('RGB')).astype(np.float32) / 255
orm = np.asarray(Image.open(orm_p).convert('RGB')).astype(np.float32) / 255
pos = np.load(os.path.join(pos_d, 'pos.npy')).astype(np.float32)[..., :3]
nrm = np.load(os.path.join(pos_d, 'nrm.npy')).astype(np.float32)[..., :3]
cover = np.asarray(Image.open(cover_p).convert('L')) > 128
x, y, z = pos[..., 0], pos[..., 1], pos[..., 2]   # Blender axes: x right, y forward (nose +y), z up
nz = nrm[..., 2]

ss = lambda a, b, v: np.clip((v - a) / (b - a), 0, 1) ** 2 * (3 - 2 * np.clip((v - a) / (b - a), 0, 1))
metal = orm[..., 2]
paint = ss(0.05, 0.15, metal) * (1 - ss(0.85, 0.98, metal)) * cover
# the car's own panel tones: its paint is near black with lighter grey panels; v = 0 at the median, 1 at the light panels
lum = base @ np.array([0.2126, 0.7152, 0.0722], np.float32)
med = np.median(lum[paint > 0.5])
hi = np.percentile(lum[paint > 0.5], 92)
# (darker than the median only a little: the atlas has dark texels along its seams, which read as cracks on light paint)
tone = np.clip((lum - med) / max(hi - med, 1e-3), -0.3, 1.4)

srgb = lambda *c: np.array(c, np.float32) / 255
top = ss(0.25, 0.45, nz)  # faces looking up (hood, roof, deck)


def shade(col, k=0.45):
    """The colour on the car's panels: the median panel as `col`, light panels brighter, recesses darker."""
    return np.clip(col * (1 + k * tone)[..., None], 0, 1)


def band(v, a, b, soft=0.012):
    return ss(a - soft, a + soft, v) * (1 - ss(b - soft, b + soft, v))


def hash3(ix, iy, iz):
    h = (ix * 73856093) ^ (iy * 19349663) ^ (iz * 83492791)
    h = (h ^ (h >> 13)) * 1274126177
    return ((h ^ (h >> 16)) & 0xffff).astype(np.float32) / 65535


def cells(size):
    """Value noise on a 3D grid of `size` metres, one value per cell (digital camo)."""
    return hash3(np.floor(x / size).astype(np.int64), np.floor(y / size).astype(np.int64), np.floor(z / size).astype(np.int64))


def solid(col, dark=None, k=0.45):
    return lambda: shade(srgb(*col), k)


def stripes(col, stripe, spans, where=None):
    def f():
        c = shade(srgb(*col))
        m = sum(band(np.abs(x), a, b) for a, b in spans) * top
        if where is not None: m = m * where
        return c * (1 - m[..., None]) + shade(srgb(*stripe), 0.3) * m[..., None]
    return f


def carbon():
    # 2x2 twill, 22 mm tows, laid on the plane facing each texel (triplanar by the strongest normal axis)
    a = np.abs(nrm); ax = a.argmax(-1)
    u = np.where(ax == 0, y, x); v = np.where(ax == 2, y, z)
    s = 0.022
    iu, iv = np.floor(u / s), np.floor(v / s)
    weave = ((iu + iv) % 4 < 2)
    fu, fv = u / s - iu, v / s - iv
    sheen = np.where(weave, np.sin(np.pi * fu), np.sin(np.pi * fv)) ** 0.6
    c = srgb(26, 27, 30) + (srgb(70, 72, 78) - srgb(26, 27, 30)) * (0.25 + 0.75 * sheen * np.where(weave, 1.0, 0.7))[..., None]
    return np.clip(c * (1 + 0.25 * tone)[..., None], 0, 1)


def camo():
    # urban digital camo: blocks of 12 cm broken by 4 cm pixels, four greys
    n = 0.7 * cells(0.12) + 0.3 * cells(0.04)
    greys = [srgb(48, 52, 56), srgb(92, 98, 104), srgb(150, 156, 162), srgb(205, 210, 214)]
    lvl = np.digitize(n, [0.3, 0.52, 0.74])
    c = np.zeros(x.shape + (3,), np.float32)
    for i, g in enumerate(greys): c[lvl == i] = g
    return np.clip(c * (1 + 0.3 * tone)[..., None], 0, 1)


def aurora():
    # iridescent sweep: deep blue at the nose, teal over the middle, violet at the tail, shifting with height
    t = np.clip(0.5 - y / 5.6 + 0.18 * z + 0.08 * np.abs(x), 0, 1)
    keys = [(0.0, srgb(30, 60, 170)), (0.45, srgb(20, 140, 150)), (0.75, srgb(70, 80, 190)), (1.0, srgb(125, 55, 175))]
    c = np.zeros(x.shape + (3,), np.float32)
    for (t0, c0), (t1, c1) in zip(keys, keys[1:]):
        k = np.clip((t - t0) / (t1 - t0), 0, 1)[..., None]
        sel = ((t >= t0) & (t <= t1))[..., None]
        c = np.where(sel, c0 + (c1 - c0) * k, c)
    return shade(c)


# code: (name, paint, finish = region map clear coat, roughness, metal of the old skin's paint, 0-255)
SKINS = {
    'r01': ('Obsidian', solid((24, 26, 30), k=0.9), (240, 48, 56)),
    'r02': ('Arctic', solid((214, 219, 225), k=0.25), (236, 64, 52)),
    'r03': ('Crimson', stripes((132, 16, 30), (16, 16, 19), [(0.10, 0.28)]), (240, 56, 120)),
    'r04': ('Cobalt', stripes((26, 58, 150), (14, 24, 62), [(0.0, 0.2)]), (240, 60, 136)),
    'r05': ('Vipersteel', solid((96, 108, 122), k=0.5), (104, 104, 204)),
    'r06': ('Desert', stripes((180, 150, 108), (104, 80, 56), [(0.0, 0.16)]), (72, 124, 68)),
    'r07': ('Forest', stripes((72, 86, 44), (24, 27, 24), [(0.0, 0.62)], where=ss(0.4, 0.6, y)), (192, 80, 112)),
    'r08': ('Carbon', carbon, (208, 80, 88)),
    'r09': ('Aurora', aurora, (240, 56, 152)),
    'r10': ('Battlecamo', camo, (8, 168, 40)),
}

# texels outside the UV islands take the nearest island texel's value: filtering and mipmaps sample across the island
# edges, and the car's black there showed as dark cracks along every seam of a light skin
from scipy.ndimage import distance_transform_edt
_, (iy, ix) = distance_transform_edt(~cover, return_indices=True)

for code, (name, paint_fn, finish) in SKINS.items():
    col = paint_fn()
    p = paint[..., None]
    b = base * (1 - p) + col * p
    o = orm.copy()
    fin = srgb(*finish)
    o = o * (1 - p) + fin * p
    b, o = b[iy, ix], o[iy, ix]
    Image.fromarray((b * 255 + 0.5).astype(np.uint8)).save(os.path.join(out, f'{code}_base.png'))
    Image.fromarray((o * 255 + 0.5).astype(np.uint8)).save(os.path.join(out, f'{code}_orm.png'))
    print(code, name)

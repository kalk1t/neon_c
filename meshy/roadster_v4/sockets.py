import sys, bpy, numpy as np
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=sys.argv[sys.argv.index('--') + 1])
o = bpy.data.objects['hi_body']; co = np.array([list(o.matrix_world @ v.co) for v in o.data.vertices])
def q(name, xa, xb, za, zb, end):
    m = (np.abs(co[:, 0]) >= xa) & (np.abs(co[:, 0]) <= xb) & (co[:, 2] >= za) & (co[:, 2] <= zb)
    y = co[m, 1]; print(name, 'n', m.sum(), 'y min/max', y.min().round(3), y.max().round(3), 'pcts', np.percentile(y, [2, 50, 98]).round(3))
q('nose cannon centre', 0, 0.05, -0.33, -0.27, 'front')
q('nose cannon ring', 0.08, 0.14, -0.33, -0.27, 'front')
q('rear exhaust centre', 0, 0.05, -0.39, -0.33, 'rear')
q('rear exhaust ring', 0.1, 0.16, -0.39, -0.33, 'rear')
# side flames: the rear corners' vents, low and outboard
for z in (-0.5, -0.4, -0.3, -0.2, -0.1, 0.0):
    m = (np.abs(co[:, 2] - z) < 0.05) & (co[:, 1] < -2.3)
    if m.any(): print('rear z', z, 'x range', np.abs(co[m, 0]).max().round(3), 'ymin', co[m, 1].min().round(3))

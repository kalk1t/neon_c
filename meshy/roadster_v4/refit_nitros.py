# v0.3.9 ROADSTER pilot: refits the 10 ROADSTER shop nitros (game/public/vfx/roadster/ROADSTER_NITRO_MANIFEST.json) to
# the new roadster4 body. The old body's side emitters sat on the rear hover pods; the new car's pods are its wheels,
# so all three emitters go on the body (as VIPER and TITAN), at the spots game/src/car.js CAR_MODELS.roadster flames from
# (sockets.py; car space there, file space here = car space - y): MAIN the round tail exhaust, POD_RL/RR (the GLBs'
# mount names are kept) the dark rear light panels. The effect GLBs are unchanged (sha256 as delivered).
# Usage: python refit_nitros.py <game dir>
import hashlib, json, os, sys
game = sys.argv[1]
Y = -0.5  # CAR_MODELS.roadster.y
SPOTS = {'MAIN': [0, -0.86, 2.73], 'POD_RL': [-1.05, -0.8, 2.78], 'POD_RR': [1.05, -0.8, 2.78]}  # car space
p = os.path.join(game, 'public/vfx/roadster/ROADSTER_NITRO_MANIFEST.json')
m = json.load(open(p, encoding='utf-8'))
car = os.path.join(game, 'public/models/car.glb')
m['sourceFile'] = 'public/models/car.glb (v0.3.9 roadster4)'
m['sourceSha256'] = hashlib.sha256(open(car, 'rb').read()).hexdigest()
m['socketStatus'] = 'v0.3.9: refitted to the roadster4 body (meshy/roadster_v4/refit_nitros.py): all three emitters on the body'
for k in [k for k in m['sourceNodeRestTranslations'] if '_pod_' in k]: del m['sourceNodeRestTranslations'][k]
for e in m['effects']:
    for s in e['sockets']:
        x, y, z = SPOTS[s['id']]
        f = [x, round(y - Y, 3), z]
        s.update(attachNodeHi='hi_body', attachNodeLo='lo_body', followsPodTilt=False, positionCarLocalMetres=f,
                 positionParentLocalHiMetres=f, positionParentLocalLoMetres=f, quaternionParentLocalXYZW=[0, 0, 0, 1])
json.dump(m, open(p, 'w', encoding='utf-8', newline='\n'), indent=2)
print('refitted', len(m['effects']), 'nitros to', m['sourceSha256'][:12])

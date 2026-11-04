from pathlib import Path
import json,hashlib,zipfile
p=Path(r'C:\Users\User\Projects\neon_c');package=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3');o=Path(__file__).parent/'reticle-fixture';o.mkdir(exist_ok=True)
sha=lambda b:hashlib.sha256(b).hexdigest();manifest=json.loads((package/'manifest.json').read_text());entries={x['file']:x for x in manifest['assets']};verified=[]
with zipfile.ZipFile(package.parent/'RoadstersSharedAssets-v3-ui.zip') as z:
 for rel in ['assets.json','source/style.css']:
  b=(package/rel).read_bytes();e=entries[rel];assert len(b)==e['bytes'] and sha(b)==e['sha256'];matches=[n for n in z.namelist() if n==rel or n.endswith('/'+rel)];assert len(matches)==1 and z.read(matches[0])==b;verified.append({'file':rel,'sha256':sha(b)})
css=p/'game/src/style.css';current=css.read_text(encoding='utf-8');old=(package/'source/style.css').read_text(encoding='utf-8')
def rules(s):return '\n'.join(x for x in s.splitlines() if x.startswith(('#crosshair','#aim')))
assert rules(current)==rules(old),'Reticle styles changed'
items=json.loads((package/'assets.json').read_text());samples=[];rows=[]
for i,key in enumerate(['hud/markers/crosshair','hud/markers/aim']):
 item=next(x for x in items if x.get('scale')==1 and x['id']==key);rect=item['observed']['rect'];border=float(item['observed']['styles']['border'].split('px')[0]);samples.append({'id':key,'width':rect['width'],'height':rect['height'],'border':border});rows.append('{'+','.join(map(str,[i,rect['width'],rect['height'],border]))+'}')
(o/'reticle_box_fixture.h').write_text('/* Manifest-verified Chromium border-box measurements. */\nstatic const ReticleBoxCase reticle_boxes[]={\n'+',\n'.join(rows)+'\n};\n')
receipt={'package':str(package),'manifest_sha256':sha((package/'manifest.json').read_bytes()),'selected_directory_and_archive_files_verified':verified,'current_css':str(css),'current_css_sha256':sha(css.read_bytes()),'selected_css_matches_packaged_reference':True,'samples':samples,'scope':'Two reused actual Chromium outer box/border measurements; selected current CSS matches archived reference. Pseudo-element tick/center-dot geometry follows current CSS, not new raster observations.'}
(o/'reticle-box-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print('Verified two measured reticle boxes and selected current CSS')

from pathlib import Path
import json,hashlib,zipfile,re
p=Path(r'C:\Users\User\Projects\neon_c');package=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3');o=Path(__file__).parent/'boost-meter-fixture';o.mkdir(exist_ok=True)
sha=lambda b:hashlib.sha256(b).hexdigest();manifest=json.loads((package/'manifest.json').read_text());entries={x['file']:x for x in manifest['assets']};verified=[]
with zipfile.ZipFile(package.parent/'RoadstersSharedAssets-v3-ui.zip') as z:
 for rel in ['assets.json','source/style.css','source/hud.js']:
  b=(package/rel).read_bytes();e=entries[rel];assert len(b)==e['bytes'] and sha(b)==e['sha256'];matches=[n for n in z.namelist() if n==rel or n.endswith('/'+rel)];assert len(matches)==1 and z.read(matches[0])==b;verified.append({'file':rel,'sha256':sha(b)})
current=p/'game/src/style.css';css=current.read_text(encoding='utf-8');old=(package/'source/style.css').read_text(encoding='utf-8')
def selected(s):return '\n'.join(line for line in s.splitlines() if line.startswith(('.boost {','.boost i {','.boost.on i {','.boost.low i {','@keyframes nitro-bar','@keyframes nitro-low')))
assert selected(css)==selected(old),'Boost CSS changed'
hud=p/'game/src/hud.js';current_hud=hud.read_text(encoding='utf-8');packaged_hud=(package/'source/hud.js').read_text(encoding='utf-8')
def boost_rules(s):return '\n'.join(line.strip() for line in s.splitlines() if line.strip().startswith('e.boost.'))
assert boost_rules(current_hud)==boost_rules(packaged_hud),'Boost HUD rules changed'
assets=json.loads((package/'assets.json').read_text());samples=[];rows=[]
for item in assets:
 if item.get('scale')!=1 or not item['id'].startswith('hud/meters/boost/'):continue
 observed=item['observed'];child=observed['children'][0];style=child['styles'];car=item['state']['car'];on=bool(car.get('boosting',False));low='low' in observed['classes'].split();animation=2 if 'nitro-low' in style['animation'] else 1 if 'nitro-bar' in style['animation'] else 0
 rect=child['rect'];panel=next(x for x in assets if x.get('scale')==1 and x['id']=='hud/speed-panel/normal')
 # Combine two measured relative rectangles, without deriving expected bounds
 # from the native skew formula being tested.
 parent=next(c for c in panel['observed']['children'] if 'boost' in c['classes'].split())['rect']
 bounds=[rect['x']+parent['x'],rect['y']+parent['y'],rect['width'],rect['height']]
 brightness=float(re.search(r'brightness\(([^)]+)\)',style['filter']).group(1)) if style['filter']!='none' else 1
 sample={'id':item['id'],'boost':car['boost'],'on':on,'low':low,'animation':animation,'opacity':float(style['opacity']),'brightness':brightness,'relative_fill_bounds':bounds,'computed_animation':style['animation'],'computed_filter':style['filter']};samples.append(sample)
 rows.append('{'+','.join(str(int(sample[k])) if isinstance(sample[k],bool) else str(sample[k]) for k in ['boost','on','low','animation','opacity','brightness'])+',{'+','.join(map(str,bounds))+'}}')
assert len(samples)==6
(o/'boost_meter_fixture.h').write_text('/* Manifest-verified Chromium initial meter styles and relative skew geometry. */\nstatic const BoostCase boost_cases[]={\n'+',\n'.join(rows)+'\n};\n')
receipt={'package':str(package),'manifest_sha256':sha((package/'manifest.json').read_bytes()),'selected_directory_and_archive_files_verified':verified,'current_css':str(current),'current_css_sha256':sha(current.read_bytes()),'current_hud':str(hud),'current_hud_sha256':sha(hud.read_bytes()),'selected_hud_boost_rules_match_packaged_reference':True,'selected_css_sha256':sha(selected(css).encode()),'selected_css_matches_packaged_chromium_reference':True,'samples':samples,'scope':'Six reused Chromium initial meter states/child geometry from verified v3 package. Current selected CSS and HUD boost rules unchanged. Animation endpoints follow source; intermediate easing and native GPU/filter pixels not engine-verified.'}
(o/'boost-meter-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps({'samples':len(samples),'directory_and_archive_files':len(verified),'selected_css_identical':True}))

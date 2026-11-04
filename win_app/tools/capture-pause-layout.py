from pathlib import Path
import hashlib,json,re,math
o=Path(__file__).parent/'pause-layout-fixture';o.mkdir(exist_ok=True)
p=Path(r'C:\Users\User\Projects\neon_c');package=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3')
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
manifest=json.loads((package/'manifest.json').read_text());assert sha(package/'manifest.json')=='e8eca0c29f6f392ca1a81ef602663de6438847db12306594011bb4677dfd0443'
files=['assets.json','source/style.css','source/index.html']
for name in files:
    entry=next(x for x in manifest['assets'] if x['file']==name);assert sha(package/name)==entry['sha256'],name
current=(p/'game/src/style.css').read_text(encoding='utf-8');prior=(package/'source/style.css').read_text(encoding='utf-8')
selectors=['  --c:','  --bg:','  --font:','* {','#menu, #pause, #results {','#pause, #results {','.menu-card {','.menu-card.small {','.brand {','.brand.sm {','button {','.pause-hint {','.pause-contact, .small-contact {','#pause {']
selected=[]
for selector in selectors:
    a=next(line for line in current.splitlines() if line.startswith(selector));b=next(line for line in prior.splitlines() if line.startswith(selector));assert a==b,selector;selected.append(a)
raw=json.loads((package/'assets.json').read_text());records=[a for a in raw if a['id'] in ['pause/panel/desktop','pause/panel/compact'] and a['scale']==1];assert len(records)==2
rows=[]
for a in records:
    rect=a['observed']['rect'];children=a['observed']['children'];buttons=children[1:4]
    assert rect['width']==380 and rect['height']==394
    assert [b['rect']['y'] for b in buttons]==[84,159,234]
    assert all(b['rect']['x']==32 and b['rect']['width']==316 and b['rect']['height']==61 and b['styles']['fontSize']=='20px' for b in buttons)
    rows.append('{'+','.join(map(str,[a['viewport']['width'],a['viewport']['height'],rect['x'],rect['y'],rect['width'],rect['height']]))+',{'+','.join('{'+','.join(map(str,[b['rect']['x'],b['rect']['y'],b['rect']['width'],b['rect']['height']]))+'}' for b in buttons)+'}}')
rule=next(x for x in selected if x.startswith('#pause {'));m=re.search(r'rgba\(3, 4, 20, ([.\d]+)\) 0%, rgba\(3, 4, 20, ([.\d]+)\) (\d+)%?, transparent (\d+)%',rule);assert m
a,b,x,y=map(float,m.groups());x/=100;y/=100;grad=[]
for u in sorted(set([i/1000 for i in range(1001)]+[-1,x-1e-9,x,x+1e-9,y-1e-9,y,y+1e-9,2])):
    alpha=a if u<0 else a+(b-a)*u/x if u<x else b*(y-u)/(y-x) if u<y else 0
    rgba=(3<<24)|(4<<16)|(20<<8)|math.floor(alpha*255+.5);grad.append('{'+repr(u)+','+repr(alpha)+','+str(rgba)+'u}')
(o/'pause_layout_fixture.h').write_text('/* Verified v3 Chrome desktop measurements with current matching CSS. */\nstatic const PauseCase pause_cases[]={\n'+',\n'.join(rows)+'\n};\nstatic const GradientCase gradient_cases[]={\n'+',\n'.join(grad)+'\n};\n')
receipt={'package':str(package),'manifest_sha256':sha(package/'manifest.json'),'verified_files':[{'path':str(package/n),'sha256':sha(package/n)} for n in files],'current_sources':[{'path':str(p/'game/src/style.css'),'sha256':sha(p/'game/src/style.css')},{'path':str(p/'game/index.html'),'sha256':sha(p/'game/index.html')}],'selected_css_matches_v3':selected,'measured_states':[{'id':a['id'],'viewport':a['viewport'],'rect':a['observed']['rect']} for a in records],'gradient_cases':len(grad),'scope':'Reuses checksum-verified asset-team Chromium desktop/compact layout measurements; current relevant CSS matches v3. Gradient expectations derive from current CSS stops, not native implementation. No fresh browser raster, native render, asset copy or account/service work.'}
(o/'pause-layout-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps({'measured_states':len(records),'gradient_cases':len(grad),'verified_files':len(files)}))

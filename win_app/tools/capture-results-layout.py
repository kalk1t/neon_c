from pathlib import Path
import json,hashlib,re,sys
p=Path(r'C:\Users\User\Projects\neon_c\win_app');w=Path(sys.argv[1]) if len(sys.argv)>1 else Path.cwd();w.mkdir(parents=True,exist_ok=True);pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3')
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
manifest=json.loads((pack/'manifest.json').read_text()); verified=[]
for rel in ['assets.json','source/style.css']:
 f=pack/rel;assert sha(f)==next(a['sha256'] for a in manifest['assets'] if a['file']==rel);verified.append(dict(path=str(f),sha256=sha(f)))
cssfile=p.parent/'game/src/style.css';css=cssfile.read_text(encoding='utf-8');old=(pack/'source/style.css').read_text(encoding='utf-8');rules=[]
for selector in ['.menu-card.small {','.brand {','.brand.sm {','.res-cols {','#results > .menu-card {','#res-laps {','#results .brand.sm {','#res-total {','#res-place {','button {']:
 blocks=re.findall(re.escape(selector)+r'[^}]*}',css);assert blocks and all(b in old for b in blocks);rules+=blocks
# Correct the old nitro audit's unbounded end search, retaining historical evidence.
a=css.index('#nitro-call {');b=css.index('.boost.on i {',a);nitro=css[a:b];a2=old.index('#nitro-call {');b2=old.index('.boost.on i {',a2);assert nitro and nitro==old[a2:b2]
assets=json.loads((pack/'assets.json').read_text());entry=next(a for a in assets if a['id']=='results/panels/multiplayer-no-next' and a['scale']==1);obs=entry['observed'];children=obs['children'];expected=[children[i]['rect'] for i in [0,1,2,3,4,6,8,9]]
assert obs['rect']['width']==680 and obs['rect']['height']==491.1875
oracle=dict(version=json.loads((p.parent/'game/package.json').read_text())['version'],current_css=dict(path=str(cssfile),sha256=sha(cssfile)),package_manifest_sha256=sha(pack/'manifest.json'),verified=verified,rules=rules,sample_id=entry['id'],panel=obs['rect'],children=expected,nitro_audit_correction=dict(old_empty_slice_sha256='e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855',nonempty_selected_block_bytes=len(nitro.encode()),current_matches_packaged=True,corrected_block_sha256=hashlib.sha256(nitro.encode()).hexdigest()),scope='Reused prepared browser snapshot; no new browser/graphics execution. Layout for offline native placeholders uses this fixed source card; account/next-road behavior not imported.')
(w/'results-layout-oracle.json').write_text(json.dumps(oracle,indent=2)+'\n')
rows=['{'+','.join(str(c[k]) for k in ['x','y','width','height'])+'}' for c in expected]
(w/'results_layout_fixture.h').write_text('/* Checksum-verified prepared browser no-next results card, not a fresh render. */\nstatic const RRUIRect result_boxes[]={\n'+',\n'.join(rows)+'\n};\n')

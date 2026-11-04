from pathlib import Path
import json,re,hashlib,math,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3');out=p/'reference/ui-parity-20261103/results-card-capture';out.mkdir(exist_ok=True)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
manifest=json.loads((pack/'manifest.json').read_text(encoding='utf-8'));entries={a['file']:a for a in manifest['assets']}
verified=[]
def verify(rel):
 f=pack/rel;assert sha(f)==entries[rel]['sha256'],rel;verified.append(dict(file=rel,sha256=sha(f)));return f
assets=json.loads(verify('assets.json').read_text(encoding='utf-8'));oldcss=verify('source/style.css').read_text(encoding='utf-8');oldhtml=verify('source/index.html').read_text(encoding='utf-8')
cssfile=p.parent/'game/src/style.css';htmlfile=p.parent/'game/index.html';css=cssfile.read_text(encoding='utf-8');html=htmlfile.read_text(encoding='utf-8')
selectors=['* {','#menu, #pause, #results {','#pause, #results {','#menu, #results {','.menu-card {','.menu-card.small {','.menu-card.wide {','#results > .menu-card {','#res-laps {','#res-unlock.hidden, #res-next.hidden {']
rules=[]
for selector in selectors:
 blocks=re.findall(re.escape(selector)+r'[^}]*}',css);assert blocks and all(b in oldcss for b in blocks),selector;rules+=blocks
def result_block(s):return s[s.index('<div id="results"'):s.index('<div id="fatal"',s.index('<div id="results"'))]
assert result_block(html)==result_block(oldhtml),'Results DOM changed'
cases=[]
for e in assets:
 if not e['id'].startswith('results/panels/') or not e['observed'].get('rect'):continue
 assert e['kind']=='results-panel-reference' and len(e['observed']['children'])==10
 verify(e['reference']);verify(e['chrome']);obs=e['observed'];children=obs['children'];mask=0;heights=[]
 for i,c in enumerate(children):
  visible=c['styles']['display']!='none';mask|=(1<<i) if visible else 0;heights.append(c['rect']['height'])
 assert mask&863==863 and not mask&~1023
 total=48+sum(heights[i] for i in range(10) if mask&(1<<i))+9*(mask.bit_count()-1)
 assert abs(total-obs['rect']['height'])<.002,(e['id'],total,obs['rect'])
 y=24
 for i,c in enumerate(children):
  if not mask&(1<<i):assert c['rect']['width']==0 and c['rect']['height']==0;continue
  assert abs(c['rect']['x']-32)<.002 and abs(c['rect']['width']-616)<.002
  assert abs(c['rect']['y']-y)<.002,(e['id'],i,c['rect'],y);y+=heights[i]+9
 assert abs(obs['rect']['x']-(e['viewport']['width']-680)/2)<.002 and abs(obs['rect']['y']-max(16,(e['viewport']['height']-total)/2))<.002
 cases.append(dict(id=e['id'],scale=e['scale'],viewport=e['viewport'],mask=mask,heights=heights,panel=obs['rect'],boxes=[c['rect'] for c in children],note=e['note']))
assert len(cases)==20 and len({c['id'] for c in cases})==10 and {c['scale'] for c in cases}=={1,2}
oracle=dict(version=json.loads((p.parent/'game/package.json').read_text(encoding='utf-8'))['version'],current_sources=[dict(path='src/style.css',sha256=sha(cssfile)),dict(path='index.html',sha256=sha(htmlfile)),dict(path='package.json',sha256=sha(p.parent/'game/package.json'))],package=dict(path=str(pack),manifest_sha256=sha(pack/'manifest.json'),verified=verified),rules=rules,results_dom_sha256=hashlib.sha256(result_block(html).encode()).hexdigest(),cases=cases,scope='Reused checksum-verified prepared browser results states at their actual scales1/2 only; current relevant CSS rules and results DOM match supplied source. Inputs are observed child border-box heights/visibility; output panel/child positions are independently observed. No fresh browser/native/graphics render or real backend/progression state was produced. Scroll/resize extensions are CSS-rule-derived mathematical checks, not new browser observations.')
(out/'oracle.json').write_text(json.dumps(oracle,indent=2)+'\n',encoding='utf-8')
for name,data in [('current-results-css.txt','\n'.join(rules)),('current-results-dom.html',result_block(html))]:(out/name).write_text(data,encoding='utf-8')
if Path(__file__).resolve().parent.name=='roadsters-ui-audit':shutil.copy2(__file__,p/'tools/capture-results-card.py')
print(json.dumps(dict(version=oracle['version'],observed_states=10,observations=len(cases),verified_package_files=len(verified),current_rules_and_dom_match=True)))

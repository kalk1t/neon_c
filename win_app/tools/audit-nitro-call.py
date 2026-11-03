from pathlib import Path
import json,hashlib,zipfile
p=Path(r'C:\Users\User\Projects\neon_c');package=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersSharedAssets-v3');o=Path(__file__).parent/'nitro-call-fixture';o.mkdir(exist_ok=True)
sha=lambda b:hashlib.sha256(b).hexdigest()
manifest=json.loads((package/'manifest.json').read_text());entries={x['file']:x for x in manifest['assets']};verified=[]
with zipfile.ZipFile(package.parent/'RoadstersSharedAssets-v3-ui.zip') as z:
 for rel in ['assets.json','source/style.css']:
  b=(package/rel).read_bytes();e=entries[rel];assert len(b)==e['bytes'] and sha(b)==e['sha256'];matches=[n for n in z.namelist() if n==rel or n.endswith('/'+rel)];assert len(matches)==1 and z.read(matches[0])==b;verified.append({'file':rel,'sha256':sha(b)})
current=p/'game/src/style.css';css=current.read_text(encoding='utf-8');old=(package/'source/style.css').read_text(encoding='utf-8')
def nitro_block(s):return s[s.index('#nitro-call {'):s.index('.boost.on i {')].replace('\r\n','\n')
assert nitro_block(css)==nitro_block(old),'Nitro CSS changed from measured browser reference'
assets=json.loads((package/'assets.json').read_text());samples=[];rows=[];seen=set()
for item in assets:
 if item['id'] not in ['hud/callouts/nitro-135ms','hud/callouts/nitro-525ms'] or item['id'] in seen:continue
 seen.add(item['id']);style=item['observed']['styles'];matrix=list(map(float,style['transform'][7:-1].split(',')));origin=list(map(lambda s:float(s.removesuffix('px')),style['transformOrigin'].split()));height=origin[1]*2
 seconds=item['timelineMs']['nitro-call']/1000;sample={'id':item['id'],'seconds':seconds,'opacity':float(style['opacity']),'scale':matrix[0],'spacing':float(style['letterSpacing'].removesuffix('px')),'translation_y':matrix[5]/height*100,'source_matrix':style['transform'],'css_line_height':height};samples.append(sample);rows.append('{'+','.join(str(sample[k]) for k in ['seconds','opacity','scale','spacing','translation_y'])+'}')
assert len(samples)==2
(o/'nitro_call_fixture.h').write_text('/* Existing manifest-verified Chromium computed styles, not a fresh browser run. */\nstatic const NitroBrowserCase nitro_browser_cases[]={\n'+',\n'.join(rows)+'\n};\n')
receipt={'package':str(package),'manifest_sha256':sha((package/'manifest.json').read_bytes()),'selected_directory_and_archive_files_verified':verified,'current_css':str(current),'current_css_sha256':sha(current.read_bytes()),'nitro_css_block_sha256':sha(nitro_block(css).encode()),'source_css_block_matches_packaged_chromium_reference':True,'samples':samples,'scope':'Reused measured Chromium135/525ms computed styles from v3 package; current selected CSS block identical. No fresh graphics/browser run, intermediate CSS timing or native pixels not engine-verified.'}
(o/'nitro-call-oracle.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps({'samples':len(samples),'directory_and_archive_files':len(verified),'selected_css_identical':True}))

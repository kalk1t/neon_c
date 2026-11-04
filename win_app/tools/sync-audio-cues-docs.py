from pathlib import Path
import hashlib,json,re,datetime,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');project=p.parent;out=p/'reference/ui-parity-20261103/audio-cues-capture';dest=out/'docs-sync';dest.mkdir(exist_ok=True)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
previous=json.loads((p/'reference/ui-parity-20261103/results-binding-capture/browser-docs-sync.json').read_text(encoding='utf-8'));old={f['path']:f['sha256'] for f in previous['catalog']}
catalog=[]
for folder in [project/'game/docs',project/'techplan']:
 for f in sorted(folder.rglob('*.md')):catalog.append(dict(path=str(f.relative_to(project)),sha256=sha(f),bytes=f.stat().st_size))
changed=[f['path'] for f in catalog if f['path'] in old and f['sha256']!=old[f['path']]];added=[f['path'] for f in catalog if f['path'] not in old];removed=sorted(set(old)-{f['path'] for f in catalog})
selected=previous['selected_snapshots']
for rel in selected:
 f=project/rel;d=dest/rel;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(f,d)
sources=[dict(path=f,sha256=sha(project/f)) for f in ['game/package.json','game/src/audio.js','game/src/main.js','game/src/race.js','game/src/ai.js','game/src/roads.js','game/src/online/protocol.js']]
protocol=(project/'game/src/online/protocol.js').read_text(encoding='utf-8');version=json.loads((project/'game/package.json').read_text(encoding='utf-8'))['version'];assert re.search(r'export const LIVE_PROTOCOL = 10;',protocol)
components=(project/'techplan/04-components.md').read_text(encoding='utf-8');rows=[s for s in components.splitlines() if 'src/audio.js' in s or 'src/ai.js' in s]
findings=['Current local package version/protocol verified; deployment/live remote not checked.','Compare hashes to the prior42-file catalog, distinguishing unchanged/changed/added/removed instead of assuming chronology.','Source audio remains procedural; cue WAV exports do not replace continuous graph, cue guards/voice cap, timing or spatial behavior.','Source remains authoritative for stale AI component text if it still mentions road.ai/RIVAL_POWER; no browser-doc fixes in Windows scope.','Audio request recipes are independent of the separate nickname-wrap measurements and do not qualify audio playback or main activation.']
report=dict(created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_version=version,live_protocol_observed_in_source=10,catalog=catalog,compared_to='reference/ui-parity-20261103/results-binding-capture/browser-docs-sync.json',changed=changed,added=added,removed=removed,selected_snapshots=selected,source_checks=sources,review_rows=rows,findings=findings,scope='Read-only browser/docs/techplan inspection. Snapshots only in win_app; no Git/process/network/deployment/graphics/audio-device or upstream writes.')
(out/'browser-docs-sync.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(version=version,protocol=10,catalog=len(catalog),changed=changed,added=added,removed=removed,review_rows=rows)))

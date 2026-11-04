from pathlib import Path
import hashlib,json,re,datetime,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');project=p.parent;out=p/'reference/ui-parity-20261103/results-bound-flow-capture';dest=out/'docs-sync';dest.mkdir(exist_ok=False)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
previous=json.loads((p/'reference/ui-parity-20261103/audio-cues-capture/browser-docs-sync.json').read_text(encoding='utf-8'));old={f['path']:f['sha256']for f in previous['catalog']};catalog=[]
for folder in [project/'game/docs',project/'techplan']:
 for f in sorted(folder.rglob('*.md')):catalog.append(dict(path=str(f.relative_to(project)),sha256=sha(f),bytes=f.stat().st_size))
changed=[f['path']for f in catalog if f['path']in old and f['sha256']!=old[f['path']]];added=[f['path']for f in catalog if f['path']not in old];removed=sorted(set(old)-{f['path']for f in catalog})
selected=previous['selected_snapshots']
for rel in selected:
 f=project/rel;d=dest/rel;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(f,d)
paths=['game/package.json','game/src/race.js','game/src/main.js','game/src/style.css','game/index.html','game/src/ai.js','game/src/roads.js','game/src/audio.js','game/src/online/protocol.js'];sources=[dict(path=f,sha256=sha(project/f))for f in paths];before={f['path']:f['sha256']for f in previous['source_checks']};source_changes=[f['path']for f in sources if f['path']in before and f['sha256']!=before[f['path']]]
version=json.loads((project/'game/package.json').read_text(encoding='utf-8'))['version'];protocol=(project/'game/src/online/protocol.js').read_text(encoding='utf-8');m=re.search(r'export const LIVE_PROTOCOL = (\d+);',protocol);assert m
components=(project/'techplan/04-components.md').read_text(encoding='utf-8');rows=[x for x in components.splitlines()if 'src/race.js'in x or 'src/ai.js'in x or 'src/roads.js'in x]
report=dict(created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_version=version,live_protocol_observed_in_source=int(m[1]),catalog=catalog,compared_to='reference/ui-parity-20261103/audio-cues-capture/browser-docs-sync.json',changed=changed,added=added,removed=removed,selected_snapshots=selected,source_checks=sources,changed_previously_checked_sources=source_changes,review_rows=rows,findings=['Current local package/protocol only; no deployment/live remote observation.','Results source still defines frozen time/coin ordering, estimated prefix, first coin crown, source player styling and two-second presentation; binding adapter keeps these separate from account save/unlock behavior.','Known stale techplan AI row still mentions road.ai/RIVAL_POWER; source/changelog instead define RIVAL_SPEED/RIVAL_AIM and whole arsenal v0.2.16. No upstream correction is in Windows write scope.','No new priority inferred solely from catalog timestamps; compare exact hashes. Native host activation/slot sizing/graphics remain genuine qualification gaps.'],scope='Read-only browser/docs/techplan. Copies only within win_app; no Git/process/network/deployment/graphics/audio-device or upstream writes.')
(out/'browser-docs-sync.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(version=version,protocol=int(m[1]),catalog=len(catalog),changed=changed,added=added,removed=removed,changed_sources=source_changes,selected_snapshots=len(selected))))

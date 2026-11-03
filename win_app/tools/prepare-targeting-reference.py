from pathlib import Path
import json,sys
if len(sys.argv)!=2:raise SystemExit('Usage: prepare-targeting-reference.py STAGING_DIRECTORY')
s=Path(sys.argv[1]);j=json.loads((s/'targeting-reference.json').read_text())
def n(x):return format(x,'.17g')
def b(x):return str(bool(x)).lower()
def v(x):return '{'+','.join(map(n,x))+'}'
def target(t):return '{'+v(t['pos'])+','+v(t['vel'])+','+','.join(b(t[k]) for k in ['member','alive','finished'])+'}'
h='/* Actual source local targeting and late coin eligibility fixtures. */\nstatic const TargetGeometry geometry[]={\n'
for x in j['geometry']:h+='{'+v(x['player'])+','+v(x['aim'])+','+target(x['target'])+','+n(x['max'])+','+b(x['valid'])+','+n(x['value'])+'},\n'
h+='};\nstatic const LockAngles angle_cases[]={\n'
for x in j['angle_cases']:h+='{'+str(x['old'])+','+n(x['charge'])+','+n(x['dt'])+','+v(x['angles'])+','+b(x['keep_valid'])+','+n(x['keep_angle'])+','+str(x['target'])+','+n(x['after'])+','+str(x['events'])+'u},\n'
h+='};\nstatic const LockTrace traces[]={\n'+',\n'.join('{'+v(t['player'])+','+v(t['aim'])+'}' for t in j['traces'])+'\n};\nstatic const LockFrame frames[]={\n'
for i,t in enumerate(j['traces']):
 for x in t['frames']:h+='{'+str(i)+','+n(x['dt'])+',{'+','.join(map(target,x['targets']))+'},'+str(x['target'])+','+n(x['charge'])+','+str(x['events'])+'u},\n'
h+='};\nstatic const LeadFixture leads[]={\n'+',\n'.join('{'+v(x['from'])+','+target(x['target'])+','+n(x['speed'])+','+v(x['expected'])+'}' for x in j['leads'])+'\n};\nstatic const PhaseFixture phases[]={\n'+',\n'.join('{'+str(x['action'])+','+str(x['collections'])+','+str(x['coins'])+','+b(x['active'])+','+n(x['timer'])+','+n(x['pop'])+'}' for x in j['phase_cases'])+'\n};\n'
(s/'generated_targeting_reference.h').write_text(h);print('Prepared geometry, exact angle-branch, coupled lock, lead and late-coin fixtures.')

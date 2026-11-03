from pathlib import Path
import json,sys
if len(sys.argv)!=2:raise SystemExit('Usage: prepare-coins-reference.py STAGING_DIRECTORY')
s=Path(sys.argv[1]);j=json.loads((s/'coin-reference.json').read_text())
def n(x):return format(x,'.17g')
def b(x):return str(bool(x)).lower()
def v(x):return '{'+','.join(map(n,x))+'}'
def state(x):return '{'+str(len(x['list']))+','+b(x['hidden'])+',{'+','.join('{'+b(c['active'])+','+n(c['timer'])+','+n(c['pop'])+'}' for c in x['list'])+'}}'
def racer(x):return '{'+v(x['pos'])+','+v(x['fwd'])+','+str(x['coins'])+','+b(x['alive'])+','+b(x['finished'])+'}'
def result(x):return '{{'+(','.join('{'+str(c['coin'])+','+str(c['racer'])+'}' for c in x['events']) if x['events'] else '{0}')+'},'+str(len(x['events']))+'}'
header='/* Actual source placement, selection, pickup/respawn/pop and visibility rules. */\nstatic const CoinLayout layouts[]={\n'
for x in j['layouts']:header+='{'+str(x['road'])+','+str(len(x['list']))+',{'+','.join('{'+v(c['pos'])+','+str(c['sample'])+'}' for c in x['list'])+'}},\n'
header+='};\nstatic const CoinAhead ahead_cases[]={\n'
for x in j['ahead']:header+='{'+str(x['road'])+','+str(x['inactive'])+','+racer(x['racer'])+','+n(x['max'])+','+str(x['best'])+'},\n'
header+='};\nstatic const CoinUpdate update_cases[]={\n'
for x in j['updates']:header+='{'+str(x['road'])+','+state(x['before'])+','+str(len(x['racers']))+',{'+','.join(map(racer,x['racers']))+'},'+n(x['dt'])+','+b(x['active'])+','+state(x['after'])+',{'+','.join(map(racer,x['after_racers']))+'},'+result(x)+'},\n'
header+='};\nstatic const CoinReset reset_cases[]={\n'
for x in j['resets']:header+='{'+str(x['road'])+','+state(x['before'])+','+state(x['after'])+'},\n'
header+='};\nstatic const CoinTrace traces[]={\n'
for x in j['traces']:header+='{'+str(x['road'])+','+state(x['before'])+'},\n'
header+='};\nstatic const CoinFrame frames[]={\n'
for index,x in enumerate(j['traces']):
 for f in x['frames']:header+='{'+str(index)+','+n(f['dt'])+','+b(f['active'])+',{'+','.join(map(racer,f['racers']))+'},'+state(f['after'])+',{'+','.join(map(racer,f['after_racers']))+'},'+result(f)+'},\n'
header+='};\nstatic const CoinTie tie_cases[]={'+','.join('{'+n(x['max'])+','+str(x['best'])+'}' for x in j['synthetic_ahead'])+'};\n'
(s/'generated_coin_reference.h').write_text(header);print('Prepared compact source fixture for756 placements,324 ahead,108 updates and1440 replay frames.')

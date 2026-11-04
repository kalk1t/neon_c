from pathlib import Path
import json,sys
if len(sys.argv)!=2:raise SystemExit('Usage: prepare-pack-reference.py STAGING_DIRECTORY')
s=Path(sys.argv[1]);j=json.loads((s/'pack-reference.json').read_text())
def n(x):return format(x,'.17g')
def b(x):return str(bool(x)).lower()
def v(x):return '{'+','.join(map(n,x))+'}'
def rs(x):
 vals=[v(x[k]) for k in ['pos','vel','fwd']]+[n(x[k]) for k in ['yaw','pitch','roll','speed','boost','max','stun']]+[b(x['boosting']),str(x['sample']),n(x['dodge'])]+[str(x[k]) for k in ['lap','next','lap_count']]+[n(x[k]) for k in ['lap_time','finish_time','hp','wreck','lost','stuck']]+[str(x['coins']),b(x['finished']),b(x['visible']),str(x['passed'])+'u']
 return '{'+','.join(vals)+','+','.join(n(x[k]) for k in ['fire_cd','react','smoke'])+','+v(x['resources'])+'}'
def rng(draws):
 assert len(draws)%2==0
 pairs=[]
 for i in range(0,len(draws),2):a,c=draws[i:i+2];assert a['owner']==c['owner'] and a['owner']>0;pairs.append('{'+str(a['owner'])+','+v([a['value'],c['value']])+'}')
 return '{'+str(len(pairs))+',{'+(','.join(pairs) if pairs else '{0}')+'}}'
def meta(x):return '{'+v(x['ranking'])+','+str(x['position'])+','+str(x['finish_position'])+','+n(x['rank_timer'])+','+n(x['shake'])+'}'
def events(x):return '{{'+(','.join('{'+str(e['coin'])+','+str(e['racer'])+'}' for e in x) if x else '{0}')+'},'+str(len(x))+'}'
header='/* Actual source post-player pack pass; no combat/world/account/graphics. */\nstatic const PackTrace traces[]={\n'
for t in j['traces']:
 a=t['start'];drivers='{'+','.join('{0,'+','.join(n(d[k]) for k in ['seed','lane','lane_y','dodge','skill'])+'}' for d in t['initial_drivers'])+'}'
 header+='{'+str(t['road'])+','+drivers+','+rs(a['player'])+','+rng(a['draws'])+',{'+','.join(map(rs,a['after']))+'},'+meta(a['stats'])+'},\n'
header+='};\nstatic const PackFrame frames[]={\n'
ticks=starts=0
for index,t in enumerate(j['traces']):
 for f in t['frames']:
  if f['restart']:
   starts+=1;header+='{.trace='+str(index)+',.restart=true,.player='+rs(f['player'])+',.random='+rng(f['draws'])+',.after={'+','.join(map(rs,f['after']))+'},.meta='+meta(f['stats'])+'},\n'
  else:
   ticks+=1;coins='{'+','.join('{'+b(c[0])+','+n(c[1])+','+n(c[2])+'}' for c in f['coins'])+'}'
   header+='{.trace='+str(index)+',.branch='+str(f['branch'])+',.dt='+n(f['dt'])+',.time='+n(f['stats']['time'])+',.race_time='+n(f['race_time'])+',.mask='+str(f['mask'])+'u,.player='+rs(f['player'])+',.input_shake='+n(f['player']['shake'])+',.random='+rng(f['draws'])+',.after={'+','.join(map(rs,f['after']))+'},.after_player='+rs(f['after_player'])+',.coin_count='+str(len(f['coins']))+',.coins='+coins+',.events='+events(f['events'])+',.notice_count='+str(len(f['notices']))+',.notices={'+(','.join('{'+str(a['index'])+','+v(a['pos'])+','+','.join(n(a[k]) for k in ['speed','hp','wreck','smoke'])+','+str(a['draws'])+','+b(a['smoked'])+'}' for a in f['notices']) if f['notices'] else '{0}')+'},.meta='+meta(f['stats'])+'},\n'
header+='};\n';(s/'pack_current_reference.h').write_text(header);print(json.dumps({'ticks':ticks,'restarts':starts,'header_bytes':(s/'pack_current_reference.h').stat().st_size}))

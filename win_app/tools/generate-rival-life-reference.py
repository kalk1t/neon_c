from pathlib import Path
import json,sys
if len(sys.argv)!=2:raise SystemExit('Usage: prepare-rival-reference.py STAGING_DIRECTORY')
s=Path(sys.argv[1]);j=json.loads((s/'rival-reference.json').read_text())
def num(x):return format(x,'.17g')
def vec(x):return '{'+','.join(map(num,x))+'}'
def boolean(x):return str(bool(x)).lower()
def car(c):
 parts=['.'+k+'='+vec(c[k]) for k in ['pos','vel','fwd']]
 parts+=['.'+k+'='+num(c.get(k,0)) for k in ['yaw','pitch','roll','speed','boost','t','stun','roll_time','surge','max_multiplier','thrust','nitro','kick','sputter','impact','roll_direction']]
 return '{'+','.join(parts)+',.boosting='+boolean(c['boosting'])+'}'
def state(r):
 ai=r['ai'];race={'.mode':'RR_RACE_TWO_LAPS','.lap':num(r['lap']),'.next_checkpoint':num(r['next']),'.lap_time':num(r['lap_time']),'.completed_laps':num(len(r['lap_times'])),'.finish_time':num(r['finish_time']),'.safe_position':vec(r['safe']),'.safe_yaw':num(r['safe_yaw']),'.finished':boolean(r['finished'])}
 a=['.car='+car(r['car']),'.ai='+vec([ai[k] for k in ['sample','seed','lane','lane_y','dodge','skill']]),'.race={'+','.join(k+'='+v for k,v in race.items())+'}', '.previous='+vec(r['previous'])]
 a+=['.'+k+'='+num(r[k]) for k in ['hp','wreck','lost','stuck','wrong_way','smoke_time','fire_cd','react','takedowns','coins','passed']]
 a+=['.lap_count='+str(len(r['lap_times'])),'.is_player='+boolean(r['is_player']),'.visible='+boolean(r['visible'])]
 return '{{'+','.join(a)+'},'+vec(r['lap_times'] or [0])+','+vec(r['resources'])+'}'
def inp(x):return '{'+','.join(num(x.get(k,0)) for k in ['throttle','brake','steer','climb'])+','+boolean(x.get('boost'))+',0,false}'
out='/* Actual local browser reset/respawn/rule/step/contact outputs. */\nstatic const RivalResetFixture rival_resets[]={\n'
for f in j['resets']:out+='{'+str(f['road'])+','+state(f['before'])+','+vec(f['position'])+','+num(f['yaw'])+','+vec(f['draws'])+','+state(f['after'])+'},\n'
out+='};\nstatic const RivalRespawnFixture rival_respawns[]={\n'
for f in j['respawns']:out+='{'+str(f['road'])+','+state(f['before'])+','+boolean(f['keep_hp'])+','+vec(f['draws'])+','+state(f['after'])+'},\n'
out+='};\nstatic const RivalRulesFixture rival_rules[]={\n'
for f in [*j['rules'],*[dict(frame,road=trace['road']) for trace in j['laps'] for frame in trace['frames']]]:out+='{'+str(f['road'])+','+state(f['before'])+','+num(f['dt'])+','+num(f['time'])+','+vec(f.get('draws',[.2,.8]))+','+state(f['after'])+','+str(f['spawns'])+'},\n'
out+='};\nstatic const RivalStepTrace rival_traces[]={\n'
for t in j['steps']:out+='{'+str(t['road'])+','+state(t['before'])+'},\n'
out+='};\nstatic const RivalStepFixture rival_steps[]={\n'
for index,t in enumerate(j['steps']):
 for f in t['frames']:out+='{'+str(index)+','+num(f['dt'])+','+num(f['time'])+','+inp(f['input'])+','+state(f['after'])+','+str(f['spawns'])+','+str(f['smokes'])+','+boolean(f['ai'])+'},\n'
out+='};\nstatic const RivalContactFixture rival_contacts[]={\n'
for f in j['contacts']:
 events=[]
 for x in f['calls']:
  player=f['before'][x['a']]['is_player'] or f['before'][x['b']]['is_player'];hit=player and x['relative_speed']< -5
  events.append('{'+str(x['a'])+','+str(x['b'])+','+num(x['relative_speed'])+','+num(-x['relative_speed']*.6 if hit else 0)+','+boolean(hit)+'}')
 out+='{'+str(len(f['before']))+',{'+','.join(map(state,f['before']))+'},{'+','.join(map(state,f['after']))+'},'+num(f['shake'])+','+num(f['after_shake'])+','+str(len(events))+',{'+(','.join(events) if events else '{0}')+'},'+str(len(f['hits']))+','+vec(f['hits'] or [0])+'},\n'
out+='};\nstatic const GateFixture gate_probes[]={\n'
for g in j['gates']:out+='{'+boolean(g['player'])+','+boolean(g['cross'])+','+num(g['radius'])+','+str(g['next'])+','+vec(g['safe'])+','+num(g['yaw'])+','+num(g['boost'])+'},\n'
out+='};\n';(s/'rival_life_reference.h').write_text(out);print('Prepared actual-source reset/respawn/rules,step and 11-car contact fixtures.')

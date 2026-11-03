from pathlib import Path
import json,sys
s=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).parent
d=json.loads((s/'combat-reference.json').read_text())
def n(x):return ('true' if x else 'false') if isinstance(x,bool) else repr(x)
def arr(x):return '{'+','.join(arr(v) if isinstance(v,list) else n(v) for v in x)+'}'
def actor(x):return arr(x)
def shot(x):return arr([x['owner'],x['target'],x['pos'],x['vel'],x['speed'],x['accel'],x['max_speed'],x['life'],x['turn'],x['damage'],x['radius'],x['trail_time'],x['kind'],x['always'],x['trail'],x['has_damage'],x['serial']])
text='''/* Actual source fixtures; generated, do not edit. */
typedef struct {RRVec3 pos,vel;double speed,stun,roll,surge,yaw,hp,wreck,fire_cd;bool finished,player,visible;} ActorFixture;
typedef struct {ActorFixture before;double amount;int kind;bool active;ActorFixture after;unsigned flags;} DamageFixture;
typedef struct {ActorFixture before;double amount;bool has_damage;ActorFixture after;unsigned flags;} DefaultFixture;
typedef struct {ActorFixture before[4],after[4];unsigned count;unsigned notices[4][2];unsigned notice_count;} EmpFixture;
typedef struct {int side;double speed,yaw,dt,immediately[7],after[2];bool dodging;} DodgeFixture;
typedef struct {int owner,target;RRVec3 pos,vel;double speed,accel,max_speed,life,turn,damage,radius,trail_time;int kind;bool always,trail,has_damage;unsigned serial;} ShotFixture;
typedef struct {unsigned event,serial;int victim;RRVec3 point;} EventFixture;
typedef struct {unsigned ns,nr;ShotFixture before[5];ActorFixture actors[4];double dt;bool world_hit;double floor;bool purge;unsigned purged,world_calls,after_count;ShotFixture after[5];ActorFixture after_actors[4];unsigned event_count;EventFixture events[12];} StepFixture;
'''
def block(name,typ,rows):return 'static const '+typ+' '+name+'[]={\n'+',\n'.join('{'+','.join(row)+'}' for row in rows)+'\n};\n'
text+=block('damage_fixtures','DamageFixture',[[actor(c['before']),n(c['amount']),n(c['kind']),n(c['active']),actor(c['after']),n(c['flags'])] for c in d['damages']])
text+=block('default_fixtures','DefaultFixture',[[actor(c['before']),n(c['amount']),n(c['has_damage']),actor(c['after']),n(c['flags'])] for c in d['defaults']])
text+=block('emp_fixtures','EmpFixture',[[arr(c['before']),arr(c['after']),n(c['count']),arr(c['notices']) if c['notices'] else '{{0}}',n(len(c['notices']))] for c in d['emps']])
text+=block('dodge_fixtures','DodgeFixture',[[n(c['side']),n(c['speed']),n(c['yaw']),n(c['dt']),arr(c['immediately']),arr(c['after']),n(c['dodging'])] for c in d['dodges']])
text+=block('step_fixtures','StepFixture',[[n(len(c['before'])),n(len(c['actors'])),'{'+','.join(map(shot,c['before']))+'}',arr(c['actors']),n(c['dt']),n(c['worldHit']),n(c['floor']),n(c['purge']),n(c['purged']),n(c['worldCalls']),n(len(c['after'])),'{'+','.join(map(shot,c['after']))+'}' if c['after'] else '{{0}}',arr(c['afterActors']),n(len(c['events'])),'{'+','.join(arr([e['event'],e['serial'],e['victim'],e['point']]) for e in c['events'])+'}' if c['events'] else '{{0}}'] for c in d['shots']])
(s/'generated_combat_reference.h').write_text(text)
print('Generated damage, EMP, dodge and projectile fixtures')

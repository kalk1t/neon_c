from pathlib import Path
import json,sys
s=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).parent;d=json.loads((s/'fire-reference.json').read_text())
def n(x):return ('true' if x else 'false') if isinstance(x,bool) else repr(x)
def arr(x):return '{'+','.join(arr(v) if isinstance(v,list) else n(v) for v in x)+'}'
text='''/* Generated actual-source fire/FX/sequence fixtures. */
typedef struct {RRVec3 pos,vel;double speed,stun,roll,surge,yaw,hp,wreck,fire_cd;bool finished,player,visible;} ActorFixture;
typedef struct {int owner,target;RRVec3 pos,vel;double speed,accel,max_speed,life,turn,damage,radius,trail_time;int kind;bool always,trail,has_damage;} ShotFixture;
typedef struct {int owner;double inherit;int target;RRVec3 dir;RRFireOptions options;ShotFixture expected;double length,width;unsigned draws;} InitFixture;
typedef struct {size_t initial;unsigned kind,seed,draws,rng;size_t count;RRFxLife fx[313];} FxFixture;
typedef struct {bool first;unsigned seed;double dt;bool restart,inject,smoke,world_hit,active;bool fire,missile,spin,emp;double steer;unsigned rng,draws,world_calls;RRArsenal arsenal;int lock;double charge;ActorFixture actors[3];size_t shot_count;ShotFixture shots[16];size_t fx_count;RRFxLife fx[41];} FrameFixture;
'''
def block(name,typ,rows):return 'static const '+typ+' '+name+'[]={\n'+',\n'.join('{'+','.join(r)+'}' for r in rows)+'\n};\n'
text+=block('init_fixtures','InitFixture',[[n(c['owner']),n(c['inherit']),n(c['target']),arr(c['dir']),arr(c['options']),arr(c['expected']),n(c['length']),n(c['width']),n(c['draws'])] for c in d['fires']])
text+=block('fx_fixtures','FxFixture',[[n(c['initial']),n(c['kind']),n(c['seed']),n(c['draws']),n(c['rng']),n(len(c['fx'])),arr(c['fx'])] for c in d['fxCases']])
rows=[]
for t in d['traces']:
 for i,f in enumerate(t['frames']):
  rows.append([n(i==0),n(t['seed']),n(f['dt']),*[n(f[k]) for k in ['restart','inject','smoke','worldHit','active']],*[n(v) for v in f['input']],n(f['rng']),n(f['draws']),n(f['worldCalls']),arr(f['arsenal']),n(f['lock']),n(f['charge']),arr(f['actors']),n(len(f['shots'])),arr(f['shots']) if f['shots'] else '{{0}}',n(len(f['fx'])),arr(f['fx']) if f['fx'] else '{{0}}'])
text+=block('frame_fixtures','FrameFixture',rows)
(s/'generated_fire_reference.h').write_text(text);print('Generated96 fire,35 FX,480 seeded frame fixtures')

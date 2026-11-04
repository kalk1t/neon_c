from pathlib import Path
import json,sys
directory=Path(sys.argv[1]);d=json.loads((directory/'current-fire-reference.json').read_text(encoding='utf-8'))
def n(v):return ('true' if v else 'false') if isinstance(v,bool) else repr(v)
def arr(v):return '{'+','.join(arr(x) if isinstance(x,list) else n(x) for x in v)+'}'
max_shots=d['summary']['maxShots'];max_fx=d['summary']['maxFx'];assert max_shots<=64 and max_fx<=512
text='''/* Current actual-source firing sequences; warm resources/no-audio-context, no rewards. */
typedef struct {RRVec3 pos,vel;double speed,stun,roll,surge,yaw,hp,wreck,fire_cd;bool finished,player,visible;} ActorFixture;
typedef struct {int owner,target;RRVec3 pos,vel;double speed,accel,max_speed,life,turn,damage,radius,trail_time;int kind;bool always,trail,has_damage;double length,width,flown;RRVec3 color;bool painted;} ShotFixture;
typedef struct {RRVec3 position,color;unsigned draws,rng;} FlashFixture;
typedef struct {bool first;unsigned seed;double dt;int model;unsigned accent;double model_world[16],yaw;bool restart,inject,smoke,world_hit,active;bool fire,missile,spin,emp;double steer;unsigned rng,draws,world_calls;RRArsenal arsenal;int lock;double charge;ActorFixture actors[3];size_t shot_count;ShotFixture shots[SHOTS];size_t fx_count;RRFxLife fx[FX];size_t flash_count;FlashFixture flashes[1];} FrameFixture;
'''.replace('shots[SHOTS]',f'shots[{max_shots}]').replace('fx[FX]',f'fx[{max_fx}]')
rows=[]
for trace in d['traces']:
 for i,f in enumerate(trace['frames']):
  row=[n(i==0),n(trace['seed']),n(f['dt']),n(f['model']),n(f['accent']),arr(f['modelWorld']),n(f['yaw']),*[n(f[k]) for k in ['restart','inject','smoke','worldHit','active']],*[n(v) for v in f['input']],n(f['rng']),n(f['draws']),n(f['worldCalls']),arr(f['arsenal']),n(f['lock']),n(f['charge']),arr(f['actors']),n(len(f['shots'])),arr(f['shots']) if f['shots'] else '{{0}}',n(len(f['fx'])),arr(f['fx']) if f['fx'] else '{{0}}',n(len(f['flashes'])),arr(f['flashes']) if f['flashes'] else '{{0}}']
  rows.append('{'+','.join(row)+'}')
text+='static const FrameFixture current_fire_frames[]={\n'+',\n'.join(rows)+'\n};\n';(directory/'current_fire_reference.h').write_text(text,encoding='utf-8');print({'frames':len(rows),'max_shots':max_shots,'max_fx':max_fx})

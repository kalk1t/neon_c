from pathlib import Path
import json,sys
s=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).parent;d=json.loads((s/'run-reference.json').read_text())
def n(x):
 if x is None:return '0'
 return ('true' if x else 'false') if isinstance(x,bool) else repr(x)
def arr(x):return '{'+','.join(arr(v) if isinstance(v,list) else n(v) for v in x)+'}'
def state(x):return '{'+','.join([arr(x[k]) for k in ['pos','vel','fwd']]+[n(x.get(k,0)) for k in ['yaw','pitch','roll','speed','boost','max','stun','t','roll_time','surge']]+[n(x.get('roll_direction',1)),n(x['boosting']),n(x['sample']),n(x['dodge'])]+[n(x[k]) for k in ['lap','next','lap_count','lap_time','finish_time','hp','wreck','lost','stuck','coins','finished','visible','passed']]+[n(x.get(k,0)) for k in ['fire_cd','react','smoke','wrong_way']]+[arr(x['previous']),arr(x['safe_pos']),n(x['safe_yaw']),arr(x['lap_times']) if x['lap_times'] else '{0}'])+'}'
def states(xs):return '{'+','.join(map(state,xs))+'}'
phases={'countdown':3,'race':1,'paused':2,'finished':4}
def meta(m):return '{'+','.join([arr(m['ranking']),*[n(m[k]) for k in ['position','finish_position','rank_timer','shake','time','last_nitro','countdown','last_count']],n(phases[m['phase']])])+'}'
def reset(x):return '{'+','.join([states(x['after']),n(x['rng']),n(x['draws']),meta(x['meta'])])+'}'
text='''/* Actual full local CPU frame, with processed control boundary. */
typedef struct {RRVec3 pos,vel,fwd;double yaw,pitch,roll,speed,boost,max,stun,t,roll_time,surge;int roll_dir;bool boosting;int sample;double dodge;int lap,next,lap_count;double lap_time,finish_time,hp,wreck,lost,stuck;int coins;bool finished,visible;uint32_t passed;double fire_cd,react,smoke,wrong_way;RRVec3 previous,safe_pos;double safe_yaw,history[2];} State;
typedef struct {int ranking[11],position,finish_position;double rank_timer,shake,time,last_nitro,countdown;int last_count,phase;} Meta;
typedef struct {State after[11];unsigned rng,draws;Meta meta;} ResetFixture;
typedef struct {int road;unsigned seed;double drivers[10][5];ResetFixture start;} TraceFixture;
typedef struct {int owner,target;RRVec3 pos,vel;double speed,accel,max_speed,life,turn,damage,radius,trail_time;int kind;bool always,trail,has_damage;} ShotFixture;
typedef struct {bool active;double timer,pop;} CoinFixture;
typedef struct {unsigned trace;bool restart;ResetFixture reset;unsigned mask;double raw;int entered;RRInput control;bool fire,missile,spin,emp;double steer;bool manual;RRVec3 aim;State after[11];unsigned rng,draws;double race_time;Meta meta;RRArsenal arsenal;int lock;double charge;size_t shot_count;ShotFixture shots[8];size_t fx_count;RRFxLife fx[58];int coin_count;CoinFixture coins[RR_COIN_MAX];int collection_count;int collections[RR_COIN_MAX][2];RRCarStepReport motion;unsigned segment_calls,segment_hits;} FrameFixture;
'''
def block(name,typ,rows):return 'static const '+typ+' '+name+'[]={\n'+',\n'.join('{'+','.join(row)+'}' for row in rows)+'\n};\n'
text+=block('traces','TraceFixture',[[n(t['road']),n(t['seed']),arr(t['drivers']),reset(t['start'])] for t in d['traces']])
rows=[]
for ti,t in enumerate(d['traces']):
 for f in t['frames']:
  rows.append([n(ti),n(f['reset'] is not None),reset(f['reset']) if f['reset'] else '{0}',n(f['mask']),n(f['raw']),n(phases[f['entered']]),arr(f['control']),*[n(v) for v in f['weapons']],n(f['manual']),arr(f['aim']),states(f['after']),n(f['rng']),n(f['draws']),n(f['race_time']),meta(f['meta']),arr(f['arsenal']),n(f['lock']),n(f['charge']),n(len(f['shots'])),arr(f['shots']) if f['shots'] else '{{0}}',n(len(f['fx'])),arr(f['fx']) if f['fx'] else '{{0}}',n(len(f['coins'])),arr(f['coins']),n(len(f['coinEvents'])),arr(f['coinEvents']) if f['coinEvents'] else '{{0}}','{'+','.join([n(f['motion']['impact']),arr(f['motion']['normal']),n(f['motion']['collided'])])+'}',n(f['segmentCalls']),n(f['segmentHits'])])
text+=block('frames','FrameFixture',rows)
(s/'generated_run_reference.h').write_text(text);print('Generated640 full local simulation tick fixtures')

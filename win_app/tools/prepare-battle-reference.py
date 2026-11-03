from pathlib import Path
import json,sys
s=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).parent;d=json.loads((s/'battle-reference.json').read_text())
def n(x):
 if x is None:return '0'
 return ('true' if x else 'false') if isinstance(x,bool) else repr(x)
def arr(x):return '{'+','.join(arr(v) if isinstance(v,list) else n(v) for v in x)+'}'
def state(x):return '{'+','.join([arr(x[k]) for k in ['pos','vel','fwd']]+[n(x.get(k,0)) for k in ['yaw','pitch','roll','speed','boost','max','stun','t','roll_time','surge']]+[n(x.get('roll_direction',1)),n(x['boosting']),n(x['sample']),n(x['dodge'])]+[n(x[k]) for k in ['lap','next','lap_count','lap_time','finish_time','hp','wreck','lost','stuck','coins','finished','visible','passed']]+[n(x.get(k,0)) for k in ['fire_cd','react','smoke']])+'}'
def states(xs):return '{'+','.join(map(state,xs))+'}'
def meta(m):return '{'+','.join([arr(m['ranking']),n(m['position']),n(m['rank_timer']),n(m['shake']),n(m['time'])])+'}'
def reset(x):return '{'+','.join([state(x['player']),states(x['after']),n(x['rng']),n(x['draws']),meta(x['meta'])])+'}'
text='''/* Actual source post-player 11-car coupled combat reference. */
typedef struct {RRVec3 pos,vel,fwd;double yaw,pitch,roll,speed,boost,max,stun,t,roll_time,surge;int roll_dir;bool boosting;int sample;double dodge;int lap,next,lap_count;double lap_time,finish_time,hp,wreck,lost,stuck;int coins;bool finished,visible;uint32_t passed;double fire_cd,react,smoke;} BattleState;
typedef struct {int ranking[11],position;double rank_timer,shake,time;} Meta;
typedef struct {BattleState player,after[11];unsigned rng,draws;Meta meta;} ResetFixture;
typedef struct {int road;unsigned seed;double drivers[10][5];ResetFixture start;} TraceFixture;
typedef struct {int owner,target;RRVec3 pos,vel;double speed,accel,max_speed,life,turn,damage,radius,trail_time;int kind;bool always,trail,has_damage;} ShotFixture;
typedef struct {bool active;double timer,pop;} CoinFixture;
typedef struct {unsigned trace;bool restart;ResetFixture reset;unsigned mask;RRPackBranch branch;double dt,race_time;BattleState before;RRVec3 muzzle,side;bool fire,missile,spin,emp;double steer;BattleState after[11];unsigned rng,draws;Meta meta;RRArsenal arsenal;int lock;double charge;size_t shot_count;ShotFixture shots[17];size_t fx_count;RRFxLife fx[112];int coin_count;CoinFixture coins[RR_COIN_MAX];int collection_count;int collections[RR_COIN_MAX][2];} FrameFixture;
typedef struct {int road;unsigned seed;BattleState before[3];bool active;double time,after_cd;size_t shot_count;ShotFixture shots[1];unsigned draws,rng;} RivalFixture;
'''
def block(name,typ,rows):return 'static const '+typ+' '+name+'[]={\n'+',\n'.join('{'+','.join(row)+'}' for row in rows)+'\n};\n'
text+=block('trace_fixtures','TraceFixture',[[n(t['road']),n(t['seed']),arr(t['drivers']),reset(t['start'])] for t in d['traces']])
rows=[]
for ti,t in enumerate(d['traces']):
 for f in t['frames']:
  rows.append([n(ti),n(f['reset'] is not None),reset(f['reset']) if f['reset'] else '{0}',n(f['mask']),n(f['branch']),n(f['dt']),n(f['race_time']),state(f['before']),arr(f['muzzle']),arr(f['side']),*[n(v) for v in f['input']],states(f['after']),n(f['rng']),n(f['draws']),meta(f['meta']),arr(f['arsenal']),n(f['lock']),n(f['charge']),n(len(f['shots'])),arr(f['shots']) if f['shots'] else '{{0}}',n(len(f['fx'])),arr(f['fx']) if f['fx'] else '{{0}}',n(len(f['coins'])),arr(f['coins']),n(len(f['coinEvents'])),arr(f['coinEvents']) if f['coinEvents'] else '{{0}}'])
text+=block('frame_fixtures','FrameFixture',rows)
text+=block('rival_fixtures','RivalFixture',[[n(f['road']),n(f['seed']),states(f['before']),n(f['active']),n(f['time']),n(f['after_cd']),n(len(f['shots'])),arr(f['shots']) if f['shots'] else '{{0}}',n(f['draws']),n(f['rng'])] for f in d['rivalProbes']])
(s/'generated_battle_reference.h').write_text(text);print('Generated720 coupled frames and192 rival probes')

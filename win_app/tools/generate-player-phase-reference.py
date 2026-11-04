from pathlib import Path
import json,sys
s=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).parent;d=json.loads((s/'battle-reference.json').read_text())
def n(x):
 if x is None:raise ValueError('Undefined or nonfinite source value')
 return ('true' if x else 'false') if isinstance(x,bool) else repr(x)
def arr(x):return '{'+','.join(arr(v) if isinstance(v,list) else n(v) for v in x)+'}'
def state(x):return '{'+','.join([arr(x[k]) for k in ['pos','vel','fwd']]+[n(x.get(k,0)) for k in ['yaw','pitch','roll','speed','boost','max','stun','t','roll_time','surge']]+[n(x.get('roll_direction',1)),n(x['boosting']),n(x['sample']),n(x['dodge'])]+[n(x[k]) for k in ['lap','next','lap_count','lap_time','finish_time','hp','wreck','lost','stuck','coins','finished','visible','passed']]+[n(x.get(k,0)) for k in ['fire_cd','react','smoke']]+[arr(x['resources'])])+'}'
def states(xs):return '{'+','.join(map(state,xs))+'}'
def meta(m):return '{'+','.join([arr(m['ranking']),n(m['position']),n(m['rank_timer']),n(m['shake']),n(m['time'])])+'}'
def reset(x):return '{'+','.join([state(x['player']),states(x['after']),n(x['rng']),n(x['draws']),meta(x['meta'])])+'}'
assert max(len(f['shots']) for t in d['traces'] for f in t['frames'])<=64 and max(len(f['fx']) for t in d['traces'] for f in t['frames'])<=512
text='''/* Actual source post-player 11-car coupled combat reference. */
typedef struct {double heat,bolt_cd;int side;bool overheated;double missiles,seeker_cd,spin_cd,emp_cd;int lock;double lock_time;} Resource;
typedef struct {RRVec3 pos,vel,fwd;double yaw,pitch,roll,speed,boost,max,stun,t,roll_time,surge;int roll_dir;bool boosting;int sample;double dodge;int lap,next,lap_count;double lap_time,finish_time,hp,wreck,lost,stuck;int coins;bool finished,visible;uint32_t passed;double fire_cd,react,smoke;Resource resources;} BattleState;
typedef struct {int ranking[11],position;double rank_timer,shake,time;} Meta;
typedef struct {BattleState player,after[11];unsigned rng,draws;Meta meta;} ResetFixture;
typedef struct {int road;unsigned seed;double drivers[10][5];ResetFixture start;} TraceFixture;
typedef struct {int owner,target;RRVec3 pos,vel;double speed,accel,max_speed,life,turn,damage,radius,trail_time;int kind;bool always,trail,has_damage,dodge_tried;double length,width,flown;RRVec3 color;bool painted;} ShotFixture;
typedef struct {bool active;double timer,pop;} CoinFixture;
typedef struct {bool valid;int model;unsigned accent;double matrix[16];} Binding;
typedef struct {double wrong;RRVec3 previous,safe;double yaw;size_t count;double history[8];} PlayerExtra;
typedef struct {unsigned trace;bool restart;ResetFixture reset;unsigned mask;RRPackBranch branch;double dt,race_time;BattleState before;RRVec3 muzzle,side;RRInput control;Binding bindings[11];bool active;bool fire,missile,spin,emp;double steer;BattleState after[11];unsigned rng,draws;Meta meta;RRArsenal arsenal;int lock;double charge;size_t shot_count;ShotFixture shots[64];size_t fx_count;RRFxLife fx[512];int coin_count;CoinFixture coins[RR_COIN_MAX];int collection_count;int collections[RR_COIN_MAX][2];PlayerExtra player_more;} FrameFixture;
typedef struct {int road;unsigned seed;BattleState before[3];bool active;double time,after_cd;size_t shot_count;ShotFixture shots[1];unsigned draws,rng;} RivalFixture;
'''
def block(name,typ,rows):return 'static const '+typ+' '+name+'[]={\n'+',\n'.join('{'+','.join(row)+'}' for row in rows)+'\n};\n'
text+=block('trace_fixtures','TraceFixture',[[n(t['road']),n(t['seed']),arr(t['drivers']),reset(t['start'])] for t in d['traces']])
rows=[]
for ti,t in enumerate(d['traces']):
 for f in t['frames']:
  rows.append([n(ti),n(f['reset'] is not None),reset(f['reset']) if f['reset'] else '{0}',n(f['mask']),n(f['branch']),n(f['dt']),n(f['race_time']),state(f['before']),arr(f['muzzle']),arr(f['side']),arr(f['control']),'{'+','.join('{true,'+str(v[0])+','+str(v[1])+','+arr(v[2])+'}' if v else '{false,0,0,{0}}' for v in f['bindings'])+'}',n(f['active']),*[n(v) for v in f['input']],states(f['after']),n(f['rng']),n(f['draws']),meta(f['meta']),arr(f['arsenal']),n(f['lock']),n(f['charge']),n(len(f['shots'])),arr(f['shots']) if f['shots'] else '{{0}}',n(len(f['fx'])),arr(f['fx']) if f['fx'] else '{{0}}',n(len(f['coins'])),arr(f['coins']),n(len(f['coinEvents'])),arr(f['coinEvents']) if f['coinEvents'] else '{{0}}',arr(f['player_more'])])
text+=block('frame_fixtures','FrameFixture',rows)

(s/'player_phase_reference.h').write_text(text);print('Generated player-phase720 full-roster frames; pose bindings supplied, no legacy probes')

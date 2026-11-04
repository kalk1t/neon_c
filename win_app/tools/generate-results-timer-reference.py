from pathlib import Path
import json
p=Path(__file__).resolve().parents[1];folder=p/'reference/ui-parity-20261103/results-timer-capture';run=json.loads((folder/'battle-reference.json').read_text());unit=json.loads((folder/'timer-cases.json').read_text())
def n(v):
 if v is None:raise ValueError('Missing timer reference value')
 return ('true' if v else 'false') if isinstance(v,bool) else repr(v)
def arr(v):return '{'+','.join(arr(x) if isinstance(x,list) else n(x) for x in v)+'}'
def board(v):
 if v is None:return '{false}'
 assert len(v['lap_values'])<=2
 return '{'+','.join(['true',n(v['total']),n(v['best']),n(len(v['lap_values'])),arr(v['lap_values']) if v['lap_values'] else '{0}',n(len(v['times'])),n(len(v['coins'])),arr(v['times']) if v['times'] else '{{0}}',arr(v['coins']),n(v['time_rank']),n(v['coin_rank']),n(v['finish_position'])])+'}'
def stamp(v):
 assert len(v['pending'])<=16
 return '{'+','.join([n(v['wall']),n(v['callbacks']),n(v['published']),n(len(v['pending'])),arr(v['pending']) if v['pending'] else '{0}',n(v['revision']),n(v['visible']),board(v['board'])])+'}'
phases={'menu':0,'race':1,'paused':2,'countdown':3,'finished':4,'results':5}
head='''/* Actual source virtual timeout queue, display-prefix and visibility tail. */
typedef struct {bool valid;double total;int best;size_t lap_count;double laps[2];size_t time_count,coin_count;RRResultsTime times[11];RRResultsCoin coins[11];int time_rank,coin_rank,finish_position;} ExpectedTimerBoard;
typedef struct {double now;unsigned callbacks,published;size_t pending;double due[16];size_t revision;bool visible;ExpectedTimerBoard board;} ExpectedTimer;
typedef struct {int kind;double now;int phase;double time;int coins;ExpectedTimer expected;} TimerAction;
typedef struct {size_t count;TimerAction actions[32];} TimerCase;
static const TimerCase timer_cases[]={
'''
rows=[]
for c in unit['cases']:
 assert len(c['actions'])<=32
 rows.append('{'+n(len(c['actions']))+',{'+','.join('{'+','.join([n(a['kind']),n(a['now']),n(phases[a['phase']]),n(a['time']),n(a['coins']),stamp(a['expected'])])+'}' for a in c['actions'])+'}}')
head+=',\n'.join(rows)+'\n};\n'
head+='static const ExpectedTimer timer_frames[]={\n'+',\n'.join(stamp(f['presentation']) for t in run['traces'] for f in t['frames'])+'\n};\n'
(p/'tests/results_timer_reference.h').write_text(head);print('Generated9 timer scenarios/48 actions and1,440 source task boundaries.')

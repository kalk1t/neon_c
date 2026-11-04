from pathlib import Path
import json
p=Path(__file__).resolve().parents[1];d=json.loads((p/'reference/ui-parity-20261103/current-results-capture/results-reference.json').read_text())
def n(v):
 if v is None:raise ValueError('Missing result value')
 return ('true' if v else 'false') if isinstance(v,bool) else repr(v)
def arr(v):return '{'+','.join(arr(x) if isinstance(x,list) else n(x) for x in v)+'}'
def board(v):
 if v is None:return '{false}'
 return '{'+','.join(['true',n(v['total']),n(v['best']),n(len(v['times'])),n(len(v['coins'])),arr(v['times']) if v['times'] else '{{0}}',arr(v['coins']),n(v['time_rank']),n(v['coin_rank']),n(v['finish_position'])])+'}'
phases={'menu':0,'race':1,'paused':2,'countdown':3,'finished':4,'results':5}
head='''/* Actual showResults display prefix: no reward, account, unlock or timer execution. */
typedef struct {bool enabled;double total;int best;size_t time_count,coin_count;RRResultsTime times[11];RRResultsCoin coins[11];int time_rank,coin_rank,finish_position;} ExpectedResults;
typedef struct {int sample,lap,next;bool finished;double finish_time;int coins;} ResultActor;
typedef struct {size_t count,player;int samples,finish,phase;double time;size_t lap_count;double laps[2];ResultActor actors[11];ExpectedResults expected;} ResultCase;
static const ResultCase result_cases[]={
'''
head+=',\n'.join('{'+','.join([n(c['count']),n(c['player']),n(c['n']),n(c['finish']),n(phases[c['phase']]),n(c['time']),n(len(c['laps'])),arr(c['laps']) if c['laps'] else '{0}',arr(c['states']),board(c['expected'])])+'}' for c in d['cases'])+'\n};\n'
head+='static const ExpectedResults run_results_expected[]={\n'+',\n'.join(board(v) for v in d['snapshots'])+'\n};\n'
(p/'tests/current_results_reference.h').write_text(head);print('Generated display cases and 1,440 full-pack snapshot expectations.')

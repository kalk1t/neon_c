from pathlib import Path
import json,sys
s=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).parent
d=json.loads((s/'arsenal-reference.json').read_text())
def n(x):
 if isinstance(x,bool):return 'true' if x else 'false'
 return repr(x)
def arr(v):return '{'+','.join(map(n,v))+'}'
rows=[]
for c in d['cases']:
 rows.append('{'+','.join([n(c['load']),n(c['action']),arr(c['before']),n(c['dt']),arr(c['input']),arr(c['random']),arr(c['after']),str(len(c['order'])),arr(c['order']+[0]*(6-len(c['order']))),str(c['draws']),arr(c['spread']),str(c['spin']),n(c['homing'])])+'}')
text='''/* Generated from actual browser playerWeapons reference; do not edit. */
typedef struct {bool load; int action; RRArsenal before; double dt; RRArsenalInput input; double random[3]; RRArsenal after; unsigned count,order[6],draws; double spread[2]; int spin; bool homing;} ArsenalFixture;
static const ArsenalFixture fixtures[]={
'''+',\n'.join(rows)+'\n};\n'
(s/'generated_arsenal_reference.h').write_text(text)
print('Generated',len(rows),'fixtures')

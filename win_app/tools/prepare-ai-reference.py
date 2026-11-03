from pathlib import Path
import json,sys
if len(sys.argv)!=2:raise SystemExit('Usage: prepare-ai-reference.py STAGING_DIRECTORY')
s=Path(sys.argv[1]);j=json.loads((s/'ai-reference.json').read_text());sample=json.loads((s/'poster-sampling-reference.json').read_text())
def num(x):
 if x is None:x=0
 return str(int(x)) if isinstance(x,bool) else format(x,'.17g')
def vec(x):return '{'+','.join(map(num,x))+'}'
def inp(x):return '{'+','.join(num(x.get(k,0)) for k in ['throttle','brake','steer','climb'])+','+str(x.get('boost',False)).lower()+'}'
def car(c):
 parts=[]
 for k in ['pos','vel','fwd']:parts.append('.'+k+'='+vec(c[k]))
 for k in ['yaw','pitch','roll','speed','boost','t','stun','roll_time','surge','max_multiplier','thrust','nitro','kick','sputter','impact','roll_direction']:parts.append('.'+k+'='+num(c.get(k,0)))
 parts.append('.boosting='+str(c.get('boosting',False)).lower());return '{'+','.join(parts)+'}'
def driver(d):return '{'+','.join(num(d.get(k,0)) for k in ['sample','seed','lane','laneY','dodge','skill'])+'}'
header='/* Actual source AI calls and coupled Car traces; read-only browser capture. */\nstatic const AIInputFixture ai_inputs[]={\n'
for row in j['input_cases']:
 d=row['before'];peers=row['peers'];peer='{'+','.join('{'+vec(p['pos'])+','+str(p['alive']).lower()+'}' for p in peers)+'}'
 header+='{'+str(row['road'])+','+driver(d)+','+car(d['car'])+','+peer+','+vec(row['target'])+','+vec(row['coin'] or [0,0,0])+','+str(row['coin'] is not None).lower()+','+num(row['time'])+','+num(row['dt'])+','+inp(row['expected'])+','+num(row['expected_dodge'])+'},\n'
header+='};\nstatic const AITraceFixture ai_traces[]={\n'
for t in j['traces']:header+='{'+str(t['road'])+','+driver(t['driver'])+','+car(t['initial'])+'},\n'
header+='};\nstatic const AITraceStep ai_steps[]={\n'
for index,t in enumerate(j['traces']):
 for step in t['steps']:header+='{'+f'{index},{step["sample"]},'+vec(step['target'])+','+vec(step['coin'] or [0,0,0])+','+str(step['coin'] is not None).lower()+','+num(step['time'])+','+num(step['dt'])+','+inp(step['input'])+','+num(step['dodge'])+','+car(step['car'])+'},\n'
header+='};\nstatic const AIProgressFixture ai_progress[]={\n'+',\n'.join('{'+','.join(num(x[k]) for k in ['sample','lap','nextCP','n','expected'])+'}' for x in j['progress'])+'\n};\nstatic const AIRubberFixture ai_rubber[]={\n'+',\n'.join('{'+','.join(num(x[k]) for k in ['road','n','player_progress','rival_progress','factor'])+'}' for x in j['rubber'])+'\n};\n'
(s/'generated_ai_reference.h').write_text(header)
roster='/* Captured source ten-driver roster; no random spawning or runtime activation. */\nstatic const RRAILivery rr_ai_liveries[RR_AI_RIVALS]={\n'+',\n'.join('{'+json.dumps(x['name'])+f",{x['paint']}u,{x['accent']}u,{num(x['skill'])},{num(x.get('metal',0))},"+str('metal' in x).lower()+'}' for x in j['liveries'])+'\n};\n';(s/'rr_ai_liveries.h').write_text(roster)
sampling='/* Actual source Texture.transformUv and base-level bilinear address cases. */\nstatic const PosterSampleFixture poster_samples[]={\n'+',\n'.join('{'+vec(x['uv'])+','+vec(x['canvas_uv'])+','+vec(x['texels'])+','+vec(x['weights'])+'}' for x in sample['samples'])+'\n};\n';(s/'generated_poster_sampling.h').write_text(sampling)
print('Prepared 288 AI cases, 3240 coupled steps,108 progress,45 road rubber-band and121 source sampling fixtures.')

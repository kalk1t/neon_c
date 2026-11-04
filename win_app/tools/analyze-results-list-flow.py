from pathlib import Path
import json,math
base=(Path(__file__).parent/'analyze-results-name-flow.py').read_text();namespace={'__file__':str(Path(__file__).parent/'analyze-results-name-flow.py')};exec(compile(base[:base.index('gold=json.loads(')],'qualified-name-flow-prototype','exec'),namespace)
name_flow=namespace['flow'];units=namespace['units'];f32=namespace['f32'];H=namespace['H'];pack=namespace['pack']
def measure(text):return sum(15 if c=='\u265b' else 9.90234375 if c=='\u2248' else f32(units[ord(c)]*f32(.015)) for c in text)
def quantize(value,mode):return (math.floor(value*64) if mode=='floor' else math.ceil(value*64) if mode=='ceil' else math.floor(value*64+.5))/64
gold=json.loads((pack/'native-layout-goldens.json').read_text(encoding='utf-8'));reports=[];best_cases=[]
for cursor_mode in ['floor','nearest','ceil']:
 for float_mode in ['ceil','nearest','floor']:
  mismatches=[];rows=0;cases=[];relocated=0;below_tail=0;positive_prior=0;stacked=0
  for o in gold:
   for kind,l in o['lists'].items():
    width=l['box']['width']-26;ledger=[];y=0;result_rows=[]
    for source in l['rows']:
     prior=[(f[0],f[1]-y,f[2],f[3]) for f in ledger];positive_prior+=sum(f[1]>0 for f in prior)
     lines,height=name_flow(source['name'],width,prior);tail=lines[-1];tail_y=tail[3]-1;cursor=quantize(measure(source['name'][tail[0]:tail[1]]),cursor_mode)
     fw=quantize(measure(source['value']),float_mode);fy=tail_y
     for attempt in range(14):
      active=[f for f in prior if f[1]<fy+H and f[1]+f[3]>fy];right=min([width]+[f[0] for f in active]);consumed=cursor if fy==tail_y else 0
      if fw+consumed<=right:break
      fy+=H
     else:raise ValueError('Float loop')
     placed=[right-fw,y+fy,fw,H];expected=[source['floatBox']['x']-(l['box']['x']+26),source['floatBox']['y']-l['box']['y'],source['floatBox']['width'],source['floatBox']['height']]
     name_lines=[list(x[2:]) for x in lines];expected_names=[[x['x']-source['box']['x'],x['y']-source['box']['y'],x['width'],x['height']] for x in source['nameLineRects']]
     okay=placed==expected and y==source['box']['y']-l['box']['y'] and height==source['box']['height'] and name_lines==expected_names
     if not okay:mismatches.append(dict(fixture=o['fixture'],scale=o['scale'],kind=kind,row=source['ordinal'],name=source['name'],value=source['value'],predicted_float=placed,expected_float=expected,predicted_y=y,expected_y=source['box']['y']-l['box']['y'],predicted_height=height,expected_height=source['box']['height']))
     result_rows.append(dict(name=source['name'],value=source['value'],y=y,height=height,name_lines=lines,float=placed,expected_float=expected));rows+=1;relocated+=fy>0;below_tail+=fy>tail_y;stacked+=bool(active and fw+consumed<=right)
     ledger.append(placed);y+=height
    bottom=max([0]+[f[1]+f[3] for f in ledger]);height=max(y,bottom) if kind=='res-standings' else y
    if height!=l['box']['height']:mismatches.append(dict(fixture=o['fixture'],kind=kind,height=height,expected_height=l['box']['height']))
    cases.append(dict(fixture=o['fixture'],scale=o['scale'],offset=o['offset'],kind=kind,width=width,height=height,normal_height=y,float_bottom=bottom,contains_floats=kind=='res-standings',rows=result_rows))
  report=dict(cursor_mode=cursor_mode,float_mode=float_mode,lists=len(cases),rows=rows,mismatches=len(mismatches),relocated=relocated,below_name_tail=below_tail,stacked=stacked,positive_prior_floats=positive_prior,examples=mismatches[:4]);reports.append(report)
  if cursor_mode=='floor' and float_mode=='ceil':best_cases=cases
data=dict(hypotheses=reports,selected_cases=best_cases,scope='Fully coupled name/current-value float loop. Only source strings and measured container widths are inputs; all preceding float states are generated, not supplied from source observations.')
(Path(__file__).parent/'results-list-flow-analysis.json').write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8')
print(json.dumps(reports,ensure_ascii=True)[:9000])

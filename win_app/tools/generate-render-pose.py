from pathlib import Path
import json
p=Path(__file__).resolve().parents[1]
# Independent expected points come from actual Three hierarchy, not C math.
d=json.loads((p/'reference/ui-parity-20261103/render-pose-capture/render-pose-reference.json').read_text());old=json.loads((p/'reference/ui-parity-20261103/car-pose-capture/car-pose-reference.json').read_text())
def n(v):
 if v is None:raise ValueError('Missing render reference value')
 return repr(v)
def arr(v):return '{'+','.join(arr(x) if isinstance(x,list) else n(x) for x in v)+'}'
rows=[]
for t,prior in zip(d['traces'],old['traces'],strict=True):
 for f,prev in zip(t['frames'],prior['frames'],strict=True):
  assert f['matrix']==prev['matrix'] and f['state']==prev['state']
  r=f['render'];rows.append('{'+','.join([arr(r['fitted']),arr(r['body']),arr(r['fallback']),arr([[pod['pitch'],pod['roll']] for pod in r['pods']]),arr([pod['points'] for pod in r['pods']])])+'}')
(p/'tests/render_pose_reference.h').write_text('/* Actual Car.useModel and Three GLB-node hierarchy, CPU only. */\ntypedef struct {double fitted[16];RRVec3 body[4],fallback[4];double pod[4][2];RRVec3 points[4][2];} RenderSample;\nstatic const RenderSample render_samples[]={\n'+',\n'.join(rows)+'\n};\n')
t=(p/'tests/test_car_pose.c').read_text().replace('#include "rr_car_pose.h"','#include "rr_car_render_transform.h"\n#include "rr_pod_transform.h"\n#include "rr_pod_pivots.h"')
t=t.replace('#include "car_pose_reference.h"','#include "car_pose_reference.h"\n#include "render_pose_reference.h"')
t=t.replace('static unsigned failures;', 'static double float_error;\nstatic unsigned failures;',1)
needle='int main(void)';at=t.index(needle)
t=t[:at]+'''static void float_near(double a,double b){double e=fabs(a-b);if(e>float_error)float_error=e;CHECK(isfinite(a)&&isfinite(b)&&e<.0005);}
static void point(Vector3 actual,RRVec3 expected){float_near(actual.x,expected.x);float_near(actual.y,expected.y);float_near(actual.z,expected.z);}
'''+t[at:]
needle='\n }\n RRCarPose empty';assert needle in t
t=t.replace(needle,'''
  const RenderSample *r=render_samples+current;Matrix fitted,code;CHECK(rr_car_pose_raylib(&pose,-.35,&fitted));CHECK(rr_car_pose_raylib(&pose,0,&code));
  float16 fv=MatrixToFloatV(fitted),cv=MatrixToFloatV(code);for(int i=0;i<16;i++){float_near(fv.v[i],r->fitted[i]);float_near(cv.v[i],f->matrix[i]);}
  const Vector3 body[4]={{0,0,0},{1.1f,.7f,-2.5f},{-1.1f,.7f,2.7f},{.8f,1.5f,1.9f}},offsets[2]={{.2f,.4f,-.3f},{-.3f,-.2f,.6f}};
  for(int i=0;i<4;i++){point(Vector3Transform(body[i],fitted),r->body[i]);point(Vector3Transform(body[i],code),r->fallback[i]);near(car.pod_pitch[i],r->pod[i][0]);near(car.pod_roll[i],r->pod[i][1]);
   Matrix composed=MatrixMultiply(rr_pod_transform(rr_pod_pivots[i],car.pod_pitch[i],car.pod_roll[i]),fitted);
   for(int k=0;k<2;k++)point(Vector3Transform(Vector3Add(rr_pod_pivots[i],offsets[k]),composed),r->points[i][k]);
  }
 }
 RRCarPose empty''',1)
t=t.replace(' printf("CAR_POSE_SUMMARY', ' CHECK(!rr_car_pose_raylib(&empty,0,&(Matrix){0})&&!rr_car_pose_raylib(&pose,NAN,&(Matrix){0})&&!rr_car_pose_raylib(&pose,0,NULL));\n RRCarPose invalid=pose;invalid.root[0]=INFINITY;CHECK(!rr_car_pose_raylib(&invalid,0,&(Matrix){0}));\n printf("RENDER_POSE_DETAIL points=23040 matrix_values=46080 max_float_error=%.17g no_GPU=true\\n",float_error);\n printf("RENDER_POSE_SUMMARY',1)
(p/'tests/test_render_pose.c').write_text(t)

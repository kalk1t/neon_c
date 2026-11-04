from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');d=json.loads((p/'reference/ui-parity-20261103/audio-control-capture/reference.json').read_text(encoding='utf-8'))
assert d['version']=='0.2.16' and len(d['engineCases'])==192 and len(d['masterCases'])==48 and len(d['frameCases'])==1280
def flag(x):return 'true' if x else 'false'
lines=['''/* Actual source parameter commands; no samples/device renderer. */
typedef struct ExpectedAudio {int kind,phase;bool ready,boost,active,playing,muted;double a,b,c,now;size_t count;RRAudioTarget targets[7];} ExpectedAudio;
static const ExpectedAudio audio_cases[]={''']
for kind,key in enumerate(['engineCases','masterCases','frameCases']):
 for x in d[key]:
  a=x.get('sf',x.get('volume',x.get('speed',0)));b=x.get('side',0);c=x.get('wreck',0)
  events=','.join('{(RRAudioParameter)%d,%s,%s,%s}'%(e['id'],repr(e['target']),repr(e['time']),repr(e['tau'])) for e in x['events']) or '{0}'
  lines.append('{%d,%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%d,{%s}},'%(kind,x.get('phase',0),flag(x['ready']),flag(x.get('boosting',False)),flag(x.get('active',False)),flag(x.get('playing',False)),flag(x.get('muted',False)),repr(a),repr(b),repr(c),repr(x['now']),len(x['events']),events))
lines.append('};\n');(p/'tests/audio_control_reference.h').write_text('\n'.join(lines),encoding='ascii')
test=r'''#include "rr_audio_current.h"
#include "audio_control_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
static unsigned failures,engine_cases,master_cases,frame_cases,targets;
static double worst;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"audio line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double error=fabs(a-b);if(error>worst)worst=error;CHECK(isfinite(a)&&isfinite(b)&&error<=1e-10*fmax(1,fabs(b)));}
int main(void){
 RRAudioPlan plan;
 for(size_t i=0;i<COUNT(audio_cases);i++){
  const ExpectedAudio *e=audio_cases+i;bool ok=false;RRLocalRacer player={0};player.car.speed=e->a;player.car.side=e->b;player.car.boosting=e->boost;player.wreck=e->c;player.hp=0;player.visible=false;
  RRLocalRacer saved=player;
  if(e->kind==0){ok=rr_audio_engine_plan(e->ready,e->a,e->boost,e->active,e->now,&plan);engine_cases++;}
  else if(e->kind==1){ok=rr_audio_master_plan(e->ready,e->a,e->muted,e->now,&plan);master_cases++;}
  else{ok=rr_audio_engine_frame(e->ready,e->playing,(RRSessionPhase)e->phase,&player,e->now,&plan);frame_cases++;CHECK(!memcmp(&saved,&player,sizeof(player)));}
  CHECK(ok&&plan.count==e->count);for(size_t k=0;k<plan.count&&k<e->count;k++){const RRAudioTarget *a=plan.targets+k,*b=e->targets+k;CHECK(a->parameter==b->parameter);near(a->target,b->target);near(a->time,b->time);near(a->time_constant,b->time_constant);targets++;}
 }
 /* Source not-ready paths return before consulting otherwise invalid values. */
 CHECK(rr_audio_engine_plan(false,NAN,true,true,-1,&plan)&&!plan.count);
 CHECK(rr_audio_master_plan(false,NAN,false,NAN,&plan)&&!plan.count);
 CHECK(rr_audio_engine_frame(false,true,(RRSessionPhase)99,NULL,NAN,&plan)&&!plan.count);
 CHECK(rr_audio_master_plan(true,NAN,true,1,&plan)&&plan.count==1&&plan.targets[0].target==0);
 RRAudioPlan guard,saved;memset(&guard,0x5a,sizeof(guard));saved=guard;
 double invalid[]={NAN,INFINITY,-INFINITY};for(size_t i=0;i<COUNT(invalid);i++){
  CHECK(!rr_audio_engine_plan(true,invalid[i],false,true,1,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
  CHECK(!rr_audio_engine_plan(true,1,false,true,invalid[i],&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
  CHECK(!rr_audio_master_plan(true,invalid[i],false,1,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 }
 CHECK(!rr_audio_engine_plan(true,DBL_MAX,true,true,1,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(!rr_audio_engine_plan(true,1,false,true,-1,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(!rr_audio_master_plan(true,.7,false,-1,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 RRLocalRacer player={0};CHECK(!rr_audio_engine_frame(true,true,RR_SESSION_ACTIVE,NULL,1,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 player.car.speed=NAN;CHECK(!rr_audio_engine_frame(true,true,RR_SESSION_ACTIVE,&player,1,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 player.car.speed=10;player.wreck=NAN;CHECK(!rr_audio_engine_frame(true,true,RR_SESSION_ACTIVE,&player,1,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 player.wreck=0;CHECK(!rr_audio_engine_frame(true,true,(RRSessionPhase)99,&player,1,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(!rr_audio_engine_plan(false,0,false,false,0,NULL)&&!rr_audio_master_plan(false,0,false,0,NULL)&&!rr_audio_engine_frame(false,false,RR_SESSION_MENU,NULL,0,NULL));
 printf("AUDIO_CONTROL_SUMMARY engine_cases=%u master_cases=%u frame_cases=%u targets=%u plan_bytes=%zu max_error=%.17g failures=%u no_audio_devices_or_samples=true production_audio_inactive=true\n",engine_cases,master_cases,frame_cases,targets,sizeof(plan),worst,failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
'''
f=p/'tests/test_audio_control.c';assert not f.exists();f.write_text(test,encoding='utf-8')
shutil.copy2(__file__,p/'tools/generate-audio-control.py')
print('Generated1520 source command cases;5176 target events plus lazy/readiness/invalid-input checks.')

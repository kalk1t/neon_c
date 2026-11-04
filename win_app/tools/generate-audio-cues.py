from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');d=json.loads((p/'reference/ui-parity-20261103/audio-cues-capture/reference.json').read_text(encoding='utf-8'));assert len(d['cases'])==145 and d['version']=='0.2.16'
lines=['''/* Raw source request recipes only, not allocated voices or playback. */
typedef struct CueExpected {RRAudioCue cue;double amount,pan;bool flag;double unit;size_t count;unsigned draws;RRAudioCueRequest requests[4];} CueExpected;
static const CueExpected cue_cases[]={''']
for c in d['cases']:
 reqs=[]
 for x in c['requests']:reqs.append('{(RRAudioRequestKind)%d,(RRAudioWave)%d,(RRAudioFilter)%d,%s}'%(x['kind'],x['wave'],x['filter'],','.join(repr(x[k]) for k in ['frequency','duration','volume','when','slide','pan','from','to'])))
 lines.append('{(RRAudioCue)%d,%s,%s,%s,%s,%d,%d,{%s}},'%(c['cue'],repr(c['amount']),repr(c['pan']),'true' if c['flag'] else 'false',repr(c['unit']),len(c['requests']),c['randomDraws'],','.join(reqs)))
lines.append('};\n');(p/'tests/audio_cues_reference.h').write_text('\n'.join(lines),encoding='ascii')
test=r'''#include "rr_audio_cues.h"
#include "audio_cues_reference.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,requests,draws,coverage,max_requests;static double worst;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"cue line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double error=fabs(a-b);if(error>worst)worst=error;CHECK(isfinite(a)&&isfinite(b)&&error<=1e-11*fmax(1,fabs(b)));}
int main(void){
 RRAudioCuePlan plan;
 for(size_t i=0;i<COUNT(cue_cases);i++){
  const CueExpected *c=cue_cases+i;CHECK(rr_audio_cue_recipe(c->cue,c->amount,c->pan,c->flag,c->unit,&plan));CHECK(plan.count==c->count&&plan.recipe_random_draws==c->draws);coverage|=1u<<(unsigned)c->cue;draws+=plan.recipe_random_draws;if(plan.count>max_requests)max_requests=(unsigned)plan.count;
  for(size_t j=0;j<plan.count&&j<c->count;j++){
   const RRAudioCueRequest *a=plan.requests+j,*b=c->requests+j;CHECK(a->kind==b->kind&&a->wave==b->wave&&a->filter==b->filter);
   near(a->frequency,b->frequency);near(a->duration,b->duration);near(a->volume,b->volume);near(a->when,b->when);near(a->slide,b->slide);near(a->pan,b->pan);near(a->from,b->from);near(a->to,b->to);requests++;
  }
 }
 CHECK(coverage==(1u<<RR_CUE_COUNT)-1&&max_requests==4);
 RRAudioCuePlan guard,saved;memset(&guard,0x5a,sizeof(guard));saved=guard;
 CHECK(!rr_audio_cue_recipe((RRAudioCue)-1,1,0,false,0,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(!rr_audio_cue_recipe(RR_CUE_COUNT,1,0,false,0,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 double bad[]={NAN,INFINITY,-INFINITY};for(size_t i=0;i<COUNT(bad);i++){
  CHECK(!rr_audio_cue_recipe(RR_CUE_COIN,bad[i],0,false,0,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
  CHECK(!rr_audio_cue_recipe(RR_CUE_EXPLOSION,1,bad[i],false,0,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
  CHECK(!rr_audio_cue_recipe(RR_CUE_HIT,bad[i],0,false,0,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
  CHECK(!rr_audio_cue_recipe(RR_CUE_BLASTER,1,0,false,bad[i],&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 }
 CHECK(!rr_audio_cue_recipe(RR_CUE_BLASTER,1,0,false,1,&guard)&&!rr_audio_cue_recipe(RR_CUE_BLASTER,1,0,false,-.01,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(!rr_audio_cue_recipe(RR_CUE_LAP,1,0,false,0,NULL));
 CHECK(rr_audio_cue_recipe(RR_CUE_LAP,NAN,NAN,false,NAN,&plan)&&plan.count==4&&!plan.recipe_random_draws);
 CHECK(rr_audio_cue_recipe(RR_CUE_COIN,-1,2,false,NAN,&plan)&&plan.requests[0].volume<0&&plan.requests[0].pan==2); /* preserve pre-guard/pre-pan-clamp args */
 CHECK(rr_audio_cue_recipe(RR_CUE_BLASTER,NAN,NAN,false,.5,&plan)&&plan.recipe_random_draws==1&&plan.count==2); /* supplied RNG, not a callback */
 printf("AUDIO_CUES_SUMMARY cues=%d cases=%zu requests=%u pre_request_rng_draws=%u max_requests=%u plan_bytes=%zu max_error=%.17g failures=%u recipes_only_no_voice_dispatch_or_playback=true\n",RR_CUE_COUNT,COUNT(cue_cases),requests,draws,max_requests,sizeof(plan),worst,failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
'''
f=p/'tests/test_audio_cues.c';assert not f.exists();f.write_text(test,encoding='utf-8');shutil.copy2(__file__,p/'tools/generate-audio-cues.py')
print('Generated145 source cases for all18 cues and227 ordered requests. No dispatcher or playback.')

#include "rr_audio_dispatch.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <float.h>
typedef struct ExpectedVoice{int request,kind,wave,filter,ramp,panner;double start,stop,f0,f1,attack,decay,peak,gain0,gain1,pan,q,offset;}ExpectedVoice;
typedef struct DispatchCase{int cue,ready,seed,muted,flag;double amount,pan,now;int requests,draws,count,ended,voices_after;double units[4];int random_voices[4];ExpectedVoice commands[4];}DispatchCase;
#include "audio_dispatch_reference.h"
static int failures,checks;static double worst;static unsigned callbacks;
#define CHECK(x) do{checks++;if(!(x)){if(failures<10)fprintf(stderr,"audio dispatch line%d\n",__LINE__);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&e<1e-12);}
typedef struct Tape{const DispatchCase *c;RRAudioVoiceBank *bank;int cursor;double bad;bool fail_second;}Tape;
static double draw(void *context){Tape *t=context;callbacks++;if(t->c){CHECK(t->cursor<t->c->draws);if(t->cursor>=t->c->draws)return NAN;CHECK(t->bank->active==(unsigned)t->c->random_voices[t->cursor]);return t->c->units[t->cursor++];}t->cursor++;return t->fail_second&&t->cursor==2?t->bad:.25;}
static RRAudioCuePlan tone_plan(void){RRAudioCuePlan p={0};p.count=1;p.requests[0]=(RRAudioCueRequest){0};p.requests[0].kind=RR_CUE_TONE;p.requests[0].frequency=440;p.requests[0].duration=.1;p.requests[0].volume=.05;return p;}
static void seed(RRAudioVoiceBank *bank,int count){rr_audio_voice_bank_init(bank);RRAudioCuePlan p=tone_plan();for(int i=0;i<count;i++){RRAudioDispatch out;CHECK(rr_audio_dispatch(bank,true,0,&p,NULL,NULL,&out)&&out.count==1&&out.complete);}}
int main(void){
 for(size_t i=0;i<sizeof(dispatch_cases)/sizeof(dispatch_cases[0]);i++){
  const DispatchCase *c=&dispatch_cases[i];RRAudioVoiceBank bank;seed(&bank,c->seed);RRAudioCuePlan p;
  CHECK(rr_audio_cue_recipe((RRAudioCue)c->cue,c->amount,c->pan,c->flag!=0,c->cue==RR_CUE_BLASTER?c->units[0]:.5,&p));CHECK(p.count==(size_t)c->requests);
  if(p.recipe_random_draws){CHECK(p.recipe_random_draws==1&&c->random_voices[0]==c->seed);}
  Tape tape={c,&bank,(int)p.recipe_random_draws,0,false};RRAudioDispatch out;
  CHECK(rr_audio_dispatch(&bank,c->ready!=0,c->now,&p,draw,&tape,&out));CHECK(out.complete&&out.count==(size_t)c->count&&out.skipped+out.count==p.count);
  CHECK(bank.active==(unsigned)(c->seed+c->count));CHECK(tape.cursor==c->draws&&out.offset_draws+p.recipe_random_draws==(unsigned)c->draws);
  for(size_t j=0;j<out.count;j++){
   const ExpectedVoice *e=&c->commands[j];const RRAudioVoiceCommand *v=&out.commands[j];const RRAudioCueRequest *request=&v->request;
   CHECK(v->ready&&v->id==(uint64_t)c->seed+j+1&&v->request_index==(size_t)e->request&&request->kind==(RRAudioRequestKind)e->kind);
   CHECK(v->panner==(e->panner!=0));CHECK((request->kind==RR_CUE_RUMBLE||request->slide!=0)==(e->ramp!=0));
   if(request->kind==RR_CUE_TONE)CHECK(request->wave==(RRAudioWave)e->wave);else CHECK(request->filter==(RRAudioFilter)e->filter);
   near(v->start,e->start);near(v->stop,e->stop);near(v->attack_end,e->attack);near(v->decay_end,e->decay);near(request->volume,e->peak);near(v->gain_start,e->gain0);near(v->gain_end,e->gain1);near(v->pan,e->pan);near(v->filter_q,e->q);near(v->noise_offset,e->offset);
   near(request->kind==RR_CUE_TONE?request->frequency:request->from,e->f0);near(request->kind==RR_CUE_TONE?(request->slide?request->slide:request->frequency):request->to,e->f1);
  }
  if(c->ended){CHECK(rr_audio_voice_end(&bank,out.commands[0].id));if(c->ended==2)CHECK(rr_audio_voice_end(&bank,out.commands[out.count-1].id));}
  CHECK(bank.active==(unsigned)c->voices_after);
 }
 // No implicit deadline release: delayed/scheduled voices count until ended.
 RRAudioVoiceBank bank;seed(&bank,18);RRAudioCuePlan p=tone_plan();RRAudioDispatch out;CHECK(rr_audio_dispatch(&bank,true,1e9,&p,NULL,NULL,&out)&&out.count==0&&out.skipped==1&&bank.active==18);
 uint64_t old=bank.slots[0].id;CHECK(rr_audio_voice_end(&bank,old));p.requests[0].when=100;CHECK(rr_audio_dispatch(&bank,true,1e9,&p,NULL,NULL,&out)&&out.count==1&&bank.active==18);CHECK(out.commands[0].start==1e9+100);CHECK(out.commands[0].id!=old);CHECK(!rr_audio_voice_end(&bank,old)&&bank.active==18);CHECK(rr_audio_voice_end(&bank,out.commands[0].id));CHECK(!rr_audio_voice_end(&bank,out.commands[0].id));
 // Guard short-circuits unused invalid fields, but admitted invalid plans fail
 // preflight atomically without RNG demand or voice mutation.
 seed(&bank,0);p=tone_plan();p.requests[0].frequency=NAN;p.requests[0].duration=NAN;p.requests[0].pan=NAN;p.requests[0].volume=NAN;CHECK(rr_audio_dispatch(&bank,false,NAN,&p,NULL,NULL,&out)&&out.complete&&out.count==0&&out.skipped==1);
 RRAudioDispatch sentinel;memset(&sentinel,0x5a,sizeof(sentinel));out=sentinel;RRAudioVoiceBank saved=bank;CHECK(!rr_audio_dispatch(&bank,true,0,&p,NULL,NULL,&out)&&!memcmp(&out,&sentinel,sizeof(out))&&!memcmp(&bank,&saved,sizeof(bank)));
 p.requests[0].volume=.002;CHECK(rr_audio_dispatch(&bank,true,NAN,&p,NULL,NULL,&out)&&out.skipped==1&&bank.active==0);
 seed(&bank,18);p.requests[0].volume=NAN;CHECK(rr_audio_dispatch(&bank,true,NAN,&p,NULL,NULL,&out)&&out.skipped==1&&bank.active==18);
 seed(&bank,0);p=tone_plan();p.requests[0].duration=DBL_MAX;p.requests[0].when=DBL_MAX;out=sentinel;saved=bank;CHECK(!rr_audio_dispatch(&bank,true,DBL_MAX,&p,NULL,NULL,&out)&&!memcmp(&out,&sentinel,sizeof(out))&&!memcmp(&bank,&saved,sizeof(bank)));
 // Invalid callback policy is partial/reserved, with irreversible draw demand
 // exposed. Retire all returned tokens; never replay the whole plan.
 seed(&bank,0);CHECK(rr_audio_cue_recipe(RR_CUE_BOOST_ON,1,0,false,.5,&p));Tape bad={NULL,&bank,0,NAN,true};
 CHECK(!rr_audio_dispatch(&bank,true,10,&p,draw,&bad,&out));CHECK(!out.complete&&out.count==3&&out.offset_draws==2&&bad.cursor==2&&bank.active==3);CHECK(out.commands[0].ready&&out.commands[1].ready&&!out.commands[2].ready);
 for(size_t i=0;i<out.count;i++)CHECK(rr_audio_voice_end(&bank,out.commands[i].id));CHECK(bank.active==0);
 out=sentinel;saved=bank;CHECK(!rr_audio_dispatch(&bank,true,0,&p,NULL,NULL,&out)&&!memcmp(&out,&sentinel,sizeof(out))&&!memcmp(&bank,&saved,sizeof(bank)));
 CHECK(rr_audio_dispatch(&bank,false,NAN,&p,NULL,NULL,&out)&&out.complete&&out.count==0&&out.skipped==p.count);
 seed(&bank,0);bank.next_id=UINT64_MAX;p=tone_plan();CHECK(rr_audio_dispatch(&bank,true,0,&p,NULL,NULL,&out)&&out.commands[0].id==UINT64_MAX&&bank.next_id==0);CHECK(rr_audio_voice_end(&bank,UINT64_MAX));out=sentinel;saved=bank;CHECK(!rr_audio_dispatch(&bank,true,0,&p,NULL,NULL,&out)&&!memcmp(&out,&sentinel,sizeof(out))&&!memcmp(&bank,&saved,sizeof(bank)));
 printf("Audio dispatch:216 actual-source cue/guard cases,253 admitted voice commands,73 recipe/offset RNG observations, callback retirement/cap18/delayed/stale/error boundaries; checks=%d offset_callbacks=%u max_error=%.12g bank_bytes=%zu dispatch_bytes=%zu failures=%d; no DSP/context/device/main activation\n",checks,callbacks,worst,sizeof(bank),sizeof(out),failures);return failures?1:0;
}

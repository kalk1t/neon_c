#include "rr_audio_submit.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
typedef struct ExpectedVoice{int request,kind,wave,filter,ramp,panner;double start,stop,f0,f1,attack,decay,peak,gain0,gain1,pan,q,offset;}ExpectedVoice;
typedef struct DispatchCase{int cue,ready,seed,muted,flag;double amount,pan,now,clock_step;int requests,draws,count,ended,voices_after;double units[4];int random_voices[4];ExpectedVoice commands[4];}DispatchCase;
#include "audio_dispatch_reference.h"
static int checks,failures;static double worst;static unsigned observed_random,observed_clock;
#define CHECK(x) do{checks++;if(!(x)){if(failures<10)fprintf(stderr,"audio submit line%d\n",__LINE__);failures++;}}while(0)
static void near(double a,double b){double error=fabs(a-b);if(error>worst)worst=error;CHECK(isfinite(a)&&error<1e-12);}
typedef struct Harness{const DispatchCase *c;RRAudioVoiceBank *bank;unsigned draws,clocks;bool bad_first_random,bad_second_random,bad_second_clock;}Harness;
static double random_unit(void *context){Harness *h=context;unsigned at=h->draws++;observed_random++;
 if(h->c){CHECK(at<(unsigned)h->c->draws);if(at>=(unsigned)h->c->draws)return NAN;CHECK(h->bank->active==(unsigned)h->c->random_voices[at]);return h->c->units[at];}
 return (h->bad_first_random&&at==0)||(h->bad_second_random&&at==1)?NAN:.25;
}
static double audio_clock(void *context){Harness *h=context;unsigned at=h->clocks++;observed_clock++;
 if(h->c){CHECK(h->bank->active==(unsigned)h->c->seed+at);return h->c->now+at*h->c->clock_step;}
 return h->bad_second_clock&&at==1?NAN:10+at*.001;
}
static void seed(RRAudioVoiceBank *bank,int count){rr_audio_voice_bank_init(bank);RRAudioCuePlan p={0};p.count=1;p.requests[0].frequency=440;p.requests[0].duration=.1;p.requests[0].volume=.05;for(int i=0;i<count;i++){RRAudioDispatch d;CHECK(rr_audio_dispatch(bank,true,0,&p,NULL,NULL,&d)&&d.count==1);}}
static void retire(RRAudioVoiceBank *bank,const RRAudioCueSubmission *s){for(size_t i=0;i<s->voices.count;i++)CHECK(rr_audio_voice_end(bank,s->voices.commands[i].id));}
int main(void){
 for(size_t i=0;i<sizeof(dispatch_cases)/sizeof(dispatch_cases[0]);i++){
  const DispatchCase *c=&dispatch_cases[i];RRAudioVoiceBank bank;seed(&bank,c->seed);Harness h={c,&bank,0,0,false,false,false};RRAudioCueSubmission s;
  CHECK(rr_audio_submit_cue(&bank,c->ready!=0,(RRAudioCue)c->cue,c->amount,c->pan,c->flag!=0,random_unit,&h,audio_clock,&h,&s));
  CHECK(s.complete&&s.voices.complete&&s.error==RR_AUDIO_SUBMIT_OK&&s.failed_request==SIZE_MAX);
  CHECK(s.recipe.count==(size_t)c->requests&&s.processed_requests==s.recipe.count&&s.voices.count==(size_t)c->count&&s.voices.skipped+s.voices.count==s.recipe.count);
  CHECK(s.total_draws==(unsigned)c->draws&&h.draws==s.total_draws&&s.recipe_draws==s.recipe.recipe_random_draws);
  CHECK(s.clock_reads==(unsigned)c->count&&h.clocks==s.clock_reads);CHECK(bank.active==(unsigned)(c->seed+c->count));
  for(size_t j=0;j<s.voices.count;j++){
   const RRAudioVoiceCommand *v=&s.voices.commands[j];const ExpectedVoice *e=&c->commands[j];const RRAudioCueRequest *r=&v->request;
   CHECK(v->ready&&v->id==(uint64_t)c->seed+j+1&&v->request_index==(size_t)e->request&&r->kind==(RRAudioRequestKind)e->kind);
   CHECK(v->panner==(e->panner!=0)&&(r->kind==RR_CUE_RUMBLE||r->slide!=0)==(e->ramp!=0));
   if(r->kind==RR_CUE_TONE)CHECK(r->wave==(RRAudioWave)e->wave);else CHECK(r->filter==(RRAudioFilter)e->filter);
   near(v->start,e->start);near(v->stop,e->stop);near(v->attack_end,e->attack);near(v->decay_end,e->decay);near(r->volume,e->peak);near(v->gain_start,e->gain0);near(v->gain_end,e->gain1);near(v->pan,e->pan);near(v->filter_q,e->q);near(v->noise_offset,e->offset);
   near(r->kind==RR_CUE_TONE?r->frequency:r->from,e->f0);near(r->kind==RR_CUE_TONE?(r->slide?r->slide:r->frequency):r->to,e->f1);
  }
  if(c->ended){CHECK(rr_audio_voice_end(&bank,s.voices.commands[0].id));if(c->ended==2)CHECK(rr_audio_voice_end(&bank,s.voices.commands[s.voices.count-1].id));}
  CHECK(bank.active==(unsigned)c->voices_after);
 }
 // Caller failures retain all irreversible work and token ownership.
 RRAudioVoiceBank bank;seed(&bank,0);Harness h={NULL,&bank,0,0,false,false,false};RRAudioCueSubmission s;
 CHECK(rr_audio_submit_cue(&bank,false,RR_CUE_BLASTER,NAN,INFINITY,false,random_unit,&h,NULL,NULL,&s));CHECK(s.recipe_draws==1&&s.total_draws==1&&h.draws==1&&s.clock_reads==0&&s.voices.count==0&&s.voices.skipped==2);
 CHECK(!rr_audio_submit_cue(&bank,false,RR_CUE_BLASTER,1,0,false,NULL,NULL,NULL,NULL,&s));CHECK(s.error==RR_AUDIO_SUBMIT_RANDOM&&s.total_draws==0&&bank.active==0);
 h=(Harness){NULL,&bank,0,0,true,false,false};CHECK(!rr_audio_submit_cue(&bank,false,RR_CUE_BLASTER,1,0,false,random_unit,&h,NULL,NULL,&s));CHECK(s.error==RR_AUDIO_SUBMIT_RANDOM&&s.total_draws==1&&h.draws==1&&s.clock_reads==0&&bank.active==0);
 CHECK(!rr_audio_submit_cue(&bank,true,RR_CUE_COUNTDOWN,1,0,false,NULL,NULL,NULL,NULL,&s));CHECK(s.error==RR_AUDIO_SUBMIT_CLOCK&&s.failed_request==0&&s.processed_requests==0&&s.clock_reads==0&&bank.active==0);
 h=(Harness){NULL,&bank,0,0,false,false,true};CHECK(!rr_audio_submit_cue(&bank,true,RR_CUE_CHECKPOINT,1,0,false,random_unit,&h,audio_clock,&h,&s));CHECK(s.error==RR_AUDIO_SUBMIT_DISPATCH&&s.failed_request==1&&s.processed_requests==1&&s.clock_reads==2&&s.total_draws==0&&s.voices.count==1&&bank.active==1);retire(&bank,&s);CHECK(bank.active==0);
 h=(Harness){NULL,&bank,0,0,false,true,false};CHECK(!rr_audio_submit_cue(&bank,true,RR_CUE_BOOST_ON,1,0,false,random_unit,&h,audio_clock,&h,&s));CHECK(s.error==RR_AUDIO_SUBMIT_DISPATCH&&s.failed_request==2&&s.processed_requests==2&&s.clock_reads==3&&s.total_draws==2&&s.voices.count==3&&bank.active==3&&!s.voices.commands[2].ready);retire(&bank,&s);CHECK(bank.active==0);
 h=(Harness){NULL,&bank,0,0,false,false,false};CHECK(!rr_audio_submit_cue(&bank,true,RR_CUE_HIT,NAN,0,false,random_unit,&h,audio_clock,&h,&s));CHECK(s.error==RR_AUDIO_SUBMIT_RECIPE&&h.draws==0&&h.clocks==0&&bank.active==0);
 seed(&bank,18);h=(Harness){NULL,&bank,0,0,false,false,false};CHECK(rr_audio_submit_cue(&bank,true,RR_CUE_BLASTER,1,0,false,random_unit,&h,NULL,NULL,&s));CHECK(s.total_draws==1&&s.clock_reads==0&&s.voices.count==0&&bank.active==18);
 CHECK(rr_audio_submit_cue(&bank,true,RR_CUE_BOOST_ON,1,0,false,NULL,NULL,NULL,NULL,&s)&&s.voices.count==0&&s.clock_reads==0&&s.total_draws==0);
 RRAudioCueSubmission sentinel;memset(&sentinel,0x5a,sizeof(sentinel));s=sentinel;RRAudioVoiceBank saved=bank;CHECK(!rr_audio_submit_cue(&bank,true,RR_CUE_COUNT,1,0,false,random_unit,&h,audio_clock,&h,&s)&&!memcmp(&s,&sentinel,sizeof(s))&&!memcmp(&bank,&saved,sizeof(bank)));
 seed(&bank,0);bank.active=1;s=sentinel;h=(Harness){NULL,&bank,0,0,false,false,false};CHECK(!rr_audio_submit_cue(&bank,false,RR_CUE_BLASTER,1,0,false,random_unit,&h,audio_clock,&h,&s)&&!memcmp(&s,&sentinel,sizeof(s))&&h.draws==0&&h.clocks==0);
 printf("Audio submit integration:216 reused actual-source cases,253 voice commands,73 source RNG observations,18 advancing-clock cases; checks=%d observed_random=%u observed_clock=%u max_error=%.12g submission_bytes=%zu failures=%d; no DSP/backend/context/device/main activation\n",checks,observed_random,observed_clock,worst,sizeof(s),failures);return failures?1:0;
}

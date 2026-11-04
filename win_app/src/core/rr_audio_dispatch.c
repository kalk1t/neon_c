#include "rr_audio_dispatch.h"
#include <math.h>
#include <string.h>
void rr_audio_voice_bank_init(RRAudioVoiceBank *s){if(s){memset(s,0,sizeof(*s));s->next_id=1;}}
bool rr_audio_voice_end(RRAudioVoiceBank *s,uint64_t id){
 if(!s||!id||!s->active)return false;
 for(size_t i=0;i<RR_AUDIO_MAX_VOICES;i++)if(s->slots[i].active&&s->slots[i].id==id){s->slots[i].active=false;s->active--;return true;}return false;
}
static bool bank_valid(const RRAudioVoiceBank *s){
 if(!s||s->active>RR_AUDIO_MAX_VOICES)return false;unsigned count=0;
 for(size_t i=0;i<RR_AUDIO_MAX_VOICES;i++)if(s->slots[i].active){if(!s->slots[i].id||(s->next_id&&s->slots[i].id>=s->next_id))return false;count++;for(size_t j=0;j<i;j++)if(s->slots[j].active&&s->slots[j].id==s->slots[i].id)return false;}
 return count==s->active;
}
static bool admitted_valid(const RRAudioCueRequest *r,double now){
 if(!isfinite(now)||now<0||!isfinite(r->duration)||r->duration<=0||!isfinite(r->volume)||!isfinite(r->pan))return false;
 double start=now;
 if(r->kind==RR_CUE_TONE){if((r->wave!=RR_CUE_SINE&&r->wave!=RR_CUE_TRIANGLE)||!isfinite(r->frequency)||r->frequency<=0||!isfinite(r->slide)||r->slide<0||!isfinite(r->when)||r->when<0)return false;start+=r->when;}
 else if((r->filter!=RR_CUE_LOWPASS&&r->filter!=RR_CUE_BANDPASS)||!isfinite(r->from)||r->from<=0||!isfinite(r->to)||r->to<=0)return false;
 double end=start+r->duration,stop=end+(r->kind==RR_CUE_TONE?.05:0),attack=start+(r->kind==RR_CUE_TONE?.008:fmin(.03,r->duration*.2));return isfinite(start)&&isfinite(end)&&isfinite(stop)&&isfinite(attack);
}
bool rr_audio_dispatch(RRAudioVoiceBank *s,bool ready,double now,const RRAudioCuePlan *p,RRAudioOffsetRandom random,void *context,RRAudioDispatch *out){
 if(!out||!p||p->count>RR_CUE_REQUESTS||!bank_valid(s))return false;
 unsigned available=RR_AUDIO_MAX_VOICES-s->active,needed=0;
 for(size_t i=0;i<p->count;i++){
  const RRAudioCueRequest *r=&p->requests[i];if(r->kind!=RR_CUE_TONE&&r->kind!=RR_CUE_RUMBLE)return false;
  if(!ready||r->volume<=.002||!available)continue;
  if(!admitted_valid(r,now)||(r->kind==RR_CUE_RUMBLE&&!random))return false;available--;needed++;
 }
 if(needed&&(!s->next_id||s->next_id>UINT64_MAX-(needed-1)))return false;
 RRAudioDispatch result={0};
 for(size_t i=0;i<p->count;i++){
  const RRAudioCueRequest *r=&p->requests[i];if(!ready||r->volume<=.002||s->active>=RR_AUDIO_MAX_VOICES){result.skipped++;continue;}
  size_t at=0;while(at<RR_AUDIO_MAX_VOICES&&s->slots[at].active)at++;
  uint64_t id=s->next_id++;s->slots[at]=(RRAudioVoiceSlot){id,true};s->active++;
  RRAudioVoiceCommand *c=&result.commands[result.count++];c->request=*r;c->id=id;c->request_index=i;
  c->start=now+(r->kind==RR_CUE_TONE?r->when:0);c->attack_end=c->start+(r->kind==RR_CUE_TONE?.008:fmin(.03,r->duration*.2));c->decay_end=c->start+r->duration;c->stop=c->decay_end+(r->kind==RR_CUE_TONE?.05:0);
  c->panner=r->pan!=0;c->pan=c->panner?fmax(-1,fmin(1,r->pan)):0;c->filter_q=r->kind==RR_CUE_RUMBLE?(r->filter==RR_CUE_BANDPASS?1.4:.7):0;c->gain_start=0;c->gain_end=.0001;
  if(r->kind==RR_CUE_RUMBLE){result.offset_draws++;c->noise_offset=random(context);if(!isfinite(c->noise_offset)||c->noise_offset<0||c->noise_offset>=1){*out=result;return false;}}
  c->ready=true;
 }
 result.complete=true;*out=result;return true;
}

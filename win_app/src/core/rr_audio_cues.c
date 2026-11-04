#include "rr_audio_cues.h"
#include <math.h>
static void tone(RRAudioCuePlan *p,double f,double dur,RRAudioWave wave,double vol,double when,double slide,double pan){RRAudioCueRequest *r=p->requests+p->count++;r->kind=RR_CUE_TONE;r->wave=wave;r->frequency=f;r->duration=dur;r->volume=vol;r->when=when;r->slide=slide;r->pan=pan;}
static void rumble(RRAudioCuePlan *p,double dur,double from,double to,double vol,double pan,RRAudioFilter filter){RRAudioCueRequest *r=p->requests+p->count++;r->kind=RR_CUE_RUMBLE;r->filter=filter;r->duration=dur;r->from=from;r->to=to;r->volume=vol;r->pan=pan;}
bool rr_audio_cue_recipe(RRAudioCue cue,double amount,double pan,bool flag,double unit,RRAudioCuePlan *out)
{
 if(!out||(unsigned)cue>=RR_CUE_COUNT)return false;RRAudioCuePlan p={0};
 switch(cue){
 case RR_CUE_COUNTDOWN:tone(&p,flag?880:440,flag?.6:.22,RR_CUE_SINE,flag?.12:.09,0,0,0);break;
 case RR_CUE_CHECKPOINT:tone(&p,784,.32,RR_CUE_SINE,.05,0,0,0);tone(&p,1175,.32,RR_CUE_SINE,.05,.07,0,0);break;
 case RR_CUE_LAP:{const double f[]={659,784,988,1318};for(size_t i=0;i<4;i++)tone(&p,f[i],.45,RR_CUE_SINE,.055,(double)i*.08,0,0);break;}
 case RR_CUE_COIN:tone(&p,1318,.09,RR_CUE_SINE,.05*amount,0,0,pan);tone(&p,1976,.22,RR_CUE_SINE,.045*amount,.06,0,pan);break;
 case RR_CUE_BOOST_ON:rumble(&p,.55,1600,260,.04,0,RR_CUE_LOWPASS);tone(&p,95,.4,RR_CUE_SINE,.05,0,40,0);rumble(&p,.3,5000,1800,.015,0,RR_CUE_BANDPASS);break;
 case RR_CUE_BOOST_OFF:rumble(&p,.35,2400,500,.018,0,RR_CUE_BANDPASS);break;
 case RR_CUE_BLASTER:{if(!isfinite(unit)||unit<0||unit>=1)return false;double f=620+unit*60;p.recipe_random_draws=1;tone(&p,f,.075,RR_CUE_TRIANGLE,.028,0,f*.45,0);tone(&p,f*2,.04,RR_CUE_SINE,.008,0,0,0);break;}
 case RR_CUE_OVERHEAT:tone(&p,520,.4,RR_CUE_TRIANGLE,.05,0,180,0);rumble(&p,.5,3000,600,.025,0,RR_CUE_BANDPASS);break;
 case RR_CUE_MISSILE:rumble(&p,.7,400,2200,.06,0,RR_CUE_BANDPASS);tone(&p,180,.35,RR_CUE_SINE,.06,0,90,0);break;
 case RR_CUE_EMP:tone(&p,700,.7,RR_CUE_SINE,.09,0,55,0);tone(&p,1400,.3,RR_CUE_TRIANGLE,.025,0,300,0);rumble(&p,.9,2400,80,.07,0,RR_CUE_LOWPASS);break;
 case RR_CUE_DENY:tone(&p,220,.1,RR_CUE_TRIANGLE,.035,0,0,0);break;
 case RR_CUE_LOCK_TICK:tone(&p,1320,.04,RR_CUE_SINE,.02,0,0,0);break;
 case RR_CUE_LOCKED:tone(&p,1568,.07,RR_CUE_SINE,.035,0,0,0);tone(&p,2093,.12,RR_CUE_SINE,.03,.07,0,0);break;
 case RR_CUE_ZAP:tone(&p,760,.18,RR_CUE_SINE,.06*amount,0,240,pan);break;
 case RR_CUE_HIT_MARK:tone(&p,flag?1760:2350,flag?.08:.035,RR_CUE_SINE,flag?.05:.018,0,0,0);break;
 case RR_CUE_HIT:{if(!isfinite(amount))return false;double v=fmin(.16,.05+amount*.005);tone(&p,140,.25,RR_CUE_SINE,v,0,45,0);rumble(&p,.18,800,120,v*.35,0,RR_CUE_LOWPASS);break;}
 case RR_CUE_DAMAGE:tone(&p,300,.14,RR_CUE_TRIANGLE,.035*amount,0,140,pan);break;
 case RR_CUE_EXPLOSION:tone(&p,85,.8,RR_CUE_SINE,.18*amount,0,28,pan);rumble(&p,1,1100,60,.12*amount,pan,RR_CUE_LOWPASS);break;
 default:return false;
 }
 for(size_t i=0;i<p.count;i++){const RRAudioCueRequest *r=p.requests+i;if(!isfinite(r->frequency)||!isfinite(r->duration)||!isfinite(r->volume)||!isfinite(r->when)||!isfinite(r->slide)||!isfinite(r->pan)||!isfinite(r->from)||!isfinite(r->to))return false;}
 *out=p;return true;
}

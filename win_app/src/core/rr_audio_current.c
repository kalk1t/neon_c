#include "rr_audio_current.h"
static bool target(RRAudioPlan *p,RRAudioParameter parameter,double value,double now,double tau)
{
 if(p->count>=RR_AUDIO_TARGETS||!isfinite(value))return false;
 p->targets[p->count++]=(RRAudioTarget){parameter,value,now,tau};return true;
}
bool rr_audio_engine_plan(bool ready,double sf,bool boosting,bool active,double now,RRAudioPlan *out)
{
 if(!out)return false;RRAudioPlan p={0};if(!ready){*out=p;return true;}
 if(!isfinite(sf)||!isfinite(now)||now<0)return false;
 double f=52+sf*78+(boosting?14:0);
 if(!target(&p,RR_AUDIO_OSC0,f,now,.12)||!target(&p,RR_AUDIO_OSC1,f*2.004,now,.12)||!target(&p,RR_AUDIO_OSC2,f*.5,now,.12)||
    !target(&p,RR_AUDIO_ENGINE_LOWPASS,320+sf*650+(boosting?350:0),now,.15)||
    !target(&p,RR_AUDIO_ENGINE_GAIN,active?.045+sf*.03:0,now,.25)||
    !target(&p,RR_AUDIO_WIND_LOWPASS,220+sf*700,now,.2)||
    !target(&p,RR_AUDIO_WIND_GAIN,active?sf*sf*.03+(boosting?.015:0):0,now,.3))return false;
 *out=p;return true;
}
bool rr_audio_master_plan(bool ready,double volume,bool muted,double now,RRAudioPlan *out)
{
 if(!out)return false;RRAudioPlan p={0};if(!ready){*out=p;return true;}
 if(!isfinite(now)||now<0||(!muted&&!isfinite(volume)))return false;
 if(!target(&p,RR_AUDIO_MASTER_GAIN,muted?0:.28*volume,now,.03))return false;*out=p;return true;
}
bool rr_audio_engine_frame(bool ready,bool playing,RRSessionPhase phase,const RRLocalRacer *player,double now,RRAudioPlan *out)
{
 if(!ready)return rr_audio_engine_plan(false,0,false,false,now,out);
 if(!player||!isfinite(player->car.speed)||!isfinite(player->wreck)||phase<RR_SESSION_MENU||phase>RR_SESSION_ERROR)return false;
 return rr_audio_engine_plan(true,fmax(0,player->car.speed)/RR_CURRENT_CAR_MAX,player->car.boosting,(playing||phase==RR_SESSION_COUNTDOWN)&&player->wreck<=0,now,out);
}

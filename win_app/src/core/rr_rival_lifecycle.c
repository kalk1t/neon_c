#include "rr_rival_lifecycle.h"
bool rr_rival_lifecycle_reset(RRLocalRacer *r,RRRivalLifeResources *s,const RRTrack *t,RRVec3 pos,double yaw,RRRaceMode mode,double bolt,double reaction){
 if(!s||!rr_rival_reset(r,t,pos,yaw,mode,bolt,reaction))return false;
 r->hp=rr_racer_max_hp(r);r->fire_cd=0;rr_rival_current_reset(&s->bolt,bolt*.5);s->missiles=3;s->seeker_cd=0;s->spin_cd=0;s->emp_cd=0;s->locked=NULL;s->lock_time=0;return true;
}
bool rr_rival_lifecycle_drive(RRLocalRacer *r,const RRTrack *t,const RRAICorners *c,const struct RRWorld *w,const RRAIPeer *peers,size_t count,RRVec3 target,const RRVec3 *coin,double time,double dt,double race_time,double floor_height,RRRivalRandom random,void *context,RRInput *input,unsigned *events){
 if(!r||!t||!input||!events||r->is_player||r->race.mode==RR_RACE_FREE_ROAM||!isfinite(dt)||dt<=0||dt>.05||!isfinite(race_time))return false;
 *input=(RRInput){0};*events=0;
 if(race_time>r->react&&!rr_ai_current_controls(&r->ai,&r->car,t,c,peers,count,target,time,dt,coin,input)){r->error=true;return false;}
 *events=rr_rival_step_current_random(r,t,w,*input,dt,race_time,floor_height,random,context);return !r->error;
}

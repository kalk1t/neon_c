#include "rr_run_current.h"
#include "rr_physics.h"
static void after_weapons(void *context){
 RRRunCurrent *s=context;RRLocalRun *h=&s->host;RRLocalRacer *r=h->battle->fire.player;
 if(h->current_input.manual_reset&&h->current_input.manual_reset_allowed&&r->wreck<=0){
  if(!rr_player_phase_respawn(s->current))h->error=true;else h->current_events|=RR_RUN_RESPAWN|RR_RUN_MANUAL;
 }
}
bool rr_run_current_begin(RRRunCurrent *s,RRBattleCurrent *c){
 if(!s||!c||(s->current&&s->current!=c)||c->error||!c->battle||!c->battle->pack||!c->battle->fire.player||!c->pack.poses[0].ready)return false;
 RRLocalBattle *b=c->battle;if(b->after_weapons&&(b->after_weapons!=after_weapons||b->after_weapons_context!=s))return false;
 s->current=c;RRLocalRun *h=&s->host;double time=h->time;*h=(RRLocalRun){0};h->time=time;h->battle=b;h->phase=h->resume_phase=RR_SESSION_COUNTDOWN;h->countdown=3.999;h->last_count=4;h->last_nitro=-9;
 b->after_weapons=after_weapons;b->after_weapons_context=s;return true;
}
void rr_run_current_detach(RRRunCurrent *s){if(!s)return;if(s->current&&s->current->battle){RRLocalBattle *b=s->current->battle;if(b->after_weapons==after_weapons&&b->after_weapons_context==s){b->after_weapons=NULL;b->after_weapons_context=NULL;}}s->current=NULL;s->host.battle=NULL;}
bool rr_run_current_pause(RRRunCurrent *s){return s&&s->current&&rr_local_run_pause(&s->host);}
bool rr_run_current_resume(RRRunCurrent *s){return s&&s->current&&rr_local_run_resume(&s->host);}
static bool segment_world(void *p,RRVec3 a,RRVec3 b,RRVec3 *hit){return rr_world_segment(p,a,b,.6,hit);}
bool rr_run_current_tick(RRRunCurrent *s,RRRunInput input,double raw,const struct RRWorld *world,RRShotWorld segment,void *context,RRRunEvents *events){
 if(!s||!s->current||!s->host.battle||s->host.error||s->current->error||!events)return false;
 RRLocalRun *h=&s->host;RRLocalBattle *b=h->battle;if(b->after_weapons!=after_weapons||b->after_weapons_context!=s)return false;RRLocalRacer *r=b->fire.player;*events=(RRRunEvents){0};h->current_events=0;
 double dt=rr_frame_dt(raw);if(dt<=0)return true;
 if(h->phase!=RR_SESSION_COUNTDOWN&&h->phase!=RR_SESSION_ACTIVE&&h->phase!=RR_SESSION_FINISHED&&h->phase!=RR_SESSION_PAUSED)return false;
 h->time+=dt;h->current_input=input;RRPackBranch branch=RR_PACK_PLAYING;
 if(h->phase==RR_SESSION_PAUSED){branch=RR_PACK_PAUSED;if(!rr_player_phase_idle(s->current,dt))h->error=true;}
 else if(h->phase==RR_SESSION_COUNTDOWN){branch=RR_PACK_COUNTDOWN;h->countdown-=dt;h->last_count=(int)ceil(h->countdown);if(h->last_count<=0){h->phase=RR_SESSION_ACTIVE;h->current_events|=RR_RUN_GO;}if(!rr_player_phase_countdown(s->current,dt,world,&events->motion))h->error=true;}
 else{
  h->race_time+=dt;bool was=r->car.boosting;unsigned flags=0;
  if(!rr_player_phase_step(s->current,input.control,dt,h->race_time,world,&events->motion,&flags))h->error=true;h->current_events|=flags;
  if(flags&RR_RACE_FINISH)h->phase=RR_SESSION_FINISHED;
  if(r->car.boosting&&!was&&h->race_time-h->last_nitro>1){h->last_nitro=h->race_time;b->pack->shake=fmin(1.2,b->pack->shake+.35);h->current_events|=RR_RUN_NITRO;}
  if(events->motion.impact>4){b->pack->shake=fmin(1.2,b->pack->shake+events->motion.impact/25);rr_fire_effect(&b->fire,RR_FX_SPARK);h->current_events|=RR_RUN_IMPACT;}
 }
 if(h->error)return false;b->fire.aim=input.aim;
 if(!segment&&world){segment=segment_world;context=(void *)world;}
 if(!rr_battle_current_after_player(s->current,branch,h->phase==RR_SESSION_ACTIVE,dt,h->time,h->race_time,world,segment,context,input.weapons,&events->pack))h->error=true;
 events->player=h->current_events;return !h->error;
}

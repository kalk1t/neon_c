#include "rr_local_run.h"
#include "rr_physics.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
static bool pair(void *p,int index,double values[2]){RRFireLocal *f=p;(void)index;values[0]=f->random(f->random_ctx);values[1]=f->random(f->random_ctx);return true;}
static void respawn(RRLocalRun *s){RRLocalRacer *r=s->battle->fire.player;rr_car_place(&r->car,r->race.safe_position,r->race.safe_yaw,35);r->visible=true;r->previous=r->car.pos;r->hp=100;r->wreck=0;r->ai.sample=rr_track_nearest(s->battle->pack->track,r->car.pos);r->lost=r->stuck=0;s->current_events|=RR_RUN_RESPAWN;}
static void after_weapons(void *p){RRLocalRun *s=p;RRLocalRacer *r=s->battle->fire.player;if(s->current_input.manual_reset&&s->current_input.manual_reset_allowed&&r->wreck<=0){respawn(s);s->current_events|=RR_RUN_MANUAL;}}
bool rr_local_run_start(RRLocalRun *s,RRLocalBattle *b,const RRTrack *track,RRLocalRacer *player){
 if(!s||!b||!track||!b->pack||!player||!player->is_player||!b->fire.random)return false;
 s->battle=b;s->error=false;s->race_time=0;s->last_nitro=-9;s->countdown=3.999;s->last_count=4;s->phase=RR_SESSION_COUNTDOWN;s->resume_phase=RR_SESSION_COUNTDOWN;s->current_events=0;
 rr_battle_restart(b);RRVec3 spawn;rr_pack_grid(7,&spawn);rr_car_place(&player->car,spawn,0,0);player->visible=true;player->car.boost=1;player->previous=spawn;rr_race_reset(&player->race,RR_RACE_TWO_LAPS,spawn,0);player->passed=0;player->lap_count=0;player->hp=100;player->wreck=player->lost=player->stuck=player->wrong_way=player->smoke_time=0;player->takedowns=player->coins=0;player->fire_cd=4+b->fire.random(b->fire.random_ctx)*(4/1.5);player->react=.1+b->fire.random(b->fire.random_ctx)*.45;player->ai.sample=rr_track_nearest(track,spawn);player->error=false;
 if(!rr_pack_start(b->pack,track,player,pair,&b->fire)){s->error=true;return false;}rr_battle_bind(b,b->pack);b->after_weapons=after_weapons;b->after_weapons_context=s;return true;
}
bool rr_local_run_pause(RRLocalRun *s){if(s->phase!=RR_SESSION_ACTIVE&&s->phase!=RR_SESSION_COUNTDOWN)return false;s->resume_phase=s->phase;s->phase=RR_SESSION_PAUSED;return true;}
bool rr_local_run_resume(RRLocalRun *s){if(s->phase!=RR_SESSION_PAUSED)return false;s->phase=s->resume_phase;return true;}
static bool remember(RRLocalRacer *r,double value){if(r->lap_count==r->lap_capacity){if(r->lap_capacity>SIZE_MAX/2/sizeof(double))return false;size_t cap=r->lap_capacity?r->lap_capacity*2:8;double *p=realloc(r->lap_times,cap*sizeof(double));if(!p)return false;r->lap_times=p;r->lap_capacity=cap;}r->lap_times[r->lap_count++]=value;return true;}
static unsigned player_step(RRLocalRun *s,RRInput input,double dt,const struct RRWorld *world,RRCarStepReport *motion){
 RRLocalRacer *r=s->battle->fire.player;const RRTrack *track=s->battle->pack->track;
 if(!(r->wreck<=0)){r->wreck-=dt;if(r->wreck<=0)respawn(s);return 0;}
 r->previous=r->car.pos;rr_car_step_world_report(&r->car,input,dt,world,motion);if(!r->race.finished)r->race.lap_time+=dt;
 unsigned flags=rr_race_step(&r->race,track->gates,track->gate_count,&r->car,r->previous,0,s->race_time);
 if(flags&RR_RACE_CHECKPOINT){int i=r->race.next_checkpoint-1;if(i>=0&&i<32)r->passed|=UINT32_C(1)<<(unsigned)i;}
 if(flags&RR_RACE_LAP){r->passed=0;if(!remember(r,r->race.last_lap_time)){s->error=true;return flags|RR_RUN_ERROR;}}
 if(flags&RR_RACE_FINISH)s->phase=RR_SESSION_FINISHED;
 r->ai.sample=rr_track_nearest_local(track,r->car.pos,r->ai.sample);double along=rr_dot(r->car.vel,track->tangents[r->ai.sample]);r->wrong_way=along< -15?r->wrong_way+dt:0;
 double off=rr_length(rr_sub(r->car.pos,track->samples[r->ai.sample]));if(r->car.pos.y<34||off>650){r->lost+=dt;if(r->lost>1.1)respawn(s);}else r->lost=0;
 if(r->hp<60){r->smoke_time-=dt;if(r->smoke_time<=0){r->smoke_time=.04+r->hp/500;s->current_events|=RR_RUN_SMOKE;if(!rr_fx_request(&s->battle->fire.fx,RR_FX_SMOKE,s->battle->fire.random,s->battle->fire.random_ctx))s->error=true;}}
 return flags;
}
static bool segment_world(void *p,RRVec3 a,RRVec3 b,RRVec3 *hit){return rr_world_segment(p,a,b,.6,hit);}
bool rr_local_run_tick(RRLocalRun *s,RRRunInput input,double raw_dt,const struct RRWorld *world,RRShotWorld segment,void *segment_context,RRRunEvents *events){
 if(!s||!s->battle||!events||s->error)return false;*events=(RRRunEvents){0};double dt=rr_frame_dt(raw_dt);if(dt<=0)return true;
 if(s->phase!=RR_SESSION_COUNTDOWN&&s->phase!=RR_SESSION_ACTIVE&&s->phase!=RR_SESSION_FINISHED&&s->phase!=RR_SESSION_PAUSED)return false;
 s->time+=dt;s->current_events=0;s->current_input=input;RRLocalBattle *b=s->battle;RRLocalRacer *r=b->fire.player;RRPackBranch branch=RR_PACK_PLAYING;
 if(s->phase==RR_SESSION_PAUSED){branch=RR_PACK_PAUSED;r->car.t+=dt;}
 else if(s->phase==RR_SESSION_COUNTDOWN){branch=RR_PACK_COUNTDOWN;s->countdown-=dt;s->last_count=(int)ceil(s->countdown);if(s->last_count<=0){s->phase=RR_SESSION_ACTIVE;s->current_events|=RR_RUN_GO;}rr_car_step_world_report(&r->car,(RRInput){0},dt,world,&events->motion);}
 else {s->race_time+=dt;bool was=r->car.boosting;s->current_events|=player_step(s,input.control,dt,world,&events->motion);
  if(r->car.boosting&&!was&&s->race_time-s->last_nitro>1){s->last_nitro=s->race_time;b->pack->shake=fmin(1.2,b->pack->shake+.35);s->current_events|=RR_RUN_NITRO;}
  if(events->motion.impact>4){b->pack->shake=fmin(1.2,b->pack->shake+events->motion.impact/25);s->current_events|=RR_RUN_IMPACT;if(!rr_fx_request(&b->fire.fx,RR_FX_SPARK,b->fire.random,b->fire.random_ctx))s->error=true;}
 }
 b->fire.aim=input.aim;b->fire.muzzle=rr_combat_muzzle(&r->car);b->fire.side_axis=rr_combat_side_axis(&r->car);
 if(!segment&&world){segment=segment_world;segment_context=(void *)world;}
 if(!rr_battle_after_player(b,branch,s->phase==RR_SESSION_ACTIVE,dt,s->time,s->race_time,world,segment,segment_context,input.weapons,&events->pack))s->error=true;
 events->player=s->current_events;return !s->error;
}

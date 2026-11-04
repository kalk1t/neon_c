#include "rr_player_phase.h"
#include <stdint.h>
#include <stdlib.h>
static bool valid(RRBattleCurrent *c){return c&&!c->error&&c->battle&&c->battle->pack&&c->battle->pack->track&&c->battle->fire.player&&c->battle->fire.player->is_player&&c->battle->fire.player->race.mode!=RR_RACE_FREE_ROAM&&c->pack.poses[0].ready;}
static bool delta(double dt){return isfinite(dt)&&dt>0&&dt<=.05;}
bool rr_player_phase_respawn(RRBattleCurrent *c){
 if(!valid(c))return false;RRLocalRacer *r=c->battle->fire.player;const RRTrack *t=c->battle->pack->track;
 rr_car_place(&r->car,r->race.safe_position,r->race.safe_yaw,35);r->visible=true;r->previous=r->car.pos;r->hp=100;r->wreck=0;r->ai.sample=rr_track_nearest(t,r->car.pos);r->lost=r->stuck=0;
 if(!rr_car_pose_sync(&c->pack.poses[0],&r->car)){c->error=true;return false;}return true;
}
static bool remember(RRLocalRacer *r,double elapsed){
 if(r->lap_count==r->lap_capacity){if(r->lap_capacity>SIZE_MAX/2/sizeof(double))return false;size_t n=r->lap_capacity?r->lap_capacity*2:8;double *history=realloc(r->lap_times,n*sizeof(double));if(!history)return false;r->lap_times=history;r->lap_capacity=n;}
 r->lap_times[r->lap_count++]=elapsed;return true;
}
bool rr_player_phase_step(RRBattleCurrent *c,RRInput input,double dt,double time,const struct RRWorld *world,RRCarStepReport *motion,unsigned *events){
 if(!valid(c)||!delta(dt)||!isfinite(time)||!motion||!events)return false;*motion=(RRCarStepReport){0};*events=0;
 RRLocalRacer *r=c->battle->fire.player;const RRTrack *t=c->battle->pack->track;
 if(r->wreck>0){r->wreck-=dt;if(r->wreck<=0){if(!rr_player_phase_respawn(c))return false;*events|=RR_RUN_RESPAWN;}return true;}
 double before=r->car.speed;r->previous=r->car.pos;rr_car_step_current_world_report(&r->car,input,dt,world,motion);
 if(!rr_car_pose_after_step(&c->pack.poses[0],&r->car,before)){c->error=true;return false;}
 if(!r->race.finished)r->race.lap_time+=dt;
 unsigned flags=rr_race_step(&r->race,t->gates,t->gate_count,&r->car,r->previous,0,time);*events|=flags;
 if(flags&RR_RACE_CHECKPOINT){int i=r->race.next_checkpoint-1;if(i>=0&&i<32)r->passed|=UINT32_C(1)<<(unsigned)i;}
 if(flags&RR_RACE_LAP){r->passed=0;if(!remember(r,r->race.last_lap_time)){c->error=true;return false;}}
 if((flags&RR_RACE_FINISH)&&!rr_pack_current_rank(&c->pack)){c->error=true;return false;}
 r->ai.sample=rr_track_nearest_local(t,r->car.pos,r->ai.sample);double along=rr_dot(r->car.vel,t->tangents[r->ai.sample]);r->wrong_way=along< -15?r->wrong_way+dt:0;
 double off=rr_length(rr_sub(r->car.pos,t->samples[r->ai.sample]));if(r->car.pos.y<34||off>650){r->lost+=dt;if(r->lost>1.1){if(!rr_player_phase_respawn(c))return false;*events|=RR_RUN_RESPAWN;}}else r->lost=0;
 if(r->hp<60){r->smoke_time-=dt;if(r->smoke_time<=0){r->smoke_time=.04+r->hp/500;*events|=RR_RUN_SMOKE;rr_fire_effect(&c->battle->fire,RR_FX_SMOKE);}}
 if(c->battle->fire.error||c->battle->fire.fx.error){c->error=true;return false;}return true;
}
bool rr_player_phase_countdown(RRBattleCurrent *c,double dt,const struct RRWorld *world,RRCarStepReport *motion){
 if(!valid(c)||!delta(dt)||!motion)return false;RRCar *car=&c->battle->fire.player->car;double before=car->speed;
 rr_car_step_current_world_report(car,(RRInput){0},dt,world,motion);if(!rr_car_pose_after_step(&c->pack.poses[0],car,before)){c->error=true;return false;}return true;
}
bool rr_player_phase_idle(RRBattleCurrent *c,double dt){
 if(!valid(c)||!delta(dt))return false;RRCar *car=&c->battle->fire.player->car;if(!isfinite(car->t+dt)){c->error=true;return false;}car->t+=dt;
 if(!rr_car_pose_idle(&c->pack.poses[0],car->t)){c->error=true;return false;}return true;
}

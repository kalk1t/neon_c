#include "rr_run_input.h"
#include "rr_finish.h"
bool rr_run_input_center(RRRunInputState *s,const RRCar *car){
 if(!s||!car||!isfinite(car->yaw)||!isfinite(car->pitch))return false;
 if(!s->initialized){*s=(RRRunInputState){0};s->aim.direction=rr_v3(0,0,-1);s->initialized=true;}
 s->aim.yaw=car->yaw;s->aim.pitch=car->pitch;return true;
}
bool rr_run_input_tick(RRRunInputState *s,RRRunCurrent *run,RRRunInputFrame f,RRMouseSettings settings,double raw,const struct RRWorld *world,RRShotWorld segment,void *context,RRRunEvents *events){
 if(!s||!s->initialized||!run||!run->current||!run->host.battle||run->host.error||run->current->error||!events)return false;
 if(rr_frame_dt(raw)<=0)return rr_run_current_tick(run,(RRRunInput){0},raw,world,segment,context,events);
 if(!isfinite(f.dx)||!isfinite(f.dy)||!isfinite(settings.sensitivity)||settings.sensitivity<.3||settings.sensitivity>2.5)return false;
 RRLocalRacer *player=run->host.battle->fire.player;RRInput control=rr_input_mix_capture(f.keys,f.pad,f.captured);bool fire=f.fire;
 if(f.center){if(!rr_run_input_center(s,&player->car))return false;f.dx=f.dy=0;}
 if(run->host.phase==RR_SESSION_FINISHED||(run->host.phase==RR_SESSION_ACTIVE&&f.autopilot)){
  RRFinishAim aim;control=rr_finish_player_controls(run->host.battle->pack->track,&player->car,player->ai.sample,&aim);s->aim.yaw=aim.yaw;s->aim.pitch=aim.pitch;s->aim.direction=aim.direction;fire=false;
 }else if(run->host.phase==RR_SESSION_ACTIVE){
  control=rr_mouse_aim_step(&s->aim,settings,player->car.yaw,player->car.pitch,control,f.dx,f.dy,f.look);s->mouse_camera=f.captured;
 }
 RRRunInput input={control,{fire,f.missile,f.spin,f.emp,false,control.steer,0},s->aim.direction,f.reset,f.reset_allowed};
 if(!rr_run_current_tick(run,input,raw,world,segment,context,events))return false;
 if(events->player&RR_RUN_RESPAWN){s->aim.yaw=player->race.safe_yaw;s->aim.pitch=0;}
 return true;
}

#include "rr_session.h"
#include <stdint.h>
#include <stdlib.h>

void rr_session_init(RRSession *s){*s=(RRSession){0};}
void rr_session_dispose(RRSession *s){free(s->lap_times);rr_session_init(s);}
void rr_session_menu(RRSession *s){s->phase=RR_SESSION_MENU;s->resume_phase=RR_SESSION_MENU;s->finish_wait=0;}
void rr_session_start(RRSession *s,RRCar *car,RRRaceMode mode)
{
    double *history=s->lap_times;size_t capacity=s->lap_capacity;
    *s=(RRSession){0};s->lap_times=history;s->lap_capacity=capacity;
    RRVec3 spawn=mode==RR_RACE_TWO_LAPS?rr_v3(8,115.5,64):rr_v3(0,115.5,42);
    rr_car_reset(car,spawn,0,0);rr_race_reset(&s->race,mode,spawn,0);
    s->phase=mode==RR_RACE_FREE_ROAM?RR_SESSION_ACTIVE:RR_SESSION_COUNTDOWN;
    s->countdown=3.999;s->last_count=4;
}
bool rr_session_pause(RRSession *s)
{
    if(s->phase!=RR_SESSION_ACTIVE && s->phase!=RR_SESSION_COUNTDOWN && s->phase!=RR_SESSION_FINISHED && s->phase!=RR_SESSION_RESULTS)return false;
    s->resume_phase=s->phase;s->phase=RR_SESSION_PAUSED;return true;
}
bool rr_session_resume(RRSession *s)
{
    if(s->phase!=RR_SESSION_PAUSED)return false;
    s->phase=s->resume_phase;return true;
}
void rr_session_respawn(const RRSession *s,RRCar *car)
{
    rr_car_place(car,s->race.safe_position,s->race.safe_yaw,35);
}
static bool remember_lap(RRSession *s,double elapsed)
{
    if(s->lap_count==s->lap_capacity){
        if(s->lap_capacity>SIZE_MAX/2/sizeof(double))return false;
        size_t capacity=s->lap_capacity?s->lap_capacity*2:8;
        double *history=realloc(s->lap_times,capacity*sizeof(double));
        if(!history)return false;
        s->lap_times=history;s->lap_capacity=capacity;
    }
    s->lap_times[s->lap_count++]=elapsed;return true;
}
unsigned rr_session_step(RRSession *s,RRCar *car,const RRGate *gates,
                         int count,RRInput input,double raw_dt)
{
    double dt=rr_frame_dt(raw_dt);
    if(dt<=0)return 0;
    if(s->phase==RR_SESSION_FINISHED || s->phase==RR_SESSION_RESULTS){
        s->race_time+=dt;rr_car_step(car,input,dt);
        if(s->phase==RR_SESSION_RESULTS)return 0;
        s->finish_wait+=raw_dt;
        if(s->finish_wait>=2){s->phase=RR_SESSION_RESULTS;return RR_SESSION_SHOW_RESULTS;}
        return 0;
    }
    if(s->phase==RR_SESSION_COUNTDOWN){
        s->countdown-=dt;s->last_count=(int)ceil(s->countdown);
        rr_car_step(car,(RRInput){0},dt);
        if(s->last_count<=0){s->phase=RR_SESSION_ACTIVE;return RR_SESSION_GO;}
        return 0;
    }
    if(s->phase!=RR_SESSION_ACTIVE)return 0;
    RRVec3 previous=car->pos;
    s->race_time+=dt;rr_car_step(car,input,dt);
    unsigned events=rr_race_step(&s->race,gates,count,car,previous,dt,s->race_time);
    if((events&RR_RACE_LAP) && !remember_lap(s,s->race.last_lap_time)){
        s->phase=RR_SESSION_ERROR;return events;
    }
    if(events&RR_RACE_FINISH){s->phase=RR_SESSION_FINISHED;s->finish_wait=0;}
    return events;
}
bool rr_session_target(const RRSession *s,const RRGate *gates,int count,RRVec3 *target)
{
    if(count<=0 || s->race.next_checkpoint<0 || s->race.next_checkpoint>count ||
       s->race.mode==RR_RACE_FREE_ROAM || s->race.finished || s->phase==RR_SESSION_MENU ||
       s->phase==RR_SESSION_RESULTS || s->phase==RR_SESSION_ERROR)return false;
    *target=s->race.next_checkpoint<count?gates[s->race.next_checkpoint].pos:rr_v3(0,126,0);
    return true;
}

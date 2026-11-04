#include "rr_race.h"

void rr_race_reset(RRRace *race, RRRaceMode mode, RRVec3 spawn, double yaw)
{
    *race=(RRRace){0};
    race->mode=mode;race->lap=1;
    race->safe_position=spawn;race->safe_yaw=yaw;
}

static unsigned step(RRRace *race,const RRGate *gates,int count,
                      RRCar *car,RRVec3 previous,double dt,double race_time,double gate_radius)
{
    unsigned events=RR_RACE_NONE;
    if(!race->finished)race->lap_time+=dt;
    if(race->finished || race->mode==RR_RACE_FREE_ROAM)return events;
    if(race->next_checkpoint<count){
        const RRGate *gate=&gates[race->next_checkpoint];
        double a=rr_dot(rr_sub(previous,gate->pos),gate->forward);
        double b=rr_dot(rr_sub(car->pos,gate->pos),gate->forward);
        RRVec3 radial=rr_sub(rr_sub(car->pos,gate->pos),rr_scale(gate->forward,b));
        if(a<0 && b>=0 && rr_length(radial)<gate_radius){
            race->next_checkpoint++;
            race->safe_position=rr_add(gate->pos,rr_scale(gate->forward,12));
            race->safe_yaw=gate->yaw;
            car->boost=fmin(1,car->boost+0.3);
            events|=RR_RACE_CHECKPOINT;
        }
    }
    /* GATE in browser main.js: x +/-23, deck 111, y (109,142), z=0. */
    if(previous.z>0 && car->pos.z<=0 && fabs(car->pos.x)<23 &&
       car->pos.y>109 && car->pos.y<142 && race->next_checkpoint>=count){
        race->last_lap_time=race->lap_time;
        if(!race->completed_laps || race->last_lap_time<race->best_lap_time)
            race->best_lap_time=race->last_lap_time;
        race->completed_laps++;race->lap_time=0;race->lap++;
        race->next_checkpoint=0;events|=RR_RACE_LAP;
        if(race->mode==RR_RACE_TWO_LAPS && race->lap>2){
            race->finished=true;race->finish_time=race_time;events|=RR_RACE_FINISH;
        }
    }
    return events;
}

unsigned rr_race_step(RRRace *r,const RRGate *g,int n,RRCar *c,RRVec3 previous,double dt,double time){return step(r,g,n,c,previous,dt,time,15);}
unsigned rr_race_step_rival_current(RRRace *r,const RRGate *g,int n,RRCar *c,RRVec3 previous,double dt,double time){return step(r,g,n,c,previous,dt,time,45);}

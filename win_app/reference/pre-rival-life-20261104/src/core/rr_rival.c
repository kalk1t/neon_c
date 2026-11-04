#include "rr_rival.h"
#include <stdlib.h>
#include <stdint.h>
static bool random_valid(double r){return isfinite(r)&&r>=0&&r<1;}
static bool local_rival(const RRLocalRacer *r,const RRTrack *track)
{return r&&track&&track->sample_count>0&&!r->is_player&&r->race.mode!=RR_RACE_FREE_ROAM;}
void rr_rival_dispose(RRLocalRacer *r){if(r){free(r->lap_times);r->lap_times=NULL;r->lap_count=r->lap_capacity=0;}}
bool rr_rival_reset(RRLocalRacer *r,const RRTrack *track,RRVec3 position,double yaw,RRRaceMode mode,double random_fire,double random_reaction)
{
    if(!r||!track||track->sample_count<=0||r->is_player||mode==RR_RACE_FREE_ROAM||!random_valid(random_fire)||!random_valid(random_reaction))return false;
    rr_car_place(&r->car,position,yaw,0);r->visible=true;r->car.boost=1;r->previous=position;
    rr_race_reset(&r->race,mode,position,yaw);r->passed=0;r->lap_count=0;r->hp=100;r->wreck=r->lost=r->stuck=r->wrong_way=r->smoke_time=0;
    r->takedowns=r->coins=0;r->fire_cd=6/1.5+random_fire*(4/1.5);r->react=.1+random_reaction*.45;r->ai.sample=rr_track_nearest(track,position);r->error=false;return true;
}
bool rr_rival_respawn(RRLocalRacer *r,const RRTrack *track,bool keep_hp,double random_x,double random_y)
{
    if(!local_rival(r,track)||!random_valid(random_x)||!random_valid(random_y))return false;
    RRVec3 pos=r->race.safe_position;pos.x+=(random_x-.5)*12;pos.y+=(random_y-.5)*6;
    rr_car_place(&r->car,pos,r->race.safe_yaw,35);r->visible=true;r->previous=pos;if(!keep_hp)r->hp=100;r->wreck=0;r->ai.sample=rr_track_nearest(track,pos);r->lost=r->stuck=0;return true;
}
typedef struct FixedRandom {double x,y;} FixedRandom;
static bool fixed_random(void *context,double values[2]){const FixedRandom *f=context;values[0]=f->x;values[1]=f->y;return true;}
static bool respawn_random(RRLocalRacer *r,const RRTrack *track,RRRivalRandom random,void *context)
{
    double values[2];if(!random||!random(context,values)||!rr_rival_respawn(r,track,false,values[0],values[1])){r->error=true;return false;}return true;
}
static bool remember(RRLocalRacer *r,double time)
{
    if(r->lap_count==r->lap_capacity){if(r->lap_capacity>SIZE_MAX/2/sizeof(double))return false;size_t cap=r->lap_capacity?r->lap_capacity*2:8;double *p=realloc(r->lap_times,cap*sizeof(double));if(!p)return false;r->lap_times=p;r->lap_capacity=cap;}
    r->lap_times[r->lap_count++]=time;return true;
}
static unsigned rules_random(RRLocalRacer *r,const RRTrack *track,double dt,double race_time,double floor_height,RRRivalRandom random,void *context)
{
    if(!local_rival(r,track)||!isfinite(dt)||dt<0||dt>.05)return 0;
    unsigned events=rr_race_step(&r->race,track->gates,track->gate_count,&r->car,r->previous,0,race_time);
    if(events&RR_RACE_CHECKPOINT){int index=r->race.next_checkpoint-1;if(index>=0&&index<32)r->passed|=UINT32_C(1)<<(unsigned int)index;}
    if(events&RR_RACE_LAP){r->passed=0;if(!remember(r,r->race.last_lap_time)){r->error=true;return events|RR_RIVAL_ERROR;}}
    r->ai.sample=rr_track_nearest_local(track,r->car.pos,r->ai.sample);
    r->stuck=r->car.speed<12&&race_time>3?r->stuck+dt:0;
    if(r->stuck>2.5&&respawn_random(r,track,random,context))events|=RR_RIVAL_RESPAWN;
    double off=rr_length(rr_sub(r->car.pos,track->samples[r->ai.sample]));
    if(r->car.pos.y<floor_height||off>650){r->lost+=dt;if(r->lost>1.1&&respawn_random(r,track,random,context))events|=RR_RIVAL_RESPAWN;}else r->lost=0;
    return events|(r->error?RR_RIVAL_ERROR:0);
}
unsigned rr_rival_rules(RRLocalRacer *r,const RRTrack *track,double dt,double race_time,double floor_height,double random_x,double random_y)
{FixedRandom f={random_x,random_y};return rules_random(r,track,dt,race_time,floor_height,fixed_random,&f);}
unsigned rr_rival_step(RRLocalRacer *r,const RRTrack *track,const struct RRWorld *world,RRInput input,double dt,double race_time,double floor_height,double random_x,double random_y)
{FixedRandom f={random_x,random_y};return rr_rival_step_random(r,track,world,input,dt,race_time,floor_height,fixed_random,&f);}
unsigned rr_rival_step_random(RRLocalRacer *r,const RRTrack *track,const struct RRWorld *world,RRInput input,double dt,double race_time,double floor_height,RRRivalRandom random,void *context)
{
    if(!local_rival(r,track)||!isfinite(dt)||dt<=0||dt>.05)return 0;
    if(!(r->wreck<=0)){r->wreck-=dt;if(r->wreck<=0&&respawn_random(r,track,random,context))return RR_RIVAL_RESPAWN;return r->error?RR_RIVAL_ERROR:0;}
    r->previous=r->car.pos;rr_car_step_world(&r->car,input,dt,world);if(!r->race.finished)r->race.lap_time+=dt;
    unsigned events=rules_random(r,track,dt,race_time,floor_height,random,context);
    if(r->hp<60){r->smoke_time-=dt;if(r->smoke_time<=0){r->smoke_time=.04+r->hp/500;events|=RR_RIVAL_SMOKE;}}
    return events;
}
bool rr_local_car_contacts(RRLocalRacer *racers,size_t count,double shake,RRCarContacts *out)
{
    if(!out||(count&&!racers)||count>RR_LOCAL_MAX_CARS)return false;*out=(RRCarContacts){0};out->shake=shake;
    for(size_t i=0;i<count;i++){
        RRLocalRacer *a=&racers[i];if(!(a->wreck<=0))continue;
        for(size_t j=i+1;j<count;j++){
            RRLocalRacer *b=&racers[j];if(!(b->wreck<=0))continue;RRVec3 d=rr_sub(a->car.pos,b->car.pos);double dist=rr_length(d);if(dist>=4.6||dist<1e-3)continue;
            d=rr_scale(d,1/dist);a->car.pos=rr_add(a->car.pos,rr_scale(d,(4.6-dist)/2));b->car.pos=rr_add(b->car.pos,rr_scale(d,-(4.6-dist)/2));
            double vrel=rr_dot(d,rr_sub(a->car.vel,b->car.vel));bool hit=false;
            if(vrel<0){a->car.vel=rr_add(a->car.vel,rr_scale(d,-vrel*.6));b->car.vel=rr_add(b->car.vel,rr_scale(d,vrel*.6));if((a->is_player||b->is_player)&&-vrel>5){out->shake=fmin(1.2,out->shake-vrel/40);hit=true;}}
            out->events[out->count++]=(RRCarContactEvent){(int)i,(int)j,vrel,hit?-vrel*.6:0,hit};
        }
    }return true;
}

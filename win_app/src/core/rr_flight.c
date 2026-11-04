/* Port of browser Car flight equations. Graphics/collision response are separate. */
#include "rr_flight.h"
#include "rr_physics.h"
#include <string.h>

static const RRCarStats profiles[]={
 {1,1,1,100,1,1,1,6,3,1},
 {1.08,1.276,1.12,75,1.3,1,.8,4,3,1},
 {.94,.797,.9,140,.4,.6,2,6,4,1.25}
};
const RRCarStats *rr_car_stats(RRCarProfile profile){return &profiles[(unsigned)profile<=RR_PROFILE_TITAN?(unsigned)profile:0];}
void rr_car_use_stats(RRCar *car,const char *id){
 car->profile=id&&!strcmp(id,"viper")?RR_PROFILE_VIPER:id&&!strcmp(id,"titan")?RR_PROFILE_TITAN:RR_PROFILE_ROADSTER;
}

static void update_forward(RRCar *c)
{
    double cp = cos(c->pitch);
    c->fwd = rr_v3(-sin(c->yaw)*cp, sin(c->pitch), -cos(c->yaw)*cp);
}

double rr_frame_dt(double elapsed)
{
    return isfinite(elapsed) && elapsed > 0 ? fmin(elapsed, 1.0/20.0) : 0;
}

void rr_car_place(RRCar *c, RRVec3 pos, double yaw, double speed)
{
    c->pos=pos;c->yaw=yaw;c->pitch=0;c->roll=0;c->speed=speed;c->side=0;
    c->stun=0;c->roll_time=0;c->surge=0;
    update_forward(c);c->vel=rr_scale(c->fwd,speed);
}

void rr_car_reset(RRCar *c, RRVec3 pos, double yaw, double speed)
{
    memset(c, 0, sizeof(*c));
    c->boost=1; c->max_multiplier=1;
    c->roll_direction=1;
    rr_car_place(c,pos,yaw,speed);
}

static void dodge(RRCar *c,int side,double maximum)
{
    c->roll_time=RR_SPIN_ROLL;
    c->roll_direction=side<0 ? -1 : 1;
    c->surge=2;
    c->speed=fmax(c->speed, maximum*0.5)*1.5;
    c->vel.x += cos(c->yaw)*c->roll_direction*22;
    c->vel.z -= sin(c->yaw)*c->roll_direction*22;
}

void rr_car_dodge(RRCar *c,int side){dodge(c,side,RR_CAR_MAX);}
void rr_car_dodge_current(RRCar *c,int side){dodge(c,side,RR_CURRENT_CAR_MAX);}
void rr_car_step(RRCar *c, RRInput in, double dt){rr_car_step_world(c,in,dt,NULL);}
static void step(RRCar *c,RRInput in,double dt,const RRWorld *world,RRCarStepReport *report,bool current)
{
    if(report)*report=(RRCarStepReport){0};
    if (!isfinite(dt) || dt<=0 || dt>0.05) return;
    c->t+=dt;
    c->stun=fmax(0,c->stun-dt);
    c->roll_time=fmax(0,c->roll_time-dt);
    c->surge=fmax(0,c->surge-dt);
    const RRCarStats *stats=rr_car_stats(current?c->profile:RR_PROFILE_ROADSTER);
    double top=current?RR_CURRENT_CAR_MAX:RR_CAR_MAX;
    double sf=fmin(1,fabs(c->speed)/top);
    bool was=c->boosting;
    c->boosting=in.boost && c->boost>0.02 && in.throttle>0 && c->stun<=0;
    if(c->boosting && !was) c->kick=1;
    if(was && !c->boosting) c->sputter=0.45;
    c->nitro+=( (c->boosting ? 1.0 : 0.0)-c->nitro)*(1-exp(-(c->boosting?10:3.5)*dt));
    c->kick=fmax(0,c->kick-dt*2.4);
    c->sputter=fmax(0,c->sputter-dt);
    c->thrust+=(fmax(0,in.throttle)-c->thrust)*(1-exp(-8*dt));
    c->boost=rr_clamp(c->boost+(c->boosting ? -0.32 : 0.06)*dt,0,1);
    double maximum=(c->boosting?(current?RR_CURRENT_CAR_BOOST_MAX:RR_CAR_BOOST_MAX):top)*c->max_multiplier*stats->top*(c->surge>0?1.5:1);
    double accel=(c->boosting?75:36)*(current?RR_CURRENT_CAR_SCALE*c->max_multiplier:1)*stats->accel;
    if(in.throttle>0) c->speed+=accel*in.throttle*dt*fmax(0,1-pow(c->speed/maximum,2));
    else {double drag=9*dt;c->speed=fabs(c->speed)<=drag?0:c->speed-drag*(c->speed>0?1:-1);}
    if(in.brake>0) c->speed-=(c->speed>0?70:14)*in.brake*dt;
    if(c->speed>maximum) c->speed-=28*dt;
    c->speed=fmax(-18,c->speed);
    double turn=rr_lerp(1.9,1.15,sf)*(in.brake>0?1.25:1)*stats->turn;
    c->yaw-=in.steer*turn*dt*(c->speed<0&&!in.aim_steer?-1:1)*fmin(1,fabs(c->speed)/8+0.35);
    c->pitch+=(in.climb*0.62-c->pitch)*(1-exp(-3.2*dt));
    update_forward(c);
    double grip=in.brake>0 && fabs(in.steer)>0 ? 1.6 : 3.4;
    double side_target=(c->stun>0?0:in.strafe)*RR_STRAFE_SPEED;
    c->side+=(side_target-c->side)*(1-exp(-RR_STRAFE_RESPONSE*dt));
    RRVec3 desired=rr_scale(c->fwd,c->speed);
    desired.x+=cos(c->yaw)*c->side;desired.z-=sin(c->yaw)*c->side;
    c->vel=rr_mix(c->vel,desired,1-exp(-grip*dt));
    c->pos=rr_add(c->pos,rr_scale(c->vel,dt));
    c->impact=0;RRContact hit=rr_world_collide(world,&c->pos,2.1);
    if(hit.hit){
        double vn=rr_dot(c->vel,hit.normal);
        if(vn<0){
            c->vel=rr_add(c->vel,rr_scale(hit.normal,-vn*1.4));c->impact=-vn;
            if(c->impact>12)c->speed*=.7;
            c->speed=fmin(c->speed,fmax(0,rr_dot(c->vel,c->fwd))+6);
        }
    }
    if(report)*report=(RRCarStepReport){c->impact,hit.normal,hit.hit};
    double slip=rr_dot(rr_sub(c->vel,desired),rr_v3(cos(c->yaw),0,-sin(c->yaw)));
    double roll_target=-in.steer*0.62*fmin(1,fabs(c->speed)/35)-slip*0.01-(c->side/RR_STRAFE_SPEED)*RR_STRAFE_LEAN;
    c->roll+=(roll_target-c->roll)*(1-exp(-5*dt));
    double tilt=-.35*in.throttle*(c->boosting?1.6:1)+.3*in.brake;
    for(int i=0;i<4;i++){
        c->pod_pitch[i]+=((tilt+(i<2?-.1:0)*in.climb)*.45-c->pod_pitch[i])*(1-exp(-6*dt));
        c->pod_roll[i]=((i%2?1:-1)*in.steer*.15-(c->side/RR_STRAFE_SPEED)*.3)*.45;
    }
}

void rr_car_step_world(RRCar *c,RRInput in,double dt,const RRWorld *world){rr_car_step_world_report(c,in,dt,world,NULL);}

void rr_car_step_world_report(RRCar *c,RRInput in,double dt,const RRWorld *world,RRCarStepReport *report){step(c,in,dt,world,report,false);}
void rr_car_step_current_world_report(RRCar *c,RRInput in,double dt,const RRWorld *world,RRCarStepReport *report){step(c,in,dt,world,report,true);}
void rr_car_step_current(RRCar *c,RRInput in,double dt){step(c,in,dt,NULL,NULL,true);}

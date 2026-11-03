/* Port of browser Car flight equations. Graphics/collision response are separate. */
#include "rr_flight.h"
#include <string.h>

static void update_forward(RRCar *c)
{
    double cp = cos(c->pitch);
    c->fwd = rr_v3(-sin(c->yaw)*cp, sin(c->pitch), -cos(c->yaw)*cp);
}

double rr_frame_dt(double elapsed)
{
    return isfinite(elapsed) && elapsed > 0 ? fmin(elapsed, 1.0/20.0) : 0;
}

void rr_car_reset(RRCar *c, RRVec3 pos, double yaw, double speed)
{
    memset(c, 0, sizeof(*c));
    c->pos=pos; c->yaw=yaw; c->speed=speed; c->boost=1; c->max_multiplier=1;
    c->roll_direction=1;
    update_forward(c);
    c->vel=rr_scale(c->fwd, speed);
}

void rr_car_dodge(RRCar *c, int side)
{
    c->roll_time=RR_SPIN_ROLL;
    c->roll_direction=side<0 ? -1 : 1;
    c->surge=2;
    c->speed=fmax(c->speed, RR_CAR_MAX*0.5)*1.5;
    c->vel.x += cos(c->yaw)*c->roll_direction*22;
    c->vel.z -= sin(c->yaw)*c->roll_direction*22;
}

void rr_car_step(RRCar *c, RRInput in, double dt)
{
    if (!isfinite(dt) || dt<=0 || dt>0.05) return;
    c->t+=dt;
    c->stun=fmax(0,c->stun-dt);
    c->roll_time=fmax(0,c->roll_time-dt);
    c->surge=fmax(0,c->surge-dt);
    double sf=fmin(1,fabs(c->speed)/RR_CAR_MAX);
    bool was=c->boosting;
    c->boosting=in.boost && c->boost>0.02 && in.throttle>0 && c->stun<=0;
    if(c->boosting && !was) c->kick=1;
    if(was && !c->boosting) c->sputter=0.45;
    c->nitro+=( (c->boosting ? 1.0 : 0.0)-c->nitro)*(1-exp(-(c->boosting?10:3.5)*dt));
    c->kick=fmax(0,c->kick-dt*2.4);
    c->sputter=fmax(0,c->sputter-dt);
    c->thrust+=(fmax(0,in.throttle)-c->thrust)*(1-exp(-8*dt));
    c->boost=rr_clamp(c->boost+(c->boosting ? -0.32 : 0.06)*dt,0,1);
    double maximum=(c->boosting?RR_CAR_BOOST_MAX:RR_CAR_MAX)*c->max_multiplier*(c->surge>0?1.5:1);
    double accel=c->boosting?75:36;
    if(in.throttle>0) c->speed+=accel*in.throttle*dt*fmax(0,1-pow(c->speed/maximum,2));
    else c->speed-=9*dt*(c->speed>0?1:c->speed<0?-1:0);
    if(in.brake>0) c->speed-=(c->speed>0?70:14)*in.brake*dt;
    if(c->speed>maximum) c->speed-=28*dt;
    c->speed=fmax(-18,c->speed);
    double turn=rr_lerp(1.9,1.15,sf)*(in.brake>0?1.25:1);
    c->yaw-=in.steer*turn*dt*(c->speed<0?-1:1)*fmin(1,fabs(c->speed)/8+0.35);
    c->pitch+=(in.climb*0.62-c->pitch)*(1-exp(-3.2*dt));
    update_forward(c);
    double grip=in.brake>0 && fabs(in.steer)>0 ? 1.6 : 3.4;
    RRVec3 desired=rr_scale(c->fwd,c->speed);
    c->vel=rr_mix(c->vel,desired,1-exp(-grip*dt));
    c->pos=rr_add(c->pos,rr_scale(c->vel,dt));
    double slip=rr_dot(rr_sub(c->vel,desired),rr_v3(cos(c->yaw),0,-sin(c->yaw)));
    double roll_target=-in.steer*0.62*fmin(1,fabs(c->speed)/35)-slip*0.01;
    c->roll+=(roll_target-c->roll)*(1-exp(-5*dt));
}

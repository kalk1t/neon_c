#ifndef RR_MOUSE_AIM_H
#define RR_MOUSE_AIM_H
#include "rr_flight.h"
typedef struct RRMouseSettings {double sensitivity;bool invert;} RRMouseSettings;
typedef struct RRMouseAim {double yaw,pitch,look_yaw,look_pitch;bool looking;RRVec3 direction;} RRMouseAim;
/* Browser #sens range min0.3 max2.5 step0.05. No persistence/account writes. */
static inline double rr_mouse_sensitivity_at(double unit)
{return .3+floor(rr_clamp(unit,0,1)*44+.5)*.05;}
/* Source updateAim: takeMouse already consumed by the platform host. */
static inline RRInput rr_mouse_aim_step(RRMouseAim *a,RRMouseSettings settings,double car_yaw,double car_pitch,RRInput in,double dx,double dy,bool look)
{
    double k=.0021*settings.sensitivity,sign=settings.invert?-1:1;
    if(look){a->look_yaw-=dx*k*1.6;a->look_pitch=rr_clamp(a->look_pitch+dy*k*1.6*sign,-.45,1.15);}
    else{a->yaw-=dx*k;a->pitch-=dy*k*sign;}
    a->looking=look;bool key_steer=in.steer!=0,key_climb=in.climb!=0;
    if(key_steer)a->yaw=car_yaw;if(key_climb)a->pitch=car_pitch;
    double difference=atan2(sin(a->yaw-car_yaw),cos(a->yaw-car_yaw));
    difference=rr_clamp(difference,-1.1,1.1);a->yaw=car_yaw+difference;
    a->pitch=rr_clamp(a->pitch,-.62,.62);
    double cp=cos(a->pitch);a->direction=rr_v3(-sin(a->yaw)*cp,sin(a->pitch),-cos(a->yaw)*cp);
    if(!key_steer){in.steer=rr_clamp(-difference*2.8,-1,1);in.aim_steer=true;}
    if(!key_climb)in.climb=rr_clamp(a->pitch/.62,-1,1);
    return in;
}
#endif

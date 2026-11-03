#ifndef RR_CAMERA_MOTION_H
#define RR_CAMERA_MOTION_H
#include "rr_math.h"
#include <stdbool.h>
typedef struct RRCameraMotion {double yaw,pitch,look_yaw,look_pitch;} RRCameraMotion;
/* Browser chaseCamera timing: called once after player/physics, not in input. */
static inline void rr_camera_motion_step(RRCameraMotion *m,double car_yaw,double car_pitch,bool looking,double dt)
{
    double dy=atan2(sin(car_yaw-m->yaw),cos(car_yaw-m->yaw));
    m->yaw+=dy*(1-exp(-7*dt));
    m->pitch+=(car_pitch*.6-m->pitch)*(1-exp(-5*dt));
    if(!looking){m->look_yaw=atan2(sin(m->look_yaw),cos(m->look_yaw))*exp(-4*dt);m->look_pitch*=exp(-4*dt);}
}
#endif

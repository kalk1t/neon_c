#ifndef RR_CAMERA_MOTION_H
#define RR_CAMERA_MOTION_H
#include "rr_math.h"
#include <stdbool.h>
typedef struct RRCameraMotion {double yaw,pitch,look_yaw,look_pitch;} RRCameraMotion;
/* Browser chaseCamera timing: called once after player/physics, not in input. */
static inline void rr_camera_motion_aim_step(RRCameraMotion *m,double target_yaw,double target_pitch,bool aim_camera,bool looking,double dt)
{
    double dy=atan2(sin(target_yaw-m->yaw),cos(target_yaw-m->yaw));
    m->yaw+=dy*(1-exp(-(aim_camera?14:7)*dt));
    m->pitch+=(target_pitch*(aim_camera?.85:.6)-m->pitch)*(1-exp(-(aim_camera?10:5)*dt));
    if(!looking){m->look_yaw=atan2(sin(m->look_yaw),cos(m->look_yaw))*exp(-4*dt);m->look_pitch*=exp(-4*dt);}
}
/* Uncaptured/legacy caller retains browser car-follow timing. */
static inline void rr_camera_motion_step(RRCameraMotion *m,double car_yaw,double car_pitch,bool looking,double dt)
{rr_camera_motion_aim_step(m,car_yaw,car_pitch,false,looking,dt);}
#endif

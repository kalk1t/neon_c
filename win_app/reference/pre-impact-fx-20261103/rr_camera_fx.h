#ifndef RR_CAMERA_FX_H
#define RR_CAMERA_FX_H
#include "rr_first_person.h"
typedef struct RRCameraFX {double last_nitro,kick,shake;} RRCameraFX;
typedef struct RRCameraPulse {double pitch,yaw,fov_add;} RRCameraPulse;
/* Source restarts reset lastNitro, retaining the camera's existing kick/shake. */
static inline void rr_camera_fx_restart(RRCameraFX *fx){fx->last_nitro=-9;}
static inline bool rr_camera_fx_ignite(RRCameraFX *fx,bool was,bool boosting,double race_time)
{
    if(!(boosting&&!was&&race_time-fx->last_nitro>1))return false;
    fx->last_nitro=race_time;fx->kick=1;fx->shake=fmin(1.2,fx->shake+.35);return true;
}
static inline RRCameraPulse rr_camera_fx_step(RRCameraFX *fx,double nitro,double dt,double time_ms)
{
    RRCameraPulse p={0};double shake=fx->shake+.08*nitro,t=time_ms*.05;
    if(shake>.001){p.pitch=sin(t*1.7)*.012*shake;p.yaw=cos(t*2.3)*.012*shake;}
    fx->shake=fmax(0,fx->shake-dt*2.2);fx->kick=fmax(0,fx->kick-dt*2.2);
    p.fov_add=fx->kick*fx->kick*10;return p;
}
static inline RRFirstPerson rr_camera_fx_apply(RRFirstPerson p,RRCameraPulse pulse)
{
    RRVec3 f=rr_normalize(p.forward),right=rr_normalize(rr_v3(f.y*p.up.z-f.z*p.up.y,f.z*p.up.x-f.x*p.up.z,f.x*p.up.y-f.y*p.up.x));
    RRVec3 up=rr_v3(right.y*f.z-right.z*f.y,right.z*f.x-right.x*f.z,right.x*f.y-right.y*f.x);
    RRVec3 tilted=rr_add(rr_scale(f,cos(pulse.pitch)),rr_scale(up,sin(pulse.pitch)));
    p.up=rr_sub(rr_scale(up,cos(pulse.pitch)),rr_scale(f,sin(pulse.pitch)));
    p.forward=rr_sub(rr_scale(tilted,cos(pulse.yaw)),rr_scale(right,sin(pulse.yaw)));return p;
}
#endif

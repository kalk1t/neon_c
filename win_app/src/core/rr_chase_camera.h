#ifndef RR_CHASE_CAMERA_H
#define RR_CHASE_CAMERA_H
#include "rr_first_person.h"
/* Three lookAt with world-up, followed by local rotateZ. No effects/RNG. */
static inline RRFirstPerson rr_chase_pose_tuned(const RRCar *car,int mode,bool alive,
    double camera_yaw,double camera_pitch,double look_yaw,double look_pitch,bool aim_camera,double maximum)
{
    double sf=fmin(1.4,fmax(0,car->speed)/maximum);
    double orbit=rr_camera_orbit(look_yaw,look_pitch);
    double yaw=camera_yaw+look_yaw,pitch=rr_clamp(camera_pitch-look_pitch,-1.3,1.3);
    RRVec3 f=rr_v3(-sin(yaw)*cos(pitch),sin(pitch),-cos(yaw)*cos(pitch));
    double d=(mode==1?15:9)*(1+sf*.22)+(alive?0:14);
    RRVec3 pos=rr_sub(car->pos,rr_scale(f,d));pos.y+=(mode==1?4.8:2.7)-sf*.6+(alive?0:6);
    RRVec3 target=rr_add(rr_add(car->pos,rr_scale(aim_camera?f:car->fwd,9*(1-orbit))),rr_v3(0,.9,0));
    RRVec3 z=rr_sub(pos,target);
    if(rr_dot(z,z)==0)z.z=1;
    z=rr_normalize(z);RRVec3 right=rr_v3(z.z,0,-z.x);
    if(rr_dot(right,right)==0){z.z+=.0001;z=rr_normalize(z);right=rr_v3(z.z,0,-z.x);}
    right=rr_normalize(right);
    RRVec3 up=rr_v3(-z.z*right.y+z.y*right.z,z.z*right.x-z.x*right.z,-z.y*right.x+z.x*right.y);
    double bank=car->roll*.28*(1-orbit);
    RRFirstPerson p={pos,rr_scale(z,-1),rr_sub(rr_scale(up,cos(bank)),rr_scale(right,sin(bank)))};
    return p;
}
static inline RRFirstPerson rr_chase_pose_aim(const RRCar *c,int mode,bool alive,double yaw,double pitch,double ly,double lp,bool aim){return rr_chase_pose_tuned(c,mode,alive,yaw,pitch,ly,lp,aim,RR_CAR_MAX);}
static inline RRFirstPerson rr_chase_pose_current_aim(const RRCar *c,int mode,bool alive,double yaw,double pitch,double ly,double lp,bool aim){return rr_chase_pose_tuned(c,mode,alive,yaw,pitch,ly,lp,aim,RR_CURRENT_CAR_MAX);}
static inline RRFirstPerson rr_chase_pose(const RRCar *car,int mode,bool alive,double yaw,double pitch,double look_yaw,double look_pitch)
{return rr_chase_pose_aim(car,mode,alive,yaw,pitch,look_yaw,look_pitch,false);}
#endif

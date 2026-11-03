#ifndef RR_FIRST_PERSON_H
#define RR_FIRST_PERSON_H
#include "rr_flight.h"
typedef struct RRFirstPerson {RRVec3 position,forward,up;} RRFirstPerson;
static inline int rr_camera_next(int mode){return (mode+1)%3;}
static inline double rr_camera_orbit(double yaw,double pitch)
{return fmin(1,(fabs(atan2(sin(yaw),cos(yaw)))+fabs(pitch))*4);}
static inline bool rr_first_person_active(int mode,bool alive,double yaw,double pitch)
{return mode==2&&alive&&rr_camera_orbit(yaw,pitch)<.05;}
static inline RRFirstPerson rr_first_person_pose(const RRCar *car,double orbit)
{
    RRVec3 f=rr_normalize(car->fwd),right=rr_normalize(rr_v3(-f.z,0,f.x));
    RRVec3 up=rr_v3(-right.z*f.y,right.z*f.x-right.x*f.z,right.x*f.y);
    double roll=car->roll*.28*(1-orbit);
    RRFirstPerson p={rr_add(rr_add(car->pos,rr_scale(car->fwd,.6)),rr_v3(0,.75,0)),f,
        rr_sub(rr_scale(up,cos(roll)),rr_scale(right,sin(roll)))};
    return p;
}
#endif

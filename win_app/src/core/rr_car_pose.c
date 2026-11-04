#include "rr_car_pose.h"
#include <string.h>
bool rr_car_pose_sync(RRCarPose *p,const RRCar *c){
 if(!p||!c)return false;
 double k=1-c->roll_time/.55,spin=c->roll_time>0?-c->roll_direction*RR_PI*2*k*k*(3-2*k):0;
 double bank=c->roll+spin+sin(c->t*26)*c->stun*.5;
 if(!isfinite(c->pos.x)||!isfinite(c->pos.y)||!isfinite(c->pos.z)||!isfinite(c->pitch)||!isfinite(c->yaw)||!isfinite(bank))return false;
 double x=sin(c->pitch),a=cos(c->pitch),y=sin(c->yaw),b=cos(c->yaw),z=sin(bank),d=cos(bank);
 double m[16]={b*d+y*x*z,a*z,-y*d+b*x*z,0,-b*z+y*x*d,a*d,y*z+b*x*d,0,y*a,-x,b*a,0,c->pos.x,c->pos.y,c->pos.z,1};
 for(int i=0;i<16;i++)if(!isfinite(m[i]))return false;memcpy(p->root,m,sizeof(m));p->ready=true;return true;
}
bool rr_car_pose_init(RRCarPose *p,const RRCar *c){if(!p)return false;*p=(RRCarPose){0};return rr_car_pose_sync(p,c);}
bool rr_car_pose_after_step(RRCarPose *p,const RRCar *c,double before){
 if(!p||!c||!isfinite(before)||!isfinite(c->t)||!isfinite(c->nitro)||!isfinite(c->kick))return false;
 double bob=sin(c->t*2.3)*.12*(1-fmin(1,fabs(before)/RR_CURRENT_CAR_MAX)*.7),lift=c->nitro*.03+c->kick*c->kick*.07;
 if(!isfinite(bob)||!isfinite(lift)||!rr_car_pose_sync(p,c))return false;p->bob=bob;p->lift=lift;return true;
}
bool rr_car_pose_idle(RRCarPose *p,double time){if(!p||!p->ready||!isfinite(time))return false;p->bob=sin(time*2.3)*.12;p->lift=0;return true;}
bool rr_car_pose_matrix(const RRCarPose *p,double m[16]){
 if(!p||!p->ready||!m||!isfinite(p->bob)||!isfinite(p->lift))return false;
 double c=cos(p->lift),s=sin(p->lift);
 for(int i=0;i<4;i++){m[i]=p->root[i];m[4+i]=p->root[4+i]*c+p->root[8+i]*s;m[8+i]=p->root[8+i]*c-p->root[4+i]*s;m[12+i]=p->root[12+i]+p->root[4+i]*p->bob;}
 for(int i=0;i<16;i++)if(!isfinite(m[i]))return false;return true;
}
bool rr_car_pose_binding(const RRCarPose *p,int model,unsigned accent,RRFireBinding *binding){if(!binding||model<0||model>3)return false;*binding=(RRFireBinding){0};binding->model=model;binding->accent=accent;return rr_car_pose_matrix(p,binding->model_world);}

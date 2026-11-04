#include "rr_session_pose.h"
bool rr_session_pose_place(RRCarPose *p,const RRCar *c){
 if(!p||!c)return false;return p->ready?rr_car_pose_sync(p,c):rr_car_pose_init(p,c);
}
bool rr_session_pose_step(RRSession *s,RRCar *c,RRCarPose *p,const RRGate *g,int count,RRInput input,double raw,unsigned *events){
 if(!s||!c||!p||!events||!p->ready)return false;
 *events=0;double before=c->speed,dt=rr_frame_dt(raw);RRSessionPhase entered=s->phase;
 *events=rr_session_step_current(s,c,g,count,input,raw);
 if(s->phase==RR_SESSION_ERROR)return false;
 if(s->motion_stepped)return rr_car_pose_after_step(p,c,before);
 if(dt>0&&(entered==RR_SESSION_MENU||entered==RR_SESSION_PAUSED)){
  if(!isfinite(c->t)||!isfinite(c->t+dt))return false;
  c->t+=dt;return rr_car_pose_idle(p,c->t);
 }
 return true;
}

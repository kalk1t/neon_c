#include "rr_pause_ui.h"
#include <stdio.h>
typedef struct PauseEntryCase{int sequence,frame,entered,phase;double now,dt;int event;double x,y,wheel,cx,cy,cz,car_yaw,yaw,pitch,dist;int dragging;double px,py,pz,tx,ty,tz,fov;}PauseEntryCase;
#include "pause_entry_reference.h"
static int failures,checks,entries;static double worst,worst_float;
#define CHECK(x) do{checks++;if(!(x)){if(failures<8)fprintf(stderr,"pause entry line%d\n",__LINE__);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(e<3e-10);}
static void float_near(double a,double b){double e=fabs(a-b);if(e>worst_float)worst_float=e;CHECK(e<.001);}
static void event(RRShowcase *s,const PauseEntryCase *c){if(c->event==1)rr_showcase_down(s,c->x,c->y,false);else if(c->event==2)rr_showcase_move(s,c->x,c->y);else if(c->event==3)rr_showcase_up(s);else if(c->event==4)rr_showcase_wheel(s,c->wheel);}
int main(void){RRSession session;RRCar car;rr_session_init(&session);RRShowcase s={0},sf={0};RRShowcasePose pose={0},pf={0};double stamp=-1;int sequence=-1;
 RRSessionPhase phases[]={RR_SESSION_COUNTDOWN,RR_SESSION_ACTIVE,RR_SESSION_FINISHED,RR_SESSION_RESULTS};
 for(size_t i=0;i<sizeof(pause_entry_cases)/sizeof(pause_entry_cases[0]);i++){
  const PauseEntryCase *c=&pause_entry_cases[i];
  if(sequence!=c->sequence){sequence=c->sequence;rr_session_start(&session,&car,RR_RACE_TIME_ATTACK);rr_showcase_init(&s);s.yaw=7;s.pitch=-.1;s.dist=22;sf=s;pose.position=rr_v3(20+sequence,120-sequence,-30);pose.fov=68+sequence;pf=pose;}
  if(c->entered){session.phase=phases[c->phase];CHECK(rr_ui_pause_showcase(&session,&stamp,c->now,&s));CHECK(stamp==c->now&&session.phase==RR_SESSION_PAUSED&&session.resume_phase==phases[c->phase]);sf=s;entries++;}
  event(&s,c);event(&sf,c);pose=rr_showcase_pause_step(&s,rr_v3(c->cx,c->cy,c->cz),c->car_yaw,pose.position,pose.fov,c->dt);
  pf=rr_showcase_pause_step(&sf,rr_v3(c->cx,c->cy,c->cz),c->car_yaw,pf.position,pf.fov,c->dt);pf.position=rr_v3((float)pf.position.x,(float)pf.position.y,(float)pf.position.z);pf.fov=(float)pf.fov;
  near(s.yaw,c->yaw);near(s.pitch,c->pitch);near(s.dist,c->dist);CHECK(s.dragging==(c->dragging!=0));near(pose.position.x,c->px);near(pose.position.y,c->py);near(pose.position.z,c->pz);near(pose.target.x,c->tx);near(pose.target.y,c->ty);near(pose.target.z,c->tz);near(pose.fov,c->fov);
  float_near(pf.position.x,c->px);float_near(pf.position.y,c->py);float_near(pf.position.z,c->pz);float_near(pf.fov,c->fov);
  RRShowcase prior=s;double old_stamp=stamp;CHECK(!rr_ui_pause_showcase(&session,&stamp,c->now+1,&s));CHECK(stamp==old_stamp&&s.yaw==prior.yaw&&s.pitch==prior.pitch&&s.dist==prior.dist&&s.dragging==prior.dragging);
 }
 CHECK(entries==12);
 for(int p=0;p<4;p++){
  rr_session_start(&session,&car,RR_RACE_TWO_LAPS);session.phase=phases[p];rr_showcase_init(&s);s.yaw=7;s.pitch=1;s.dist=22;rr_showcase_down(&s,10,20,false);
  CHECK(rr_ui_pause_showcase(&session,&stamp,1000,&s));CHECK(s.yaw==.6&&s.pitch==.14&&s.dist==9.5&&!s.dragging);
  rr_showcase_move(&s,1000,1000);CHECK(s.yaw==.6&&s.pitch==.14);CHECK(!rr_ui_resume_key(&session,stamp,1400,true,true));CHECK(!rr_ui_resume_key(&session,stamp,1401,true,false));CHECK(rr_ui_resume_key(&session,stamp,1400.001,true,true)&&session.phase==phases[p]);
 }
 session.phase=RR_SESSION_MENU;s.yaw=7;CHECK(!rr_ui_pause_showcase(&session,&stamp,2000,&s)&&s.yaw==7);session.phase=RR_SESSION_ERROR;CHECK(!rr_ui_pause_showcase(&session,&stamp,2000,&s)&&s.yaw==7);
 rr_session_dispose(&session);printf("Pause entry:720 actual source camera/callback states,12 transitions across4 phases, duplicate rejection, stale-drag cancel and strict400ms resume retained; checks=%d source_error=%.12g float_error=%.12g failures=%d; no graphics/hardware/scene-loading qualification\n",checks,worst,worst_float,failures);return failures?1:0;
}


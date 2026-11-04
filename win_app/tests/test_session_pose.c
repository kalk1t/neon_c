#include "rr_session_pose.h"
#include <string.h>
#include "rr_weapon_current.h"
#include <stdio.h>
#include "car_pose_reference.h"
static unsigned failures;static size_t current;static double max_error;
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"pose%zu line%d: %s\n",current,__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>max_error)max_error=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-8);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
int main(void){RRCar car={0};RRCarPose pose={0};unsigned actions[6]={0};RRSession session={0};unsigned events=0;
 for(current=0;current<sizeof(pose_frames)/sizeof(*pose_frames);current++){
  const PoseFrame *f=pose_frames+current;if(f->first){rr_car_reset(&car,rr_v3(3,200,5),f->yaw,f->speed);CHECK(rr_car_pose_init(&pose,&car));}
  near(car.speed,f->before);actions[f->action]++;
  if(f->action==0){session.phase=RR_SESSION_ACTIVE;session.race.mode=RR_RACE_FREE_ROAM;CHECK(rr_session_pose_step(&session,&car,&pose,NULL,0,f->input,f->dt,&events));CHECK(events==0&&session.motion_stepped);}
  else if(f->action==1){rr_car_place(&car,rr_v3(3+f->index*.3,200+f->index*.2,5-f->index*.5),f->yaw+.1*f->index,35);CHECK(rr_session_pose_place(&pose,&car));}
  else if(f->action==2){car.pos.x+=4;car.vel.z-=3;car.stun=.7;car.speed*=.8;}
  else if(f->action==3)rr_car_dodge_current(&car,f->index%2?-1:1);
  else if(f->action==4){session.phase=current%2?RR_SESSION_MENU:RR_SESSION_PAUSED;CHECK(rr_session_pose_step(&session,&car,&pose,NULL,0,(RRInput){1,0,1,1,true},f->dt,&events));CHECK(!session.motion_stepped&&events==0);}
  else{car.yaw+=.3;car.pitch=-.2;car.roll=.15;car.stun=.4;CHECK(rr_session_pose_place(&pose,&car));}
  const State *s=&f->state;vector(car.pos,s->pos);vector(car.vel,s->vel);vector(car.fwd,s->fwd);double a[]={car.yaw,car.pitch,car.roll,car.speed,car.boost,car.t,car.stun,car.roll_time,car.surge,car.max_multiplier,car.thrust,car.nitro,car.kick,car.sputter},b[]={s->yaw,s->pitch,s->roll,s->speed,s->boost,s->t,s->stun,s->roll_time,s->surge,s->mul,s->thrust,s->nitro,s->kick,s->sputter};for(int i=0;i<14;i++)near(a[i],b[i]);CHECK(car.roll_direction==s->roll_direction&&car.boosting==s->boosting);
  double matrix[16];CHECK(rr_car_pose_matrix(&pose,matrix));for(int i=0;i<16;i++){near(pose.root[i],f->root[i]);near(matrix[i],f->matrix[i]);}near(pose.bob,f->bob);near(pose.lift,f->lift);vector(rr_weapon_current_muzzle(f->model,f->side,matrix),f->muzzle);RRFireBinding binding;CHECK(rr_car_pose_binding(&pose,f->model,0xff26d4,&binding));for(int i=0;i<16;i++)near(binding.model_world[i],f->matrix[i]);
 }
 RRCarPose empty={0};double matrix[16];CHECK(!rr_car_pose_matrix(&empty,matrix)&&!rr_car_pose_idle(&empty,0));CHECK(!rr_car_pose_binding(&pose,4,0,NULL));car.pos.x=NAN;CHECK(!rr_session_pose_place(&pose,&car));CHECK(!rr_car_pose_after_step(&pose,&car,NAN));
 /* Session lifecycle boundaries are independent assertions, not a second source capture. */
 rr_session_dispose(&session);rr_session_start(&session,&car,RR_RACE_TWO_LAPS);CHECK(rr_session_pose_place(&pose,&car));
 double old_bob=pose.bob,old_lift=pose.lift;CHECK(rr_session_pause(&session));double old_t=car.t;
 RRCar frozen=car;double root[16];memcpy(root,pose.root,sizeof(root));
 CHECK(rr_session_pose_step(&session,&car,&pose,NULL,0,(RRInput){1,0,1,1,true},2,&events));near(car.t,old_t+.05);near(car.speed,frozen.speed);vector(car.pos,frozen.pos);CHECK(!memcmp(root,pose.root,sizeof(root)));near(pose.lift,0);near(session.countdown,3.999);
 CHECK(rr_session_resume(&session));unsigned go=0;for(int i=0;i<80;i++){CHECK(rr_session_pose_step(&session,&car,&pose,NULL,0,(RRInput){1,0,1,1,true},.05,&events));go|=events;}CHECK(go==RR_SESSION_GO&&session.phase==RR_SESSION_ACTIVE);near(session.race_time,0);near(car.speed,0);
 session.phase=RR_SESSION_FINISHED;session.race.finished=true;session.race.finish_time=12.3;double clock=session.race_time;
 CHECK(rr_session_pose_step(&session,&car,&pose,NULL,0,(RRInput){1,0,.2,0,true},1.99,&events));CHECK(session.motion_stepped&&events==0);near(session.race_time,clock+.05);
 CHECK(rr_session_pose_step(&session,&car,&pose,NULL,0,(RRInput){1,0,.2,0,true},.02,&events));CHECK(events==RR_SESSION_SHOW_RESULTS&&session.phase==RR_SESSION_RESULTS);near(session.race.finish_time,12.3);
 CHECK(rr_session_pose_step(&session,&car,&pose,NULL,0,(RRInput){0},.02,&events));CHECK(session.motion_stepped&&events==0);
 old_bob=pose.bob;old_lift=pose.lift;rr_session_respawn(&session,&car);CHECK(rr_session_pose_place(&pose,&car));near(pose.bob,old_bob);near(pose.lift,old_lift);
 rr_session_start(&session,&car,RR_RACE_TIME_ATTACK);CHECK(rr_session_pose_place(&pose,&car));near(pose.bob,old_bob);near(pose.lift,old_lift);near(session.finish_wait,0);
 rr_session_menu(&session);RRCar saved=car;RRCarPose saved_pose=pose;
 double invalid[]={0,-1,NAN,INFINITY};for(int i=0;i<4;i++){CHECK(rr_session_pose_step(&session,&car,&pose,NULL,0,(RRInput){1,0,1,1,true},invalid[i],&events));CHECK(events==0&&!session.motion_stepped&&!memcmp(&saved,&car,sizeof(car))&&!memcmp(&saved_pose,&pose,sizeof(pose)));}
 CHECK(!rr_session_pose_step(&session,&car,&empty,NULL,0,(RRInput){0},.02,&events));
 session.phase=RR_SESSION_ERROR;CHECK(!rr_session_pose_step(&session,&car,&pose,NULL,0,(RRInput){0},.02,&events));
 rr_session_dispose(&session);
 printf("SESSION_POSE_SUMMARY frames=%zu actions=%u,%u,%u,%u,%u,%u pose_bytes=%zu max_error=%.17g failures=%u; no graphics/world\n",sizeof(pose_frames)/sizeof(*pose_frames),actions[0],actions[1],actions[2],actions[3],actions[4],actions[5],sizeof(pose),max_error,failures);return failures?1:0;
}

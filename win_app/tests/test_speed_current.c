#include "rr_player_controls.h"
#include "rr_chase_camera.h"
#include "rr_physics.h"
#include <stdio.h>
#include <string.h>
#include "speed_current_reference.h"
static unsigned failures;static size_t current;static double max_error;
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"row%zu line%d: %s\n",current,__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>max_error)max_error=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-8);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
static void state(const RRCar *c,RRCarStepReport report,double out[33]){
 double values[]={c->pos.x,c->pos.y,c->pos.z,c->vel.x,c->vel.y,c->vel.z,c->fwd.x,c->fwd.y,c->fwd.z,c->yaw,c->pitch,c->roll,c->speed,c->boost,c->side,c->t,c->stun,c->roll_time,c->surge,c->thrust,c->nitro,c->kick,c->sputter,c->pod_pitch[0],c->pod_roll[0],c->pod_pitch[1],c->pod_roll[1],c->pod_pitch[2],c->pod_roll[2],c->pod_pitch[3],c->pod_roll[3],rr_car_display_speed(c),report.impact};
 memcpy(out,values,sizeof(values));
}
static void samecar(const RRCar *a,const RRCar *b){double x[33],y[33];state(a,(RRCarStepReport){0},x);state(b,(RRCarStepReport){0},y);for(int k=0;k<33;k++)near(x[k],y[k]);CHECK(a->boosting==b->boosting);}
static double random_value(void *p){(void)p;return .75;}
int main(void){
 RRCar car={0};static RRWorld world;unsigned cameras=0,collisions=0,dodges=0;double t90=0;
 for(current=0;current<sizeof(current_speed_frames)/sizeof(*current_speed_frames);current++){
  const Fixture *f=current_speed_frames+current;
  if(f->start){rr_car_reset(&car,rr_v3(0,400,0),f->yaw,f->speed);car.max_multiplier=f->mul;world.count=0;
   if(f->scenario==6)CHECK(rr_world_add(&world,rr_box(rr_v3(0,400,-20),rr_v3(20,20,2),0)));
   if(f->scenario==7)CHECK(rr_world_add(&world,rr_cylinder(0,-20,6,380,420)));
   if(f->scenario==8)CHECK(rr_world_add(&world,rr_torus(rr_v3(0,400,-20),0,15.5,2.45)));
  }
  if(f->reset)rr_car_place(&car,rr_v3(0,400,0),f->yaw,0);if(f->stun)car.stun=.7;if(f->dodge){rr_car_dodge_current(&car,f->side);dodges++;}
  RRCarStepReport report;rr_car_step_current_world_report(&car,f->input,f->dt,&world,&report);double actual[33];state(&car,report,actual);for(int k=0;k<33;k++)near(actual[k],f->values[k]);CHECK(car.boosting==f->boosting&&report.collided==f->collision);vector(report.normal,f->normal);if(report.collided)collisions++;
  if(f->scenario==0&&t90==0&&car.speed>=135*.9)t90=(double)(current+1)*f->dt;
  if(f->camera){RRFirstPerson eye=rr_chase_pose_current_aim(&car,f->mode,true,.2,-.1,.1,.2,f->aim);double values[]={eye.position.x,eye.position.y,eye.position.z,eye.forward.x,eye.forward.y,eye.forward.z,eye.up.x,eye.up.y,eye.up.z};for(int k=0;k<9;k++)near(values[k],f->eye[k]);cameras++;}
 }
 near(t90,3.8833333333333333);near(RR_CURRENT_CAR_BOOST_MAX,196.10526315789474);
 // Actual native host boundary: current session uses one capped flight step,
 // countdown uses idle, pause/zero dt don't move, finish/results still fly.
 RRSession session;rr_session_init(&session);RRCar direct;rr_session_start(&session,&car,RR_RACE_FREE_ROAM);direct=car;
 RRInput input={.throttle=1,.boost=true,.strafe=1,.steer=.2};
 for(int i=0;i<120;i++){double raw=i==0?.2:1./60;rr_car_step_current(&direct,input,rr_frame_dt(raw));CHECK(rr_session_step_current(&session,&car,NULL,0,input,raw)==0&&session.motion_stepped);samecar(&car,&direct);}
 CHECK(rr_session_pause(&session));direct=car;rr_session_step_current(&session,&car,NULL,0,input,.05);CHECK(!session.motion_stepped);samecar(&car,&direct);CHECK(rr_session_resume(&session));rr_session_step_current(&session,&car,NULL,0,input,NAN);CHECK(!session.motion_stepped);samecar(&car,&direct);
 for(int i=0;i<3;i++){session.phase=i==0?RR_SESSION_COUNTDOWN:i==1?RR_SESSION_FINISHED:RR_SESSION_RESULTS;session.countdown=3;direct=car;rr_car_step_current(&direct,i==0?(RRInput){0}:input,.02);rr_session_step_current(&session,&car,NULL,0,input,.02);CHECK(session.motion_stepped);samecar(&car,&direct);}
 // The production native spin adapter selects current minimum surge speed.
 rr_car_reset(&car,rr_v3(0,400,0),0,0);session.phase=RR_SESSION_ACTIVE;double ay=0,ap=0,cd=0,kick=0;RRRecovery recovery={0};RRPlayerControlState controls={&session,&car,&recovery,NULL,&ay,&ap,&cd,&kick};
 RRPlayerControlEvents events=rr_player_controls_step_current(&controls,(RRPlayerControlInput){.dt=.01,.steer=1,.spin=true,.alive=true},random_value,NULL);CHECK(events.spin.accepted&&events.spin.side==1);near(car.speed,101.25);near(kick,.6);near(cd,6);
 rr_session_dispose(&session);
 printf("SPEED_CURRENT_SUMMARY frames=%zu scenarios=10 cameras=%u collisions=%u dodges=%u first90=%.17g max_error=%.17g failures=%u; current native session/spin/chase connected,no graphics\n",sizeof(current_speed_frames)/sizeof(*current_speed_frames),cameras,collisions,dodges,t90,max_error,failures);return failures?1:0;
}

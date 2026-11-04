#include "rr_player_controls.h"
#include "rr_chase_camera.h"
#include "rr_physics.h"
#include <stdio.h>
#include <string.h>
#include "car_stats_reference.h"
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
 RRCar car={0};static RRWorld world;unsigned collisions=0,dodges=0,resets=0;
 for(current=0;current<sizeof(profile_frames)/sizeof(*profile_frames);current++){
  const Fixture *f=profile_frames+current;
  if(f->start){rr_car_reset(&car,rr_v3(0,400,0),f->yaw,f->speed);car.max_multiplier=f->mul;rr_car_use_stats(&car,f->id);world.count=0;
   if(f->scenario==6)CHECK(rr_world_add(&world,rr_box(rr_v3(0,400,-20),rr_v3(20,20,2),0)));
   if(f->scenario==7)CHECK(rr_world_add(&world,rr_cylinder(0,-20,6,380,420)));
   if(f->scenario==8)CHECK(rr_world_add(&world,rr_torus(rr_v3(0,400,-20),0,15.5,2.45)));
  }
  if(f->reset){RRCarProfile selected=car.profile;rr_car_place(&car,rr_v3(0,400,0),f->yaw,0);CHECK(car.profile==selected);resets++;}if(f->stun)car.stun=.7;if(f->dodge){rr_car_dodge_current(&car,f->side);dodges++;}
  RRCarStepReport report;rr_car_step_current_world_report(&car,f->input,f->dt,&world,&report);double actual[33];state(&car,report,actual);for(int k=0;k<33;k++)near(actual[k],f->values[k]);CHECK(car.boosting==f->boosting&&report.collided==f->collision);vector(report.normal,f->normal);if(report.collided)collisions++;

 }

 const char *ids[]={"roadster","viper","titan"};
 for(unsigned i=0;i<3;i++){
 const RRCarStats *stats=rr_car_stats((RRCarProfile)i);double values[]={stats->top,stats->accel,stats->turn,stats->hp,stats->knock,stats->stun,stats->mass,stats->spin_cd,stats->missiles,stats->emp};for(int k=0;k<10;k++)near(values[k],profile_values[i][k]);
 rr_car_reset(&car,rr_v3(0,400,0),0,0);rr_car_use_stats(&car,ids[i]);rr_car_dodge_current(&car,1);near(car.speed,101.25);
 // Legacy fixtures intentionally keep old handling even for a selected profile.
 RRCar legacy=car,base=car;base.profile=RR_PROFILE_ROADSTER;rr_car_step(&legacy,(RRInput){.throttle=1,.steer=.8},.02);rr_car_step(&base,(RRInput){.throttle=1,.steer=.8},.02);samecar(&legacy,&base);
 }
 rr_car_use_stats(&car,"VIPER");CHECK(car.profile==RR_PROFILE_ROADSTER);rr_car_use_stats(&car,NULL);CHECK(car.profile==RR_PROFILE_ROADSTER);CHECK(rr_car_stats((RRCarProfile)-1)==rr_car_stats(RR_PROFILE_ROADSTER));
 rr_car_use_stats(&car,"titan");rr_car_reset(&car,rr_v3(0,400,0),0,0);CHECK(car.profile==RR_PROFILE_ROADSTER);

 for(unsigned profile=0;profile<3;profile++){
 // Actual native host boundary: current session uses one capped flight step,
 // countdown uses idle, pause/zero dt don't move, finish/results still fly.
 RRSession session;rr_session_init(&session);RRCar direct;rr_session_start(&session,&car,RR_RACE_FREE_ROAM);rr_car_use_stats(&car,ids[profile]);direct=car;
 RRInput input={.throttle=1,.boost=true,.strafe=1,.steer=.2};
 for(int i=0;i<120;i++){double raw=i==0?.2:1./60;rr_car_step_current(&direct,input,rr_frame_dt(raw));CHECK(rr_session_step_current(&session,&car,NULL,0,input,raw)==0&&session.motion_stepped);samecar(&car,&direct);}
 CHECK(rr_session_pause(&session));direct=car;rr_session_step_current(&session,&car,NULL,0,input,.05);CHECK(!session.motion_stepped);samecar(&car,&direct);CHECK(rr_session_resume(&session));rr_session_step_current(&session,&car,NULL,0,input,NAN);CHECK(!session.motion_stepped);samecar(&car,&direct);
 for(int i=0;i<3;i++){session.phase=i==0?RR_SESSION_COUNTDOWN:i==1?RR_SESSION_FINISHED:RR_SESSION_RESULTS;session.countdown=3;direct=car;rr_car_step_current(&direct,i==0?(RRInput){0}:input,.02);rr_session_step_current(&session,&car,NULL,0,input,.02);CHECK(session.motion_stepped);samecar(&car,&direct);}
 rr_session_dispose(&session);
 }
 RRSession session;rr_session_init(&session);
 // The production native spin adapter selects current minimum surge speed.
 rr_car_reset(&car,rr_v3(0,400,0),0,0);session.phase=RR_SESSION_ACTIVE;double ay=0,ap=0,cd=0,kick=0;RRRecovery recovery={0};RRPlayerControlState controls={&session,&car,&recovery,NULL,&ay,&ap,&cd,&kick};
 RRPlayerControlEvents events=rr_player_controls_step_current(&controls,(RRPlayerControlInput){.dt=.01,.steer=1,.spin=true,.alive=true},random_value,NULL);CHECK(events.spin.accepted&&events.spin.side==1);near(car.speed,101.25);near(kick,.6);near(cd,6);
 rr_session_dispose(&session);
 printf("CAR_STATS_SUMMARY frames=%zu profiles=3 scenarios=30 collisions=%u dodges=%u preserved_placements=%u max_error=%.17g car_bytes=%zu failures=%u; current session connected, production selection Roadster, no graphics/ownership/online changes\n",sizeof(profile_frames)/sizeof(*profile_frames),collisions,dodges,resets,max_error,sizeof(RRCar),failures);return failures?1:0;
}

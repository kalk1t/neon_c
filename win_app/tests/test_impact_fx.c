#include "rr_camera_fx.h"
#include "rr_session.h"
#include "rr_physics.h"
#include <stdio.h>
typedef struct ImpactCase {double shake,impact,dt;int ignite,event;double before,after;} ImpactCase;
#include "impact_fx_fixture.h"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"Impact check line%d\n",__LINE__);failures++;}}while(0)
int main(void){double worst=0;
for(size_t i=0;i<sizeof(impact_cases)/sizeof(impact_cases[0]);i++){
const ImpactCase *c=&impact_cases[i];RRCameraFX fx={0};fx.shake=c->shake;fx.last_nitro=-9;
if(c->ignite)CHECK(rr_camera_fx_ignite(&fx,false,true,2));
CHECK(rr_camera_fx_impact(&fx,c->impact)==(c->event!=0));
double e=fabs(fx.shake-c->before);if(e>worst)worst=e;CHECK(e<1e-12);
rr_camera_fx_step(&fx,0,c->dt,0);e=fabs(fx.shake-c->after);if(e>worst)worst=e;CHECK(e<1e-12);
}
/* Real native session-to-physics report seam; no extra movement step. */
static RRWorld world;CHECK(rr_world_add(&world,rr_box(rr_v3(0,118,-20),rr_v3(20,20,2),0)));
RRSession s;RRCar car;rr_session_init(&s);rr_session_start(&s,&car,RR_RACE_TIME_ATTACK);s.phase=RR_SESSION_ACTIVE;s.world=&world;
rr_car_reset(&car,rr_v3(0,118,-15),0,60);RRCar direct=car;RRCarStepReport expected={0};
rr_car_step_world_report(&direct,(RRInput){0},.05,&world,&expected);
rr_session_step(&s,&car,NULL,0,(RRInput){0},.05);
CHECK(s.motion_stepped&&s.motion.collided&&s.motion.impact>4);
CHECK(fabs(s.motion.impact-expected.impact)<1e-12&&rr_length(rr_sub(car.pos,direct.pos))<1e-12);
CHECK(rr_length(rr_sub(s.motion.normal,expected.normal))<1e-12);
rr_session_step(&s,&car,NULL,0,(RRInput){0},0);CHECK(!s.motion_stepped&&!s.motion.collided&&s.motion.impact==0);
CHECK(rr_session_pause(&s));rr_session_step(&s,&car,NULL,0,(RRInput){0},.05);CHECK(!s.motion_stepped&&s.motion.impact==0);
CHECK(rr_session_resume(&s));rr_car_reset(&car,rr_v3(0,118,40),0,0);rr_session_step(&s,&car,NULL,0,(RRInput){0},.05);CHECK(s.motion_stepped&&!s.motion.collided&&s.motion.impact==0);
rr_session_start(&s,&car,RR_RACE_TIME_ATTACK);CHECK(!s.motion_stepped&&s.motion.impact==0);
for(int phase=RR_SESSION_COUNTDOWN;phase<=RR_SESSION_RESULTS;phase++){
s.phase=(RRSessionPhase)phase;s.world=&world;rr_car_reset(&car,rr_v3(0,118,-15),0,60);
rr_session_step(&s,&car,NULL,0,(RRInput){0},.05);CHECK(s.motion_stepped&&s.motion.collided&&s.motion.impact>4);
}
rr_session_menu(&s);rr_session_step(&s,&car,NULL,0,(RRInput){0},.05);CHECK(!s.motion_stepped&&s.motion.impact==0);
rr_session_dispose(&s);
printf("Actual browser impact shake: %zu boundary/order cases, worst %.17g, %d failures; session report lifecycle checked\n",sizeof(impact_cases)/sizeof(impact_cases[0]),worst,failures);return failures?1:0;}

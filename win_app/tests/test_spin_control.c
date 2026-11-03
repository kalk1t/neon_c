#include "rr_spin_control.h"
#include "rr_sub_notice.h"
#include <string.h>
typedef struct SpinCase {int active,hit;double cooldown,dt,steer,random,kick,speed,yaw;int accepted,denied,side,draws;double expected[7];const char *caption;double timer;} SpinCase;
#include "spin_control_fixture.h"
typedef struct Random {double value;unsigned draws;} Random;
static double random_value(void *p){Random *r=p;r->draws++;return r->value;}
int main(void){int failures=0;double worst=0;
for(size_t i=0;i<sizeof(spin_cases)/sizeof(spin_cases[0]);i++){
const SpinCase *c=&spin_cases[i];double cd=c->cooldown,kick=c->kick;Random random={c->random,0};RRCar car={0};car.speed=c->speed;car.yaw=c->yaw;car.vel=rr_v3(2,0,-3);RRSubNotice notice={0};
RRSpinResult event=rr_spin_control_step(&cd,c->active!=0,c->dt,c->hit!=0,c->steer,random_value,&random);
rr_spin_control_apply(event,&car,&kick);if(event.accepted)rr_sub_notice_spin(&notice);
if(event.accepted!=(c->accepted!=0)||event.denied!=(c->denied!=0)||event.side!=c->side||random.draws!=(unsigned)c->draws||event.random_draws!=random.draws||strcmp(notice.text,c->caption)||notice.timer!=c->timer)failures++;
double got[]={cd,kick,car.speed,car.vel.x,car.vel.z,car.roll_time,car.surge};
for(int j=0;j<7;j++){double e=fabs(got[j]-c->expected[j]);if(e>worst)worst=e;if(!isfinite(got[j])||e>1e-11)failures++;}
if(event.accepted&&car.roll_direction!=event.side)failures++;
}
/* Session finished in this step: a simultaneous spin request must be ignored. */
RRSession session;RRCar car;rr_session_init(&session);rr_session_start(&session,&car,RR_RACE_TWO_LAPS);session.phase=RR_SESSION_ACTIVE;session.race.lap=2;session.race.next_checkpoint=1;session.race.safe_position=car.pos;
RRGate gate={{0,118,-100},{0,0,-1},0};rr_car_reset(&car,rr_v3(0,118,1),0,95);unsigned flags=rr_session_step(&session,&car,&gate,1,(RRInput){0},.05);
Random random={.1,0};double cd=.01;RRSpinResult e=rr_spin_control_step(&cd,session.phase==RR_SESSION_ACTIVE,.05,true,0,random_value,&random);
if(!(flags&RR_RACE_FINISH)||e.accepted||e.denied||random.draws||cd!=.01)failures++;
rr_session_dispose(&session);
printf("Actual browser spin control/dodge: %zu cases, worst %.17g, %d failures; finish-frame suppression checked\n",sizeof(spin_cases)/sizeof(spin_cases[0]),worst,failures);return failures?1:0;}

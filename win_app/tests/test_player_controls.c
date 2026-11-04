#include "rr_player_controls.h"
#include "rr_camera_motion.h"
#include <stdio.h>
typedef struct ControlCase {int active,alive,blocked,spin,reset,automatic,looking;double cooldown,dt,steer,random;int accepted,denied,did_reset,was_blocked,draws;double expected[23];} ControlCase;
#include "player_control_fixture.h"
typedef struct Random {double value;unsigned used;} Random;
static double random_value(void *p){Random *r=p;r->used++;return r->value;}
int main(void){static RRTrack track;if(!rr_track_init(&track))return 1;int failures=0;double worst=0;
for(size_t i=0;i<sizeof(control_cases)/sizeof(control_cases[0]);i++){
const ControlCase *c=&control_cases[i];RRSession session;rr_session_init(&session);session.phase=c->active?RR_SESSION_ACTIVE:RR_SESSION_FINISHED;session.race.safe_position=rr_v3(12,118,-32);session.race.safe_yaw=.7;session.race.lap_time=23;session.race.next_checkpoint=3;
RRCar car={0};car.pos=rr_v3(20,130,10);car.vel=rr_v3(15,2,-70);car.yaw=1.1;car.pitch=.2;car.roll=-.15;car.speed=80;car.boost=.4;car.nitro=.7;car.thrust=.3;car.kick=.4;car.roll_time=.2;car.roll_direction=1;car.surge=.6;car.stun=1.2;
car.fwd=rr_v3(-sin(car.yaw)*cos(car.pitch),sin(car.pitch),-cos(car.yaw)*cos(car.pitch));
RRRecovery recovery={0};recovery.wrong_way=.6;recovery.lost_time=.8;double yaw=-.4,pitch=.3,cooldown=c->cooldown,kick=.2;Random random={c->random,0};
RRPlayerControlState state={&session,&car,&recovery,&track,&yaw,&pitch,&cooldown,&kick};RRCameraMotion motion={2,-.3,.2,-.1};
if(c->automatic)rr_player_controls_respawn(&state);
RRPlayerControlEvents events=rr_player_controls_step(&state,(RRPlayerControlInput){c->dt,c->steer,c->spin!=0,c->reset!=0,c->alive!=0,c->blocked!=0},random_value,&random);
rr_camera_motion_step(&motion,car.yaw,car.pitch,c->looking!=0,c->dt);
double got[]={car.pos.x,car.pos.y,car.pos.z,car.yaw,car.pitch,car.speed,car.vel.x,car.vel.y,car.vel.z,car.roll_time,car.surge,car.stun,cooldown,kick,yaw,pitch,recovery.lost_time,recovery.wrong_way,motion.yaw,motion.pitch,motion.look_yaw,motion.look_pitch,car.roll};
if(events.spin.accepted!=(c->accepted!=0)||events.spin.denied!=(c->denied!=0)||events.reset!=(c->did_reset!=0)||events.reset_blocked!=(c->was_blocked!=0)||random.used!=(unsigned)c->draws)failures++;
for(int j=0;j<23;j++){double e=fabs(got[j]-c->expected[j]);if(e>worst)worst=e;if(!isfinite(got[j])||e>1e-11){if(failures<6)fprintf(stderr,"Controls row%zu field%d error%.17g\n",i,j,e);failures++;}}
if(car.boost!=.4||car.nitro!=.7||car.thrust!=.3||car.kick!=.4||session.race.lap_time!=23||session.race.next_checkpoint!=3)failures++;
if((c->automatic||events.reset)&&recovery.sample!=rr_track_nearest(&track,car.pos))failures++;
rr_session_dispose(&session);
}
printf("Actual browser reset/spin/camera prefix: %zu scenarios, worst%.17g, %d failures; resources/progress retained, no camera snap\n",sizeof(control_cases)/sizeof(control_cases[0]),worst,failures);return failures?1:0;}

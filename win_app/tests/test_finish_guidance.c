#include "rr_finish.h"
#include "rr_session.h"
#include "rr_recovery.h"
#include <stdio.h>
typedef struct PilotCase {int road,sample;RRVec3 pos;double yaw,pitch;RRVec3 fwd;RRInput input;RRFinishAim aim;} PilotCase;
typedef struct TargetCase {int road,next;RRVec3 expected;} TargetCase;
#include "finish_guidance_fixture.h"
static RRTrack track;static int failures;static double worst;
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;if(!isfinite(a)||e>1e-8)failures++;}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
static unsigned crossing(RRSession *session,RRCar *car){
 if(session->race.next_checkpoint<track.gate_count){const RRGate *g=&track.gates[session->race.next_checkpoint];rr_car_reset(car,rr_sub(g->pos,rr_scale(g->forward,.1)),g->yaw,35);car->pitch=atan2(g->forward.y,hypot(g->forward.x,g->forward.z));}
 else rr_car_reset(car,rr_v3(0,118,.1),0,35);
 return rr_session_step(session,car,track.gates,track.gate_count,(RRInput){0},1.0/30);
}
int main(void){int road=0;
for(size_t i=0;i<sizeof(pilot_cases)/sizeof(pilot_cases[0]);i++){
 const PilotCase *c=&pilot_cases[i];if(c->road!=road){road=c->road;if(!rr_track_init_road(&track,road))return 1;}
 RRCar car={0};car.pos=c->pos;car.yaw=c->yaw;car.pitch=c->pitch;car.fwd=c->fwd;car.boost=.45;RRFinishAim aim={9,8,{7,6,5}};
 RRInput input=rr_finish_player_controls(&track,&car,c->sample,&aim);
 near(input.throttle,c->input.throttle);near(input.brake,c->input.brake);near(input.steer,c->input.steer);near(input.climb,c->input.climb);if(input.boost!=c->input.boost)failures++;
 near(aim.yaw,c->aim.yaw);near(aim.pitch,c->aim.pitch);vector(aim.direction,c->aim.direction);vector(car.pos,c->pos);vector(car.fwd,c->fwd);near(car.boost,.45);near(car.yaw,c->yaw);near(car.pitch,c->pitch);
}
const RRSessionPhase phases[]={RR_SESSION_ACTIVE,RR_SESSION_COUNTDOWN,RR_SESSION_FINISHED,RR_SESSION_RESULTS};road=0;
for(size_t i=0;i<sizeof(target_cases)/sizeof(target_cases[0]);i++){
 const TargetCase *c=&target_cases[i];if(c->road!=road){road=c->road;if(!rr_track_init_road(&track,road))return 1;}
 RRSession session;rr_session_init(&session);session.race.next_checkpoint=c->next;
 for(size_t p=0;p<sizeof(phases)/sizeof(phases[0]);p++){session.phase=phases[p];session.race.finished=p>1;RRVec3 target={0};if(!rr_session_target(&session,track.gates,track.gate_count,&target))failures++;else vector(target,c->expected);}
 session.phase=RR_SESSION_MENU;RRVec3 target={0};if(rr_session_target(&session,track.gates,track.gate_count,&target))failures++;
 session.phase=RR_SESSION_ACTIVE;session.race.mode=RR_RACE_FREE_ROAM;if(rr_session_target(&session,track.gates,track.gate_count,&target))failures++;
 rr_session_dispose(&session);
}
/* Actual two-lap crossing, continued pilot before physics, results, restart. */
for(road=1;road<=RR_ROAD_COUNT;road++){
 if(!rr_track_init_road(&track,road))return 1;RRSession s;RRCar car={0};rr_session_init(&s);rr_session_start(&s,&car,RR_RACE_TWO_LAPS);s.phase=RR_SESSION_ACTIVE;
 for(int lap=0;lap<2;lap++){for(int gate=0;gate<track.gate_count;gate++)if(crossing(&s,&car)!=RR_RACE_CHECKPOINT)failures++;unsigned e=crossing(&s,&car);if(e!=(unsigned)(RR_RACE_LAP|(lap?RR_RACE_FINISH:0)))failures++;}
 RRVec3 target={0};if(!rr_session_target(&s,track.gates,track.gate_count,&target))failures++;else vector(target,track.gates[0].pos);
 double finish=s.race.finish_time;RRFinishAim aim={0};RRRecovery recovery;rr_recovery_init(&recovery,&track,car.pos);
 for(int frame=0;frame<80;frame++){RRVec3 before=car.fwd;double yaw=car.yaw,pitch=car.pitch;RRInput input=rr_finish_player_controls(&track,&car,recovery.sample,&aim);vector(aim.direction,before);near(aim.yaw,yaw);near(aim.pitch,pitch);rr_session_step(&s,&car,track.gates,track.gate_count,input,1.0/30);rr_recovery_update(&recovery,&track,car.pos,car.vel,34,1.0/30,3,false);if(s.race.finish_time!=finish||!rr_session_target(&s,track.gates,track.gate_count,&target))failures++;}
 if(s.phase!=RR_SESSION_RESULTS||s.lap_count!=2)failures++;
 RRVec3 retained=aim.direction;rr_session_menu(&s);if(rr_session_target(&s,track.gates,track.gate_count,&target))failures++;rr_session_start(&s,&car,RR_RACE_TIME_ATTACK);vector(aim.direction,retained);if(s.phase!=RR_SESSION_COUNTDOWN||!rr_session_target(&s,track.gates,track.gate_count,&target))failures++;
 rr_session_dispose(&s);
}
printf("Browser finish guidance: %zu aim/control cases, %zu target cases across four phases; nine two-lap completion/pilot/results/restart sequences, worst%.17g, %d failures\n",sizeof(pilot_cases)/sizeof(pilot_cases[0]),sizeof(target_cases)/sizeof(target_cases[0]),worst,failures);return failures?1:0;}

#include "rr_recovery_feedback.h"
#include "rr_player_controls.h"
#include <stdio.h>
#include <string.h>
typedef struct FeedbackCase {int scenario,phase;double dt,along,off,height;int roam;double initial_wrong,initial_lost;const char *initial_text;double initial_timer;int initial_warn;unsigned events;double wrong,lost;const char *text;double timer;int shown,warn,respawns;} FeedbackCase;
#include "recovery_feedback_fixture.h"
int main(void){int failures=0,scenario=-1,respawns=0;double worst=0;RRRecovery recovery={0};RRHUDMessage message={0};
for(size_t i=0;i<sizeof(feedback_cases)/sizeof(feedback_cases[0]);i++){
 const FeedbackCase *c=&feedback_cases[i];if(c->scenario!=scenario){scenario=c->scenario;recovery=(RRRecovery){0};recovery.wrong_way=c->initial_wrong;recovery.lost_time=c->initial_lost;rr_hud_message_set(&message,c->initial_text,c->initial_timer,c->initial_warn!=0);message.shown=c->initial_timer>0;respawns=0;}
 unsigned events=rr_recovery_feedback_conditions((RRSessionPhase)c->phase,&recovery,&message,c->along,c->off,c->height,34,c->dt,c->roam!=0);if(events&RR_RECOVERY_RESPAWN)respawns++;
 rr_hud_message_tick(&message,c->dt);double got[]={recovery.wrong_way,recovery.lost_time,message.timer},expected[]={c->wrong,c->lost,c->timer};
 for(int j=0;j<3;j++){double e=fabs(got[j]-expected[j]);if(e>worst)worst=e;if(!isfinite(got[j])||e>1e-11)failures++;}
 if(events!=c->events||strcmp(message.text,c->text)||message.shown!=(c->shown!=0)||message.warn!=(c->warn!=0)||respawns!=c->respawns){if(failures<5)fprintf(stderr,"Feedback row%zu phase%d flags%u/%u text%s/%s\n",i,c->phase,events,c->events,message.text,c->text);failures++;}
}
/* Real nearest-track/update/respawn seam on every road: recovery in the finish
 * frame and results retains frozen laps/finish time, view and boost resources. */
static RRTrack track;const RRSessionPhase phases[]={RR_SESSION_ACTIVE,RR_SESSION_FINISHED,RR_SESSION_RESULTS};for(int road=1;road<=RR_ROAD_COUNT;road++){
 if(!rr_track_init_road(&track,road)){failures++;continue;}
 for(size_t phase=0;phase<sizeof(phases)/sizeof(phases[0]);phase++){
 RRSession session;rr_session_init(&session);session.phase=phases[phase];session.race.finished=phase!=0;session.race.finish_time=72;session.race.lap_time=0;session.race.safe_position=track.samples[0];session.race.safe_yaw=.7;
 RRCar car={0};car.pos=rr_v3(0,0,0);car.boost=.45;car.nitro=.6;car.thrust=.2;rr_recovery_init(&recovery,&track,car.pos);recovery.lost_time=1.09;rr_hud_message_finish(&message);
 unsigned events=rr_recovery_feedback_update(session.phase,&recovery,&track,&car,&message,34,.05,false);
 double aim_yaw=1,aim_pitch=.4,spin_cd=.3,kick=.2;RRPlayerControlState control={&session,&car,&recovery,&track,&aim_yaw,&aim_pitch,&spin_cd,&kick};
 if(events&RR_RECOVERY_RESPAWN)rr_player_controls_respawn(&control);else failures++;
 if(session.race.finish_time!=72||session.race.finished!=(phase!=0)||session.phase!=phases[phase]||car.boost!=.45||car.nitro!=.6||car.thrust!=.2||spin_cd!=.3||kick!=.2||aim_yaw!=.7||aim_pitch!=0||recovery.sample!=rr_track_nearest(&track,car.pos)||car.speed!=35)failures++;
 RRRecovery saved=recovery;RRHUDMessage saved_message=message;
 if(rr_recovery_feedback_update(RR_SESSION_COUNTDOWN,&recovery,&track,&car,&message,34,.05,false)||memcmp(&saved,&recovery,sizeof(saved))||memcmp(&saved_message,&message,sizeof(message)))failures++;
 rr_session_dispose(&session);
 }
}
printf("Browser recovery feedback: %zu frames across %d sequences, all nine real update/respawn seams; worst error%.17g, %d failures\n",sizeof(feedback_cases)/sizeof(feedback_cases[0]),scenario+1,worst,failures);return failures?1:0;}

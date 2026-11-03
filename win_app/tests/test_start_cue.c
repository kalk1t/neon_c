#include "rr_start_cue.h"
#include "rr_session.h"
#include <math.h>
typedef struct CueCase {int reset,paused,phase,count,shown;double raw,timer;const char *text;} CueCase;
#include "start_cue_fixture.h"
int main(void){int failures=0;double worst=0;RRStartCue cue={0};RRSession session;RRCar car;rr_session_init(&session);
    for(size_t i=0;i<sizeof(cue_cases)/sizeof(cue_cases[0]);i++){
        const CueCase *c=&cue_cases[i];if(c->reset){if(c->reset==1)rr_start_cue_reset(&cue);else rr_start_cue_restart(&cue);rr_session_start(&session,&car,RR_RACE_TIME_ATTACK);}
        if(c->paused&&session.phase!=RR_SESSION_PAUSED)rr_session_pause(&session);
        if(!c->paused&&session.phase==RR_SESSION_PAUSED)rr_session_resume(&session);
        bool counting=session.phase==RR_SESSION_COUNTDOWN;
        rr_session_step(&session,&car,NULL,0,(RRInput){0},c->raw);
        if(counting)rr_start_cue_observe(&cue,session.last_count);
        rr_start_cue_tick(&cue,rr_frame_dt(c->raw));double e=fabs(cue.timer-c->timer);if(e>worst)worst=e;
        if((int)session.phase!=c->phase||cue.count!=c->count||cue.shown!=(c->shown!=0)||strcmp(cue.text,c->text)!=0||e>1e-12){if(failures<8)fprintf(stderr,"start cue row %zu\n",i);failures++;}
    }
    rr_session_dispose(&session);printf("Actual browser countdown/HUD timers: %zu session-coupled frames, max timer error %.17g, %d failures\n",sizeof(cue_cases)/sizeof(cue_cases[0]),worst,failures);return failures?1:0;
}

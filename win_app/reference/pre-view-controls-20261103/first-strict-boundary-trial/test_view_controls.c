#include "rr_view_controls.h"
#include "rr_cockpit_plan.h"
#include <stdio.h>
typedef struct ViewCase {int phase,mode,interior,c,i,enabled,expected_mode,expected_interior,writes;} ViewCase;
typedef struct VisibleCase {int phase,mode,interior;double look;int first_person,cockpit;} VisibleCase;
#include "view_control_fixture.h"
int main(void){int failures=0;
for(size_t n=0;n<sizeof(view_cases)/sizeof(view_cases[0]);n++){
 const ViewCase *c=&view_cases[n];RRViewControls v={c->mode,c->interior!=0};
 unsigned count=rr_view_controls_step(&v,(RRSessionPhase)c->phase,true,c->enabled!=0,c->c!=0,c->i!=0);
 if(v.mode!=c->expected_mode||v.interior!=(c->expected_interior!=0)||count!=(unsigned)c->writes){if(failures<5)fprintf(stderr,"Dispatch row%zu\n",n);failures++;}
 RRViewControls original={c->mode,c->interior!=0};v=original;
 if(rr_view_controls_step(&v,(RRSessionPhase)c->phase,false,c->enabled!=0,c->c!=0,c->i!=0)!=0||v.mode!=original.mode||v.interior!=original.interior)failures++;
}
for(size_t n=0;n<sizeof(visible_cases)/sizeof(visible_cases[0]);n++){
 const VisibleCase *c=&visible_cases[n];RRSessionPhase phase=(RRSessionPhase)c->phase;
 bool first=rr_view_flight_phase(phase)&&!c->interior&&rr_first_person_active(c->mode,true,c->look,0);
 bool cockpit=rr_cockpit_visible(c->interior!=0,phase,c->look,0,true);
 if(first!=(c->first_person!=0)||cockpit!=(c->cockpit!=0)){if(failures<5)fprintf(stderr,"Visible row%zu\n",n);failures++;}
 if(rr_cockpit_visible(c->interior!=0,phase,c->look,0,false))failures++;
}
/* Native-only error/focus guards; source has no unavailable state. */
RRViewControls view={1,true};if(rr_view_controls_step(&view,RR_SESSION_ERROR,true,true,true,true)||view.mode!=1||!view.interior)failures++;
/* C dispatch occurs before pause: countdown I+pause toggles before pausing;
 * paused I+resume does not. Restart/menu never reset the retained view. */
RRSession session;rr_session_init(&session);session.phase=RR_SESSION_COUNTDOWN;view=(RRViewControls){2,false};
rr_view_controls_step(&view,session.phase,true,true,false,true);if(!rr_session_pause(&session)||!view.interior)failures++;
rr_view_controls_step(&view,session.phase,true,true,true,true);if(view.interior||view.mode!=2||!rr_session_resume(&session))failures++;
rr_view_controls_step(&view,session.phase,true,true,true,true);if(!view.interior||view.mode!=0)failures++;
rr_session_menu(&session);rr_view_controls_step(&view,session.phase,true,true,true,true);if(view.interior||view.mode!=0)failures++;
RRCar car={0};rr_session_start(&session,&car,RR_RACE_TWO_LAPS);if(view.mode!=0||view.interior)failures++;
rr_session_dispose(&session);
printf("Actual browser C/I: %zu dispatch + %zu visibility cases; native focus/error/readiness and lifecycle sequence: %d failures\n",sizeof(view_cases)/sizeof(view_cases[0]),sizeof(visible_cases)/sizeof(visible_cases[0]),failures);
return failures?1:0;}

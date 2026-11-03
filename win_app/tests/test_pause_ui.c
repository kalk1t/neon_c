#include "rr_pause_ui.h"
#include "rr_cockpit_plan.h"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);failures++;}}while(0)
int main(void)
{
    RRSession session;RRCar car;rr_session_init(&session);double stamp=-1;
    RRSessionPhase phases[]={RR_SESSION_COUNTDOWN,RR_SESSION_ACTIVE,RR_SESSION_FINISHED,RR_SESSION_RESULTS};
    for(int p=0;p<4;p++){
        rr_session_start(&session,&car,RR_RACE_TWO_LAPS);session.phase=phases[p];
        CHECK(rr_ui_pause_stamp(&session,&stamp,1000));CHECK(stamp==1000&&session.phase==RR_SESSION_PAUSED);
        CHECK(!rr_ui_pause_stamp(&session,&stamp,1100)&&stamp==1000);
        const double blocked[]={999,1000,1001,1399.999,1400};
        for(size_t i=0;i<sizeof(blocked)/sizeof(blocked[0]);i++)CHECK(!rr_ui_resume_key(&session,stamp,blocked[i],true,true)&&session.phase==RR_SESSION_PAUSED);
        CHECK(!rr_ui_resume_key(&session,stamp,1400.001,false,true));CHECK(!rr_ui_resume_key(&session,stamp,1400.001,true,false));
        CHECK(rr_ui_resume_key(&session,stamp,1400.001,true,true)&&session.phase==phases[p]);
        CHECK(rr_ui_pause_stamp(&session,&stamp,2000));CHECK(rr_session_resume(&session)&&session.phase==phases[p]); /* mouse resume intentionally has no key guard */
        CHECK(rr_ui_pause_stamp(&session,&stamp,3000));rr_session_start(&session,&car,RR_RACE_TWO_LAPS);CHECK(session.phase==RR_SESSION_COUNTDOWN&&session.lap_count==0);
        CHECK(rr_ui_pause_stamp(&session,&stamp,4000));rr_session_menu(&session);CHECK(!rr_ui_resume_key(&session,stamp,5000,true,true)&&session.phase==RR_SESSION_MENU);
    }
    CHECK(!rr_ui_resume_key(&session,0,NAN,true,true));CHECK(!rr_ui_resume_key(&session,NAN,5000,true,true));
    RRCKReadout values={0};values.charge="empty";CHECK(rr_cockpit_boost_rgba(&values)==UINT32_C(0xc4ccd7ff));values.charge="partial";CHECK(rr_cockpit_boost_rgba(&values)==UINT32_C(0xf6f9ffff));
    values.hot=true;CHECK(rr_cockpit_heat_rgba(&values)==UINT32_C(0xffb15aff));values.hot=false;CHECK(rr_cockpit_heat_rgba(&values)==UINT32_C(0xff5adeff));
    rr_session_dispose(&session);printf("Source pause400ms guard: 4 phases, boundary/held/focus/button/restart/menu and v3 cockpit palette: %d failures\n",failures);return failures?1:0;
}

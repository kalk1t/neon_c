#include "rr_pause_scene.h"
#include "rr_showcase.h"
#include "rr_render_policy.h"
#include <stdio.h>
typedef struct SceneTimeCase {int sequence,frame,phase;double now,expected;} SceneTimeCase;
#include "pause_scene_fixture.h"
static int failures;static double worst_float_error;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"pause scene line%d: %s\n",__LINE__,#x);failures++;}}while(0)
int main(void)
{
    RRSceneClock clock={0};int sequence=-1;
    for(size_t i=0;i<sizeof(scene_time_cases)/sizeof(scene_time_cases[0]);i++){
        const SceneTimeCase *c=&scene_time_cases[i];if(sequence!=c->sequence){sequence=c->sequence;clock=(RRSceneClock){0};}
        CHECK(rr_scene_frame_time(&clock,(RRSessionPhase)c->phase,c->now)==c->expected);
    }
    int policies=0;
    for(int phase=0;phase<=RR_SESSION_ERROR;phase++)for(int flags=0;flags<16;flags++){
        bool focused=(flags&1)!=0,smoke=(flags&2)!=0,ready=(flags&4)!=0,overloaded=(flags&8)!=0;
        bool prior_phase=phase==RR_SESSION_ACTIVE||phase==RR_SESSION_COUNTDOWN||phase==RR_SESSION_FINISHED||phase==RR_SESSION_RESULTS;
        bool expected=ready&&(focused||smoke)&&(prior_phase||(phase==RR_SESSION_PAUSED&&!overloaded));
        CHECK(rr_scene_frame_requested((RRSessionPhase)phase,focused,smoke,ready,overloaded)==expected);policies++;
    }
    /* A slow manual showcase pauses GPU work after the same10-frame detector;
       the next frame cannot request3D until explicit resume/restart clears it. */
    int slow=0;bool blocked=false;
    for(int frame=0;frame<10;frame++){CHECK(rr_scene_frame_requested(RR_SESSION_PAUSED,true,false,true,blocked));blocked=rr_render_overloaded(.151,&slow);CHECK(blocked==(frame==9));}
    CHECK(!rr_scene_frame_requested(RR_SESSION_PAUSED,true,false,true,blocked));
    CHECK(rr_scene_frame_requested(RR_SESSION_ACTIVE,true,false,true,false));
    /* Numeric host boundary: raylib stores position/FOV as floats each frame.
       Compare it with a double lane for representative paused views. */
    for(int view=0;view<8;view++){
        RRShowcase a,b;rr_showcase_init(&a);rr_showcase_init(&b);RRVec3 car=rr_v3(view*73-400,115+view*.7,view*35-200),pa=rr_v3(20+view,120-view,-30),pb=pa;double fa=68+view,fb=fa;
        for(int frame=0;frame<600;frame++){
            double dt=1.0/(view%2?30:144);RRShowcasePose x=rr_showcase_pause_step(&a,car,view*.57-3,pa,fa,dt),y=rr_showcase_pause_step(&b,car,view*.57-3,pb,fb,dt);
            pa=x.position;fa=x.fov;pb=rr_v3((float)y.position.x,(float)y.position.y,(float)y.position.z);fb=(float)y.fov;
            double e=fmax(rr_length(rr_sub(pa,pb)),fabs(fa-fb));if(e>worst_float_error)worst_float_error=e;CHECK(e<.001);
        }
    }
    RRSession s;RRCar car={0};rr_session_init(&s);rr_session_start(&s,&car,RR_RACE_TWO_LAPS);s.phase=RR_SESSION_ACTIVE;s.race_time=4;CHECK(rr_session_pause(&s));double t=s.race_time;
    for(int i=0;i<20;i++){CHECK(rr_scene_frame_requested(s.phase,true,false,true,false));rr_session_step(&s,&car,NULL,0,(RRInput){0},.05);CHECK(s.phase==RR_SESSION_PAUSED&&s.race_time==t);}
    CHECK(rr_session_resume(&s));rr_session_dispose(&s);
    printf("Pause scene: %zu actual-source freeze/resume states, %d eligibility combinations,10-frame overload cutoff,4800 float-boundary camera steps; worst error %.12g, %d failures; no GPU\n",sizeof(scene_time_cases)/sizeof(scene_time_cases[0]),policies,worst_float_error,failures);return failures?1:0;
}

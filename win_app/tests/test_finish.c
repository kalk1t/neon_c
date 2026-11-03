#include "rr_finish.h"
#include "rr_session.h"
#include "rr_recovery.h"
#include <stdio.h>
#include "generated_finish_reference.h"
static RRTrack track;
static int failures;static double worst;
#define CHECK(x) do{if(!(x)){printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(e<1e-8);}
static unsigned cross(RRSession *s,RRCar *car)
{
    if(s->race.next_checkpoint<track.gate_count){
        const RRGate *gate=&track.gates[s->race.next_checkpoint];
        rr_car_reset(car,rr_sub(gate->pos,rr_scale(gate->forward,.1)),gate->yaw,35);
        car->pitch=atan2(gate->forward.y,hypot(gate->forward.x,gate->forward.z));
    }else rr_car_reset(car,rr_v3(0,118,.1),0,35);
    return rr_session_step(s,car,track.gates,track.gate_count,(RRInput){0},1.0/30);
}
int main(void)
{
    RRCar car={0};RRSession s;rr_session_init(&s);
    for(size_t i=0;i<sizeof(finish_pilots)/sizeof(finish_pilots[0]);i++){
        const FinishPilotFixture *f=&finish_pilots[i];CHECK(rr_track_init_road(&track,f->road));
        car.pos=f->pos;car.yaw=f->yaw;RRInput c=rr_finish_controls(&track,&car,f->sample);
        near(c.throttle,f->control.throttle);near(c.brake,f->control.brake);near(c.steer,f->control.steer);near(c.climb,f->control.climb);CHECK(c.boost==f->control.boost);
    }
    for(size_t i=0;i<sizeof(finish_results)/sizeof(finish_results[0]);i++){
        const FinishResultFixture *f=&finish_results[i];RRResultSummary result=rr_finish_summary(f->laps,2);near(result.total,f->total);CHECK(result.best==(size_t)f->best);
    }
    for(int road=1;road<=RR_ROAD_COUNT;road++){
        CHECK(rr_track_init_road(&track,road));rr_session_start(&s,&car,RR_RACE_TWO_LAPS);
        while(s.phase==RR_SESSION_COUNTDOWN)rr_session_step(&s,&car,track.gates,track.gate_count,(RRInput){0},1.0/30);
        for(int lap=0;lap<2;lap++){
            for(int gate=0;gate<track.gate_count;gate++)CHECK(cross(&s,&car)==RR_RACE_CHECKPOINT);
            unsigned event=cross(&s,&car);CHECK(event==(unsigned)(RR_RACE_LAP|(lap==1?RR_RACE_FINISH:0)));
        }
        CHECK(s.phase==RR_SESSION_FINISHED && s.lap_count==2);
        double finish=s.race.finish_time,lap_clock=s.race.lap_time,total=rr_finish_summary(s.lap_times,s.lap_count).total;near(total,finish);
        RRRecovery recovery;rr_recovery_init(&recovery,&track,car.pos);RRVec3 initial=car.pos;
        int shows=0;
        for(int frame=0;frame<90;frame++){
            unsigned event=rr_session_step(&s,&car,track.gates,track.gate_count,rr_finish_controls(&track,&car,recovery.sample),1.0/30);
            if(event&RR_SESSION_SHOW_RESULTS)shows++;
            CHECK(!(event&(RR_RACE_LAP|RR_RACE_CHECKPOINT|RR_RACE_FINISH)));
            rr_recovery_update(&recovery,&track,car.pos,car.vel,34,1.0/30,0,false);
        }
        CHECK(shows==1 && s.phase==RR_SESSION_RESULTS);CHECK(rr_length(rr_sub(car.pos,initial))>1);
        near(s.race.finish_time,finish);near(s.race.lap_time,lap_clock);near(rr_finish_summary(s.lap_times,2).total,total);near(s.race_time,finish+3);
        CHECK(rr_session_pause(&s));double clock=s.race_time;
        CHECK(rr_session_step(&s,&car,track.gates,track.gate_count,(RRInput){1,0,0,0,false},5)==0);near(s.race_time,clock);CHECK(rr_session_resume(&s));CHECK(s.phase==RR_SESSION_RESULTS);
        rr_session_start(&s,&car,RR_RACE_TWO_LAPS);CHECK(s.lap_count==0 && !s.race.finished && s.finish_wait==0 && s.phase==RR_SESSION_COUNTDOWN);
        /* Restart/cancel while the old two-second result delay is pending. */
        s.phase=RR_SESSION_FINISHED;s.finish_wait=1.99;rr_session_start(&s,&car,RR_RACE_TWO_LAPS);
        CHECK(rr_session_step(&s,&car,track.gates,track.gate_count,(RRInput){0},.02)==0);CHECK(s.phase==RR_SESSION_COUNTDOWN);
        s.phase=RR_SESSION_FINISHED;s.finish_wait=1.99;rr_session_menu(&s);
        CHECK(rr_session_step(&s,&car,track.gates,track.gate_count,(RRInput){0},10)==0);CHECK(s.phase==RR_SESSION_MENU && s.finish_wait==0);
    }
    rr_session_dispose(&s);
    printf("270 browser autopilot controls; 6 solo summaries; nine-road session completion/motion/results/restart/cancellation; worst %.3g; %d failures.\n",worst,failures);
    return failures?1:0;
}

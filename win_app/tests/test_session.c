#include <stdio.h>
#include "rr_session.h"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static bool near(double a,double b){return fabs(a-b)<1e-9;}
static const RRGate gate={{0,118,-100},{0,0,-1},0};
static unsigned crossing(RRSession *s,RRCar *c,bool finish)
{
    rr_car_place(c,rr_v3(0,118,finish?1:-99),0,95);
    return rr_session_step(s,c,&gate,1,(RRInput){0},0.05);
}
int main(void)
{
    RRSession s;RRCar car;rr_session_init(&s);
    rr_session_start(&s,&car,RR_RACE_TWO_LAPS);
    CHECK(car.pos.x==8 && car.pos.y==115.5 && car.pos.z==64);
    CHECK(rr_session_step(&s,&car,&gate,1,(RRInput){1,0,0,0,true},NAN)==0);
    CHECK(rr_session_step(&s,&car,&gate,1,(RRInput){0},-1)==0);
    CHECK(near(s.countdown,3.999));
    CHECK(rr_session_pause(&s));
    rr_session_step(&s,&car,&gate,1,(RRInput){0},10);
    CHECK(near(s.countdown,3.999));CHECK(rr_session_resume(&s));
    CHECK(s.phase==RR_SESSION_COUNTDOWN);
    unsigned events=0;
    for(int i=0;i<80;i++)events|=rr_session_step(&s,&car,&gate,1,(RRInput){1,0,1,1,true},0.05);
    CHECK(events==RR_SESSION_GO);CHECK(s.race_time==0);CHECK(s.race.lap_time==0);
    CHECK(car.speed==0 && car.pos.z==64); // countdown ignores player input
    CHECK(rr_session_pause(&s));
    rr_session_step(&s,&car,&gate,1,(RRInput){1,0,0,0,false},3);
    CHECK(s.race_time==0);CHECK(rr_session_resume(&s));CHECK(s.phase==RR_SESSION_ACTIVE);
    RRVec3 target={0};CHECK(rr_session_target(&s,&gate,1,&target));CHECK(target.z==-100);
    CHECK(crossing(&s,&car,false)==RR_RACE_CHECKPOINT);
    CHECK(rr_session_target(&s,&gate,1,&target));CHECK(target.y==126 && target.z==0);
    car.boost=.4;car.t=17;car.nitro=.7;car.thrust=.3;car.stun=1;car.surge=1;car.roll_time=.5;
    double clock=s.race_time,lap_time=s.race.lap_time;
    rr_session_respawn(&s,&car);
    CHECK(car.pos.z==-112 && car.speed==35 && car.pitch==0);
    CHECK(car.boost==.4 && car.t==17 && car.nitro==.7 && car.thrust==.3);
    CHECK(car.stun==0 && car.surge==0 && car.roll_time==0);
    CHECK(s.race.next_checkpoint==1 && s.race_time==clock && s.race.lap_time==lap_time);
    CHECK(crossing(&s,&car,true)==RR_RACE_LAP);
    CHECK(s.lap_count==1 && near(s.lap_times[0],.1));
    CHECK(crossing(&s,&car,false)==RR_RACE_CHECKPOINT);
    CHECK(crossing(&s,&car,true)==(RR_RACE_LAP|RR_RACE_FINISH));
    CHECK(s.phase==RR_SESSION_FINISHED && s.lap_count==2);
    CHECK(near(s.lap_times[0]+s.lap_times[1],s.race.finish_time));
    CHECK(!rr_session_target(&s,&gate,1,&target));
    double finish=s.race.finish_time;
    CHECK(rr_session_step(&s,&car,&gate,1,(RRInput){0},1.99)==0);
    CHECK(rr_session_step(&s,&car,&gate,1,(RRInput){0},.02)==RR_SESSION_SHOW_RESULTS);
    CHECK(s.phase==RR_SESSION_RESULTS && s.race.finish_time==finish);
    CHECK(rr_session_step(&s,&car,&gate,1,(RRInput){0},10)==0);
    rr_session_start(&s,&car,RR_RACE_TIME_ATTACK);CHECK(s.lap_count==0 && car.pos.z==42);
    for(int i=0;i<80;i++)rr_session_step(&s,&car,&gate,1,(RRInput){0},.05);
    for(int lap=0;lap<12;lap++){crossing(&s,&car,false);crossing(&s,&car,true);}
    CHECK(s.phase==RR_SESSION_ACTIVE && s.lap_count==12 && !s.race.finished);
    CHECK(s.lap_capacity>=12); // exercises history growth beyond initial allocation
    rr_session_start(&s,&car,RR_RACE_FREE_ROAM);
    CHECK(s.phase==RR_SESSION_ACTIVE && s.lap_count==0);
    CHECK(!rr_session_target(&s,&gate,1,&target));
    crossing(&s,&car,false);crossing(&s,&car,true);CHECK(s.race.next_checkpoint==0 && s.lap_count==0);
    rr_session_menu(&s);CHECK(s.phase==RR_SESSION_MENU);
    CHECK(rr_session_step(&s,&car,&gate,1,(RRInput){0},10)==0);
    rr_session_dispose(&s);CHECK(!s.lap_times && s.lap_capacity==0);
    rr_session_start(&s,&car,RR_RACE_TWO_LAPS);
    rr_session_step(&s,&car,&gate,1,(RRInput){0},2);
    CHECK(near(s.countdown,3.949)); // a long frame advances simulation by 0.05
    CHECK(!rr_session_resume(&s));CHECK(rr_session_pause(&s));CHECK(!rr_session_pause(&s));
    CHECK(rr_session_resume(&s));
    for(int i=0;i<79;i++)rr_session_step(&s,&car,&gate,1,(RRInput){0},.05);
    crossing(&s,&car,false);crossing(&s,&car,true);crossing(&s,&car,false);crossing(&s,&car,true);
    CHECK(s.phase==RR_SESSION_FINISHED);
    rr_session_menu(&s);
    CHECK(rr_session_step(&s,&car,&gate,1,(RRInput){0},5)==0);
    CHECK(s.phase==RR_SESSION_MENU); // returning to menu cancels pending results
    rr_session_dispose(&s);
    for(int rate=30;rate<=144;rate+=(rate==30?30:84)){
        rr_session_start(&s,&car,RR_RACE_TIME_ATTACK);int frames=0;
        while(s.phase==RR_SESSION_COUNTDOWN && frames<1000){rr_session_step(&s,&car,&gate,1,(RRInput){0},1.0/rate);frames++;}
        CHECK(frames==(int)ceil(3.999*rate));CHECK(s.race_time==0);
        rr_session_step(&s,&car,&gate,1,(RRInput){0},1.0/rate);CHECK(near(s.race_time,1.0/rate));
    }
    rr_session_dispose(&s);
    printf("Session lifecycle, clocks, targeting, respawn, history, results and 30/60/144 Hz: %d failures.\n",failures);
    return failures?1:0;
}

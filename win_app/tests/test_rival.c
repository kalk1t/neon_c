#include "rr_rival.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct RivalState {RRLocalRacer racer;double times[4];} RivalState;
typedef struct RivalResetFixture {int road;RivalState before;RRVec3 pos;double yaw,draws[2];RivalState after;} RivalResetFixture;
typedef struct RivalRespawnFixture {int road;RivalState before;bool keep_hp;double draws[2];RivalState after;} RivalRespawnFixture;
typedef struct RivalRulesFixture {int road;RivalState before;double dt,time,draws[2];RivalState after;int spawns;} RivalRulesFixture;
typedef struct RivalStepTrace {int road;RivalState before;} RivalStepTrace;
typedef struct RivalStepFixture {int trace;double dt,time;RRInput input;RivalState after;int spawns,smokes;} RivalStepFixture;
typedef struct RivalContactFixture {size_t count;RivalState before[11],after[11];double shake,after_shake;size_t events;RRCarContactEvent expected[55];size_t hits;double volumes[55];} RivalContactFixture;
#include "generated_rival_reference.h"
static int failures;static double worst;static RRTrack track;static RRLocalRacer racers[11];
#define CHECK(x) do{if(!(x)){if(failures<15)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-7);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
static RRLocalRacer make(const RivalState *f)
{
    RRLocalRacer r=f->racer;r.lap_capacity=8;r.lap_times=calloc(8,sizeof(double));CHECK(r.lap_times!=NULL);if(r.lap_times)memcpy(r.lap_times,f->times,r.lap_count*sizeof(double));return r;
}
static void compare(const RRLocalRacer *r,const RivalState *f)
{
    const RRLocalRacer *e=&f->racer;vector(r->car.pos,e->car.pos);vector(r->car.vel,e->car.vel);vector(r->car.fwd,e->car.fwd);
#define CAR(field) near(r->car.field,e->car.field)
    CAR(yaw);CAR(pitch);CAR(roll);CAR(speed);CAR(boost);CAR(t);CAR(stun);CAR(roll_time);CAR(surge);CAR(max_multiplier);CAR(thrust);CAR(nitro);CAR(kick);CAR(sputter);CAR(impact);CAR(roll_direction);CHECK(r->car.boosting==e->car.boosting);
#undef CAR
    CHECK(r->ai.sample==e->ai.sample);near(r->ai.seed,e->ai.seed);near(r->ai.lane,e->ai.lane);near(r->ai.lane_y,e->ai.lane_y);near(r->ai.dodge,e->ai.dodge);near(r->ai.skill,e->ai.skill);vector(r->previous,e->previous);
    CHECK(r->race.lap==e->race.lap&&r->race.next_checkpoint==e->race.next_checkpoint&&r->race.finished==e->race.finished);near(r->race.lap_time,e->race.lap_time);near(r->race.finish_time,e->race.finish_time);vector(r->race.safe_position,e->race.safe_position);near(r->race.safe_yaw,e->race.safe_yaw);
#define STATE(field) near(r->field,e->field)
    STATE(hp);STATE(wreck);STATE(lost);STATE(stuck);STATE(wrong_way);STATE(smoke_time);STATE(fire_cd);STATE(react);STATE(takedowns);STATE(coins);CHECK(r->passed==e->passed&&r->lap_count==e->lap_count&&r->visible==e->visible&&r->is_player==e->is_player&&!r->error);
#undef STATE
    for(size_t i=0;i<r->lap_count;i++)near(r->lap_times[i],f->times[i]);
}
int main(void)
{
    for(size_t i=0;i<sizeof(rival_resets)/sizeof(*rival_resets);i++){const RivalResetFixture *f=&rival_resets[i];CHECK(rr_track_init_road(&track,f->road));RRLocalRacer r=make(&f->before);CHECK(rr_rival_reset(&r,&track,f->pos,f->yaw,RR_RACE_TWO_LAPS,f->draws[0],f->draws[1]));compare(&r,&f->after);rr_rival_dispose(&r);}
    for(size_t i=0;i<sizeof(rival_respawns)/sizeof(*rival_respawns);i++){const RivalRespawnFixture *f=&rival_respawns[i];CHECK(rr_track_init_road(&track,f->road));RRLocalRacer r=make(&f->before);CHECK(rr_rival_respawn(&r,&track,f->keep_hp,f->draws[0],f->draws[1]));compare(&r,&f->after);rr_rival_dispose(&r);}
    for(size_t i=0;i<sizeof(rival_rules)/sizeof(*rival_rules);i++){const RivalRulesFixture *f=&rival_rules[i];CHECK(rr_track_init_road(&track,f->road));RRLocalRacer r=make(&f->before);unsigned ev=rr_rival_rules(&r,&track,f->dt,f->time,34,f->draws[0],f->draws[1]);CHECK(!!(ev&RR_RIVAL_RESPAWN)==!!f->spawns);compare(&r,&f->after);rr_rival_dispose(&r);}
    int trace=-1;RRLocalRacer r={0};
    for(size_t i=0;i<sizeof(rival_steps)/sizeof(*rival_steps);i++){
        const RivalStepFixture *f=&rival_steps[i];if(trace!=f->trace){rr_rival_dispose(&r);trace=f->trace;CHECK(rr_track_init_road(&track,rival_traces[trace].road));r=make(&rival_traces[trace].before);}
        unsigned ev=rr_rival_step(&r,&track,NULL,f->input,f->dt,f->time,34,.2,.8);CHECK(!!(ev&RR_RIVAL_RESPAWN)==!!f->spawns&&!!(ev&RR_RIVAL_SMOKE)==!!f->smokes);compare(&r,&f->after);
    }rr_rival_dispose(&r);
    size_t pairs=0,hits=0;
    for(size_t i=0;i<sizeof(rival_contacts)/sizeof(*rival_contacts);i++){
        const RivalContactFixture *f=&rival_contacts[i];for(size_t k=0;k<f->count;k++)racers[k]=make(&f->before[k]);RRCarContacts result;CHECK(rr_local_car_contacts(racers,f->count,f->shake,&result));CHECK(result.count==f->events);near(result.shake,f->after_shake);pairs+=result.count;size_t volume=0;
        for(size_t k=0;k<result.count&&k<f->events;k++){const RRCarContactEvent *a=&result.events[k],*b=&f->expected[k];CHECK(a->a==b->a&&a->b==b->b&&a->player_hit==b->player_hit);near(a->relative_speed,b->relative_speed);near(a->hit_volume,b->hit_volume);if(a->player_hit){CHECK(volume<f->hits);if(volume<f->hits)near(a->hit_volume,f->volumes[volume]);volume++;}}
        CHECK(volume==f->hits);hits+=volume;for(size_t k=0;k<f->count;k++){compare(&racers[k],&f->after[k]);rr_rival_dispose(&racers[k]);}
    }
    CHECK(!rr_local_car_contacts(NULL,12,0,NULL));CHECK(!rr_rival_reset(NULL,&track,rr_v3(0,0,0),0,RR_RACE_TWO_LAPS,0,0));
    r=(RRLocalRacer){0};r.is_player=true;CHECK(!rr_rival_respawn(&r,&track,false,.2,.8));r.is_player=false;r.race.mode=RR_RACE_FREE_ROAM;CHECK(!rr_rival_respawn(&r,&track,false,.2,.8));CHECK(!rr_rival_reset(&r,&track,rr_v3(0,118,0),0,RR_RACE_TWO_LAPS,1,.5));
    printf("RIVAL_SUMMARY {\"roads\":9,\"resets\":54,\"respawns\":72,\"recovery_rule_cases\":108,\"two_lap_rule_frames\":252,\"mixed_dt_steps\":1296,\"contact_cases\":84,\"contact_pairs\":%zu,\"player_hit_cues\":%zu,\"max_state_error\":%.12g,\"runtime_rivals\":false,\"network_account_behavior\":false,\"failures\":%d}\n",pairs,hits,worst,failures);return failures?1:0;
}

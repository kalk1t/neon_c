#include "rr_targeting.h"
#include "rr_pack.h"
#include <stdio.h>
#include <string.h>
typedef struct TargetGeometry {RRVec3 player,aim;RRLocalTarget target;double max;bool valid;double angle;} TargetGeometry;
typedef struct LockAngles {int old;double charge,dt,angles[3];bool keep_valid;double keep_angle;int target;double after;unsigned events;} LockAngles;
typedef struct LockTrace {RRVec3 player,aim;} LockTrace;
typedef struct LockFrame {int trace;double dt;RRLocalTarget targets[3];int target;double charge;unsigned events;} LockFrame;
typedef struct LeadFixture {RRVec3 from;RRLocalTarget target;double speed;RRVec3 expected;} LeadFixture;
typedef struct PhaseFixture {int action,collections,coins;bool active;double timer,pop;} PhaseFixture;
#include "generated_targeting_reference.h"
static int failures;static double worst;static RRTrack track;static RRLocalPack pack;static RRLocalRacer player;
#define CHECK(x) do{if(!(x)){if(failures<12)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-12);}
int main(void)
{
    for(size_t i=0;i<sizeof(geometry)/sizeof(*geometry);i++){const TargetGeometry *f=&geometry[i];double angle=0;bool valid=rr_target_angle(f->player,f->aim,&f->target,f->max,&angle);CHECK(valid==f->valid);if(valid)near(angle,f->angle);}
    for(size_t i=0;i<sizeof(angle_cases)/sizeof(*angle_cases);i++){const LockAngles *f=&angle_cases[i];RRLocalLock s={f->old,f->charge};unsigned events=rr_lock_angles(&s,f->angles,3,f->keep_valid,f->keep_angle,f->dt);CHECK(s.target==f->target&&events==f->events);near(s.charge,f->after);}
    int trace=-1;RRLocalLock lock;
    for(size_t i=0;i<sizeof(frames)/sizeof(*frames);i++){const LockFrame *f=&frames[i];if(trace!=f->trace){trace=f->trace;rr_lock_init(&lock);}unsigned events=rr_lock_update(&lock,traces[trace].player,traces[trace].aim,f->targets,3,f->dt);CHECK(events==f->events&&lock.target==f->target);near(lock.charge,f->charge);}
    for(size_t i=0;i<sizeof(leads)/sizeof(*leads);i++){const LeadFixture *f=&leads[i];RRVec3 direction;CHECK(rr_target_lead(f->from,&f->target,f->speed,&direction));near(direction.x,f->expected.x);near(direction.y,f->expected.y);near(direction.z,f->expected.z);}
    CHECK(rr_track_init_road(&track,1));
    for(size_t i=0;i<sizeof(phases)/sizeof(*phases);i++){
        const PhaseFixture *f=&phases[i];pack=(RRLocalPack){0};player=(RRLocalRacer){0};player.is_player=true;player.hp=100;rr_car_reset(&player.car,rr_v3(0,118,0),0,0);rr_race_reset(&player.race,RR_RACE_TWO_LAPS,player.car.pos,0);pack.player=&player;pack.track=&track;
        for(int r=0;r<10;r++){RRLocalRacer *p=&pack.rivals[r];p->react=1000;p->hp=100;rr_car_reset(&p->car,rr_v3(10000+r*20,118,10000),0,0);rr_race_reset(&p->race,RR_RACE_TWO_LAPS,p->car.pos,0);}
        pack.coins.count=1;pack.coins.list[0]=(RRLocalCoin){{0,118,0},0,true,0,0};RRPackEvents e;CHECK(rr_pack_rivals_after_player(&pack,RR_PACK_PLAYING,.05,1,1,NULL,NULL,NULL,&e));CHECK(pack.coin_pending&&player.coins==0&&pack.coins.list[0].pop==0);
        RRPackEvents duplicate;CHECK(!rr_pack_rivals_after_player(&pack,RR_PACK_PLAYING,.05,1,1,NULL,NULL,NULL,&duplicate));
        if(f->action==1){player.wreck=2.4;player.hp=0;}if(f->action==2)player.car.pos.x=100;if(f->action==3)player.race.finished=true;
        RRCoinEvents coins;CHECK(rr_pack_coins_after_weapons(&pack,&coins));CHECK(coins.count==f->collections&&player.coins==f->coins&&pack.coins.list[0].active==f->active&&!pack.coin_pending);near(pack.coins.list[0].timer,f->timer);near(pack.coins.list[0].pop,f->pop);CHECK(!rr_pack_coins_after_weapons(&pack,&coins));rr_pack_dispose(&pack);
    }
    RRVec3 direction;CHECK(!rr_target_lead(rr_v3(0,0,0),NULL,430,&direction));CHECK(!rr_target_lead(rr_v3(0,0,0),&leads[0].target,0,&direction));
    printf("TARGETING_SUMMARY {\"geometry_cases\":576,\"angle_branch_cases\":384,\"live_lock_frames\":480,\"lead_cases\":256,\"late_coin_probes\":4,\"max_error\":%.12g,\"production_combat\":false,\"damage_rewards_network\":false,\"failures\":%d}\n",worst,failures);return failures?1:0;
}

#include <stdio.h>
#include "rr_recovery.h"
#include "rr_flight.h"
typedef struct RecoveryFixture {
    int scenario,roam;
    double along,off,height,dt,message,wrong,lost;
    unsigned events;
    RRVec3 pos;
    double speed,boost;
} RecoveryFixture;
typedef struct TangentFixture {int index;RRVec3 tangent;} TangentFixture;
typedef struct NearestFixture {int hint;RRVec3 pos;int local,global;} NearestFixture;
#include "generated_recovery_reference.h"
static int failures;
static double worst;
static void near(double got,double want,const char *label,size_t row)
{
    double error=fabs(got-want);if(error>worst)worst=error;
    if(!isfinite(got)||error>1e-8){if(failures<8)fprintf(stderr,"%s[%zu]: %.17g != %.17g\n",label,row,got,want);failures++;}
}
static void vector(RRVec3 got,RRVec3 want,const char *label,size_t row)
{near(got.x,want.x,label,row);near(got.y,want.y,label,row);near(got.z,want.z,label,row);}
int main(void)
{
    RRRecovery recovery={0};int scenario=-1;
    size_t count=sizeof(recovery_fixtures)/sizeof(recovery_fixtures[0]);
    for(size_t i=0;i<count;i++){
        const RecoveryFixture *f=&recovery_fixtures[i];
        if(scenario!=f->scenario){recovery=(RRRecovery){0};scenario=f->scenario;}
        unsigned events=rr_recovery_conditions(&recovery,f->along,f->off,f->height,34,f->dt,f->message,f->roam!=0);
        near(events,f->events,"events",i);near(recovery.wrong_way,f->wrong,"wrong way",i);near(recovery.lost_time,f->lost,"lost time",i);
        if(events&RR_RECOVERY_RESPAWN){
            RRCar car;rr_car_reset(&car,rr_v3(0,0,0),0,42);car.boost=.4;
            rr_car_place(&car,rr_v3(0,118,12),0,35);
            vector(car.pos,f->pos,"browser respawn position",i);
            near(car.speed,f->speed,"browser respawn speed",i);near(car.boost,f->boost,"browser respawn boost",i);
        }
    }
    static RRTrack track;
    if(!rr_track_init(&track)){fputs("Track init failed\n",stderr);return 1;}
    for(size_t i=0;i<sizeof(tangent_fixtures)/sizeof(tangent_fixtures[0]);i++){
        const TangentFixture *f=&tangent_fixtures[i];vector(track.tangents[f->index],f->tangent,"arc tangent",i);
    }
    for(size_t i=0;i<sizeof(nearest_fixtures)/sizeof(nearest_fixtures[0]);i++){
        const NearestFixture *f=&nearest_fixtures[i];near(rr_track_nearest_local(&track,f->pos,f->hint),f->local,"local nearest",i);
        near(rr_track_nearest(&track,f->pos),f->global,"global nearest",i);
    }
    rr_recovery_init(&recovery,&track,track.samples[0]);
    RRVec3 pos=rr_add(track.samples[0],rr_v3(0,0.1,0));
    unsigned events=rr_recovery_update(&recovery,&track,pos,rr_scale(track.tangents[0],-30),34,.05,0,false);
    near(events,0,"first reversed step",0);near(recovery.wrong_way,.05,"projected velocity",0);
    recovery.lost_time=.5;rr_recovery_rebase(&recovery,&track,pos);
    near(recovery.lost_time,0,"respawn clears lost timer",0);near(recovery.wrong_way,.05,"respawn preserves wrong timer",0);
    printf("%zu actual browser recovery steps; 83 tangent and 28 nearest cases; worst error %.3g; %d failures.\n",count,worst,failures);
    return failures?1:0;
}

#include <stdio.h>
#include "rr_race.h"
typedef struct RaceFixture {
    int scenario,mode,offset,count;
    RRVec3 previous,current;
    double dt,clock;
    int lap,next,completed,finished;
    double lap_time,last,best,finish_time,boost;
    RRVec3 safe;
    double yaw;
    unsigned events;
} RaceFixture;
#include "generated_race_reference.h"
static int failures;
static double worst;
static void near_value(double actual,double expected,size_t row,const char *field)
{
    double error=fabs(actual-expected);
    if(error>worst)worst=error;
    if(!isfinite(actual) || error>1e-8){
        if(failures<8)fprintf(stderr,"row %zu %s: %.17g != %.17g\n",row,field,actual,expected);
        failures++;
    }
}
#define CHECK(field,expected) near_value((double)(field),(double)(expected),i,#field)
int main(void)
{
    RRRace race={0};RRCar car={0};int scenario=-1;
    size_t count=sizeof(race_fixtures)/sizeof(race_fixtures[0]);
    for(size_t i=0;i<count;i++){
        const RaceFixture *f=&race_fixtures[i];
        if(f->scenario!=scenario){
            rr_race_reset(&race,(RRRaceMode)f->mode,rr_v3(0,118,70),0);
            car.boost=0.1;scenario=f->scenario;
        }
        car.pos=f->current;
        unsigned events=rr_race_step(&race,race_gates+f->offset,f->count,&car,f->previous,f->dt,f->clock);
        CHECK(race.lap,f->lap);CHECK(race.next_checkpoint,f->next);
        CHECK(race.completed_laps,f->completed);CHECK(race.finished,f->finished);
        CHECK(race.lap_time,f->lap_time);CHECK(race.last_lap_time,f->last);
        CHECK(race.best_lap_time,f->best);CHECK(race.finish_time,f->finish_time);
        CHECK(car.boost,f->boost);CHECK(race.safe_position.x,f->safe.x);
        CHECK(race.safe_position.y,f->safe.y);CHECK(race.safe_position.z,f->safe.z);
        CHECK(race.safe_yaw,f->yaw);CHECK(events,f->events);
    }
    printf("%zu actual browser race-rule steps, %d scenarios; worst error %.3g; %d failures.\n",count,scenario+1,worst,failures);
    return failures?1:0;
}

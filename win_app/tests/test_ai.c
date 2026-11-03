#include "rr_ai.h"
#include <stdio.h>
#include <string.h>
typedef struct AIInputFixture {int road;RRAIDriver driver;RRCar car;RRAIPeer peers[3];RRVec3 target,coin;bool has_coin;double time,dt;RRInput expected;double dodge;} AIInputFixture;
typedef struct AITraceFixture {int road;RRAIDriver driver;RRCar car;} AITraceFixture;
typedef struct AITraceStep {int trace,sample;RRVec3 target,coin;bool has_coin;double time,dt;RRInput input;double dodge;RRCar car;} AITraceStep;
typedef struct AIProgressFixture {int sample,lap,next,n;double progress;} AIProgressFixture;
typedef struct AIRubberFixture {int road,n;double player,rival,factor;} AIRubberFixture;
#include "generated_ai_reference.h"
static int failures;static double worst_input,worst_car;static RRTrack track,straight;
#define CHECK(x) do{if(!(x)){if(failures<12)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b,double tolerance,double *worst){double e=fabs(a-b);if(e>*worst)*worst=e;CHECK(isfinite(a)&&isfinite(b)&&e<tolerance);}
static void input_near(RRInput a,RRInput b){near(a.throttle,b.throttle,1e-8,&worst_input);near(a.brake,b.brake,1e-8,&worst_input);near(a.steer,b.steer,1e-8,&worst_input);near(a.climb,b.climb,1e-8,&worst_input);CHECK(a.boost==b.boost);}
static void car_near(const RRCar *a,const RRCar *b)
{
    near(rr_length(rr_sub(a->pos,b->pos)),0,1e-5,&worst_car);near(rr_length(rr_sub(a->vel,b->vel)),0,1e-5,&worst_car);near(rr_length(rr_sub(a->fwd,b->fwd)),0,1e-5,&worst_car);
    near(a->yaw,b->yaw,1e-5,&worst_car);near(a->pitch,b->pitch,1e-5,&worst_car);near(a->roll,b->roll,1e-5,&worst_car);near(a->speed,b->speed,1e-5,&worst_car);near(a->boost,b->boost,1e-5,&worst_car);CHECK(a->boosting==b->boosting);
}
int main(void)
{
    int loaded=0;
    for(size_t i=0;i<sizeof(ai_inputs)/sizeof(*ai_inputs);i++){
        const AIInputFixture *f=&ai_inputs[i];if(loaded!=f->road){CHECK(rr_track_init_road(&track,f->road));loaded=f->road;}
        RRAIDriver d=f->driver;RRInput input;CHECK(rr_ai_controls(&d,&f->car,&track,f->peers,3,f->target,f->time,f->dt,f->has_coin?&f->coin:NULL,&input));input_near(input,f->expected);near(d.dodge,f->dodge,1e-8,&worst_input);
    }
    int trace=-1;RRCar car={0};RRAIDriver driver={0};
    for(size_t i=0;i<sizeof(ai_steps)/sizeof(*ai_steps);i++){
        const AITraceStep *s=&ai_steps[i];if(s->trace!=trace){trace=s->trace;const AITraceFixture *f=&ai_traces[trace];CHECK(rr_track_init_road(&track,f->road));car=f->car;driver=f->driver;}
        driver.sample=rr_track_nearest(&track,car.pos);CHECK(driver.sample==s->sample);RRInput input;
        CHECK(rr_ai_controls(&driver,&car,&track,NULL,0,s->target,s->time,s->dt,s->has_coin?&s->coin:NULL,&input));input_near(input,s->input);near(driver.dodge,s->dodge,1e-8,&worst_input);rr_car_step(&car,input,s->dt);car_near(&car,&s->car);
    }
    for(size_t i=0;i<sizeof(ai_progress)/sizeof(*ai_progress);i++){const AIProgressFixture *f=&ai_progress[i];near(rr_ai_progress(f->sample,f->lap,f->next,f->n),f->progress,1e-12,&worst_input);}
    for(size_t i=0;i<sizeof(ai_rubber)/sizeof(*ai_rubber);i++){const AIRubberFixture *f=&ai_rubber[i];CHECK(rr_road_by_id(f->road)!=NULL);near(rr_ai_speed_factor(f->player,f->rival,f->n,rr_road_by_id(f->road)->ai_speed),f->factor,1e-12,&worst_input);}
    for(int i=0;i<RR_AI_RIVALS;i++){const RRAILivery *l=rr_ai_livery(i);CHECK(l&&strlen(l->name)>0&&l->skill>=.92&&l->skill<=1&&l->paint<=0xffffffu&&l->accent<=0xffffffu);}
    /* Strict avoidance thresholds, dead peers and last-in-source-order override. */
    straight.sample_count=64;for(int i=0;i<64;i++){straight.samples[i]=rr_v3(0,100,-20);straight.tangents[i]=rr_v3(0,0,-1);}
    rr_car_reset(&car,rr_v3(0,100,0),0,0);driver=(RRAIDriver){0,0,0,0,0,1};RRInput ctl;RRVec3 target=rr_v3(0,100,-1000);RRAIPeer peers[2]={{ {5.5,100,-10},true },{ {1,100,-32},true }};
    CHECK(rr_ai_controls(&driver,&car,&straight,peers,2,target,0,0,NULL,&ctl));CHECK(driver.dodge==0);
    peers[0].pos.x=1;peers[1].pos=rr_v3(-1,100,-15);CHECK(rr_ai_controls(&driver,&car,&straight,peers,2,target,0,.05,NULL,&ctl));near(driver.dodge,7*exp(-.06),1e-12,&worst_input);
    driver.dodge=0;peers[0].alive=peers[1].alive=false;CHECK(rr_ai_controls(&driver,&car,&straight,peers,2,target,0,0,NULL,&ctl));CHECK(driver.dodge==0);
    CHECK(!rr_ai_controls(NULL,&car,&straight,NULL,0,target,0,.01,NULL,&ctl));CHECK(!rr_ai_controls(&driver,&car,&straight,NULL,1,target,0,.01,NULL,&ctl));CHECK(!rr_ai_livery(-1)&&!rr_ai_livery(10));
    printf("AI_SUMMARY {\"roads\":9,\"drivers\":10,\"input_cases\":288,\"coupled_traces\":18,\"coupled_steps\":3240,\"progress_cases\":108,\"rubber_band_cases\":45,\"max_input_error\":%.12g,\"max_car_error\":%.12g,\"runtime_rivals\":false,\"failures\":%d}\n",worst_input,worst_car,failures);return failures?1:0;
}

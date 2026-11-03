#include <stdio.h>
#include <string.h>
#include "rr_track.h"
#include "rr_race.h"
typedef struct RoadFixture {int id;const char *name;double ai,fire,length;int samples,gates,sample_offset,gate_offset;} RoadFixture;
typedef struct RoadSampleFixture {RRVec3 pos,tangent;} RoadSampleFixture;
typedef struct RoadGateFixture {RRVec3 pos,forward;double yaw;int sample;} RoadGateFixture;
typedef struct RoadCurveFixture {int road;double u;RRVec3 point,at,tangent,tangent_at;} RoadCurveFixture;
typedef struct RoadNearestFixture {int road,hint;RRVec3 pos;int local,global;} RoadNearestFixture;
#include "generated_all_roads_reference.h"
static int failures;
static double worst;
static void near(double got,double want,const char *label,int road,int row)
{
    double error=fabs(got-want);if(error>worst)worst=error;
    if(!isfinite(got)||error>1e-8){if(failures<12)fprintf(stderr,"road %d %s[%d]: %.17g != %.17g\n",road,label,row,got,want);failures++;}
}
static void vector(RRVec3 got,RRVec3 want,const char *label,int road,int row)
{near(got.x,want.x,label,road,row);near(got.y,want.y,label,road,row);near(got.z,want.z,label,road,row);}
int main(void)
{
    static RRTrack track;
    size_t sample_total=0,gate_total=0;
    for(size_t r=0;r<sizeof(road_fixtures)/sizeof(road_fixtures[0]);r++){
        const RoadFixture *f=&road_fixtures[r];
        if(!rr_track_init_road(&track,f->id)){fprintf(stderr,"road %d init failed\n",f->id);return 1;}
        const RRRoadDefinition *road=track.road;
        near(road->id,f->id,"id",f->id,0);
        if(strcmp(road->name,f->name)!=0){fprintf(stderr,"road %d name mismatch\n",f->id);failures++;}
        near(road->ai_speed,f->ai,"AI metadata",f->id,0);near(road->fire_interval,f->fire,"fire metadata",f->id,0);
        near(track.length,f->length,"length",f->id,0);near(track.sample_count,f->samples,"sample count",f->id,0);near(track.gate_count,f->gates,"gate count",f->id,0);
        if(track.sample_count!=f->samples||track.gate_count!=f->gates)return 1;
        for(int i=0;i<track.sample_count;i++){
            const RoadSampleFixture *p=&road_sample_fixtures[f->sample_offset+i];
            vector(track.samples[i],p->pos,"sample",f->id,i);vector(track.tangents[i],p->tangent,"tangent",f->id,i);
        }
        for(int i=0;i<track.gate_count;i++){
            const RoadGateFixture *g=&road_gate_fixtures[f->gate_offset+i];
            vector(track.gates[i].pos,g->pos,"gate position",f->id,i);vector(track.gates[i].forward,g->forward,"gate forward",f->id,i);
            near(track.gates[i].yaw,g->yaw,"gate yaw",f->id,i);near(track.gate_samples[i],g->sample,"gate sample",f->id,i);
        }
        for(size_t i=0;i<sizeof(road_curve_fixtures)/sizeof(road_curve_fixtures[0]);i++){
            const RoadCurveFixture *c=&road_curve_fixtures[i];if(c->road!=f->id)continue;
            vector(rr_track_point_for(&track,c->u),c->point,"raw curve",f->id,(int)i);
            vector(rr_track_tangent_for(&track,c->u),c->tangent,"raw tangent",f->id,(int)i);
            vector(rr_track_point_at(&track,c->u),c->at,"arc curve",f->id,(int)i);
            vector(rr_track_tangent_at(&track,c->u),c->tangent_at,"arc tangent",f->id,(int)i);
        }
        for(size_t i=0;i<sizeof(road_nearest_fixtures)/sizeof(road_nearest_fixtures[0]);i++){
            const RoadNearestFixture *n=&road_nearest_fixtures[i];if(n->road!=f->id)continue;
            near(rr_track_nearest_local(&track,n->pos,n->hint),n->local,"local nearest",f->id,(int)i);
            near(rr_track_nearest(&track,n->pos),n->global,"global nearest",f->id,(int)i);
        }
        RRRace race;RRCar car={0};double clock=0;
        rr_race_reset(&race,RR_RACE_TWO_LAPS,rr_v3(8,115.5,64),0);
        for(int lap=0;lap<2;lap++){
            for(int i=0;i<track.gate_count;i++){
                const RRGate *g=&track.gates[i];clock+=.02;
                RRVec3 previous=rr_sub(g->pos,rr_scale(g->forward,20));
                car.pos=rr_add(g->pos,rr_scale(g->forward,20));
                unsigned events=rr_race_step(&race,track.gates,track.gate_count,&car,previous,.02,clock);
                near(events,RR_RACE_CHECKPOINT,"coupled checkpoint",f->id,i);
                near(race.next_checkpoint,i+1,"coupled checkpoint order",f->id,i);
            }
            clock+=.02;car.pos=rr_v3(0,118,-1);
            unsigned events=rr_race_step(&race,track.gates,track.gate_count,&car,rr_v3(0,118,1),.02,clock);
            near(events,RR_RACE_LAP|(lap==1?RR_RACE_FINISH:0),"coupled lap",f->id,lap);
        }
        near(race.completed_laps,2,"coupled completed laps",f->id,0);
        near(race.finished,1,"coupled finished",f->id,0);
        sample_total+=(size_t)track.sample_count;gate_total+=(size_t)track.gate_count;
    }
    const int invalid_ids[]={-1,0,RR_ROAD_COUNT+1};
    for(size_t i=0;i<sizeof(invalid_ids)/sizeof(invalid_ids[0]);i++)
        if(!rr_track_init_road(&track,invalid_ids[i])||track.road->id!=1)failures++;
    printf("9 browser roads; %zu sample/tangent pairs; %zu gates; 729 curve and 189 nearest cases; two-lap integration on every road; worst error %.3g; %d failures.\n",sample_total,gate_total,worst,failures);
    return failures?1:0;
}

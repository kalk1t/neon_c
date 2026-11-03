#include <stdio.h>
#include "rr_city_plan.h"
typedef struct CorridorQueryFixture {int context;double x,z,radius,top,margin,distance,height;int clear;} CorridorQueryFixture;
typedef struct CityFitFixture {int context;double x,z,w,d,top,random;int accepted;double fitted;int calls,steps;} CityFitFixture;
typedef struct CorridorPointFixture {int index;RRVec3 point;} CorridorPointFixture;
#include "generated_city_clearance_reference.h"
static int failures;
static double worst;
static void near(double got,double want,const char *label,size_t row)
{
    double error=fabs(got-want);if(error>worst)worst=error;
    if(!isfinite(got)||error>1e-8){if(failures<10)fprintf(stderr,"%s[%zu] %.17g != %.17g\n",label,row,got,want);failures++;}
}
static void vector(RRVec3 got,RRVec3 want,const char *label,size_t row)
{near(got.x,want.x,label,row);near(got.y,want.y,label,row);near(got.z,want.z,label,row);}
typedef struct FixedRandom {double value;int calls;} FixedRandom;
static double fixed_random(void *context){FixedRandom *r=context;r->calls++;return r->value;}
int main(void)
{
    static RRCorridor corridor;static RRTrack scratch;static RRCityPlan plan,again;
    for(int context=0;context<4;context++){
        bool initialized=false;
        if(context==0)initialized=rr_corridor_init_all(&corridor,&scratch);
        if(context==1){const RRVec3 points[]={{0,260,0},{10,160,0},{-161,118,-161},{161,90,161}};initialized=rr_corridor_init_points(&corridor,points,4,NULL,0);}
        if(context==2){const RRVec3 points[]={{0,200,0}};initialized=rr_corridor_init_points(&corridor,points,1,NULL,0);}
        if(context==3){const RRVec3 points[]={{500,200,500}};const RRGate gates[]={{{0,118,0},{0,0,-1},0}};initialized=rr_corridor_init_points(&corridor,points,1,gates,1);}
        if(!initialized){fputs("Corridor initialization failed\n",stderr);return 1;}
        if(context==0){
            near(corridor.point_count,reference_corridor_counts[0],"full count",0);near(corridor.coarse_count,reference_corridor_counts[1],"coarse count",0);
            near(corridor.gate_count,reference_corridor_counts[2],"gate count",0);near(corridor.occupied_cells,reference_corridor_counts[3],"grid count",0);
            vector(corridor.minimum,reference_corridor_min,"bounds min",0);vector(corridor.maximum,reference_corridor_max,"bounds max",0);
            for(size_t i=0;i<sizeof(corridor_coarse_fixtures)/sizeof(corridor_coarse_fixtures[0]);i++){const CorridorPointFixture *f=&corridor_coarse_fixtures[i];vector(corridor.coarse[f->index],f->point,"coarse order",i);}
            for(size_t i=0;i<sizeof(corridor_point_fixtures)/sizeof(corridor_point_fixtures[0]);i++){const CorridorPointFixture *f=&corridor_point_fixtures[i];vector(corridor.points[f->index],f->point,"point order",i);}
        }
        for(size_t i=0;i<sizeof(corridor_queries)/sizeof(corridor_queries[0]);i++){
            const CorridorQueryFixture *f=&corridor_queries[i];if(f->context!=context)continue;
            RRNearestXZ nearest=rr_corridor_nearest_xz(&corridor,f->x,f->z);
            near(nearest.distance,f->distance,"nearest distance",i);near(nearest.height,f->height,"nearest height",i);
            near(rr_corridor_clear(&corridor,f->x,f->z,f->radius,f->top,f->margin),f->clear,"clearance",i);
        }
        for(size_t i=0;i<sizeof(city_fit_fixtures)/sizeof(city_fit_fixtures[0]);i++){
            const CityFitFixture *f=&city_fit_fixtures[i];if(f->context!=context)continue;
            FixedRandom random={f->random,0};RRCityFit fit=rr_city_fit(&corridor,f->x,f->z,f->w,f->d,f->top,fixed_random,&random);
            near(fit.accepted,f->accepted,"fit accepted",i);near(fit.top,f->fitted,"fit top",i);near(random.calls,f->calls,"fit random calls",i);near(fit.steps,f->steps,"fit steps",i);
        }
    }
    if(!rr_corridor_init_all(&corridor,&scratch))return 1;
    rr_city_plan(&plan,&corridor);int intersections=0;
    for(int b=0;b<plan.count;b++){
        const RRPlannedBuilding *building=&plan.buildings[b];
        /* Analytical native body/roof AABB plus 4 units, at every real sampled
         * route center. This is not a continuous mesh/collision proof. */
        for(int i=0;i<corridor.point_count;i++){
            RRVec3 p=corridor.points[i];
            if(p.y>=-4 && p.y<=building->top+1.7+4 && fabs(p.x-building->x)<=building->width*.5+.5+4 && fabs(p.z-building->z)<=building->depth*.5+.5+4)intersections++;
        }
    }
    near(intersections,0,"sampled route/AABB intersections",0);
    for(int road=1;road<=RR_ROAD_COUNT;road++){
        if(!rr_track_init_road(&scratch,road))return 1;
        rr_city_plan(&again,&corridor);near(again.count,plan.count,"road-independent city count",road);
        for(int i=0;i<plan.count;i++){
            const RRPlannedBuilding *a=&plan.buildings[i],*b=&again.buildings[i];
            near(a->x,b->x,"stable x",i);near(a->z,b->z,"stable z",i);near(a->top,b->top,"stable top",i);
            near(a->width,b->width,"stable width",i);near(a->depth,b->depth,"stable depth",i);near(a->pattern,b->pattern,"stable pattern",i);
            near(a->red,b->red,"stable red",i);near(a->green,b->green,"stable green",i);near(a->blue,b->blue,"stable blue",i);
        }
    }
    printf("%zu browser corridor queries; %zu fitting cases; city %d buildings (%d lowered, %d rejected); %d sampled route/AABB intersections; all-road independence; worst error %.3g; %d failures.\n",sizeof(corridor_queries)/sizeof(corridor_queries[0]),sizeof(city_fit_fixtures)/sizeof(city_fit_fixtures[0]),plan.count,plan.lowered,plan.rejected,intersections,worst,failures);
    return failures?1:0;
}

#include "rr_shared_scenery.h"
#include "rr_world_plan.h"
#include "rr_atmosphere.h"
#include "rr_shared_material.h"
#include <stdio.h>
typedef struct SharedPropFixture {int kind,index;RRVec3 pos,right,up,back;} SharedPropFixture;
typedef struct SharedColliderFixture {double x,z,r,y0,y1;} SharedColliderFixture;
typedef struct NeonFixture {double rgb[3],amount;} NeonFixture;
typedef struct FogFixture {double depth,amount;} FogFixture;
#include "generated_shared_scenery_reference.h"
static RRCorridor corridor;static RRTrack skyline,selected;static RRSharedScenery scenery,again;static RRCourse course;static RRWorld world;static RRCityPlan city;
static int failures;static double worst;
#define CHECK(x) do{if(!(x)){if(failures<10)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&e<1e-8);}
static void vec(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
int main(void)
{
    CHECK(rr_corridor_init_all(&corridor,&skyline));CHECK(rr_shared_scenery_plan(&scenery,&skyline,&corridor));
    CHECK(scenery.prop_count==39 && scenery.platforms==18 && scenery.columns==9 && scenery.trees==21 && scenery.collider_count==48);
    for(size_t i=0;i<sizeof(shared_props)/sizeof(shared_props[0]);i++){
        const SharedPropFixture *f=&shared_props[i];int at=0;const RRSharedProp *prop=NULL;
        for(int j=0;j<scenery.prop_count;j++)if(scenery.props[j].kind==f->kind && at++==f->index){prop=&scenery.props[j];break;}
        CHECK(prop);if(prop){vec(prop->pos,f->pos);vec(rr_v3(cos(prop->yaw)*prop->scale,0,-sin(prop->yaw)*prop->scale),f->right);vec(rr_v3(0,prop->scale,0),f->up);vec(rr_v3(sin(prop->yaw)*prop->scale,0,cos(prop->yaw)*prop->scale),f->back);}
    }
    for(size_t i=0;i<sizeof(shared_colliders)/sizeof(shared_colliders[0]);i++){
        const SharedColliderFixture *f=&shared_colliders[i];const RRShape *shape=&scenery.colliders[i];CHECK(shape->kind==RR_SHAPE_CYLINDER && !shape->off);
        near(shape->center.x,f->x);near(shape->center.z,f->z);near(shape->radius,f->r);near(shape->y0,f->y0);near(shape->y1,f->y1);
        RRVec3 point=rr_v3(f->x+f->r-.1,(f->y0+f->y1)/2,f->z);CHECK(rr_shape_resolve(shape,point,2.1).hit);
    }
    for(int i=0;i<scenery.prop_count;i++)if(scenery.props[i].kind<=RR_SHARED_COLUMN){
        RRVec3 p=scenery.props[i].pos;CHECK(rr_corridor_clear(&corridor,p.x,p.z,17,p.y+14,40));
        for(int gate=0;gate<corridor.gate_count;gate++)CHECK(rr_length(rr_sub(p,corridor.gates[gate].pos))>=70);
    }
    for(int road=1;road<=RR_ROAD_COUNT;road++){
        CHECK(rr_track_init_road(&selected,road));CHECK(rr_course_plan(&course,&selected));CHECK(rr_shared_scenery_plan(&again,&skyline,&corridor));CHECK(again.prop_count==scenery.prop_count);
        for(int i=0;i<again.prop_count;i++){vec(again.props[i].pos,scenery.props[i].pos);near(again.props[i].yaw,scenery.props[i].yaw);}
        for(int roam=0;roam<=1;roam++){
            CHECK(rr_world_plan_shared(&world,&city,&course,&scenery,roam!=0));CHECK(world.count==9+48+course.collider_count);
            for(int i=0;i<48;i++){CHECK(!world.shapes[9+i].off);vec(world.shapes[9+i].center,scenery.colliders[i].center);}
            for(int i=57;i<world.count;i++)CHECK(world.shapes[i].off==(roam!=0));
        }
    }
    CHECK(!rr_shared_scenery_plan(&again,&selected,&corridor)); /* Road 9 cannot silently replace shared Skyline base. */
    for(size_t i=0;i<sizeof(fog_fixtures)/sizeof(fog_fixtures[0]);i++)near(rr_fog_amount(fog_fixtures[i].depth),fog_fixtures[i].amount);
    for(int i=0;i<3;i++)near(rr_fog_color_srgb[i],shared_fog_srgb_reference[i]);
    for(size_t i=0;i<sizeof(neon_fixtures)/sizeof(neon_fixtures[0]);i++){const NeonFixture *f=&neon_fixtures[i];near(rr_sculpt_neon(f->rgb[0],f->rgb[1],f->rgb[2]),f->amount);}
    CHECK(RR_CYAN_TREE_GLOW==2.4 && RR_CYAN_TREE_HEIGHT==13);
    CHECK(RR_FOG_DENSITY==.00062);CHECK(rr_fog_color_linear[0]>0 && rr_fog_color_linear[2]>rr_fog_color_linear[0]);
    printf("Shared scenery: 39 browser transforms, 48 ordered solid shapes, all-road clearance/independence/roam; 10 actual FogExp2 and 15 source sculpt-mask cases; worst %.3g; %d failures.\n",worst,failures);return failures?1:0;
}

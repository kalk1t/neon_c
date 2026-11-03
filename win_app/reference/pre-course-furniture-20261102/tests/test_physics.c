#include "rr_world_plan.h"
#include "rr_session.h"
#include <stdio.h>
#include <string.h>
typedef struct ShapeSpec {int kind;RRVec3 center,half;double yaw,radius,tube,y0,y1;} ShapeSpec;
typedef struct ShapeCase {int shape;RRVec3 pos;double radius;bool hit;RRVec3 normal;double depth;bool nonfinite;} ShapeCase;
typedef struct WorldCase {int world;RRVec3 pos,result;double radius;bool hit;RRVec3 normal;double depth;} WorldCase;
typedef struct SegmentCase {int world;RRVec3 a,b;bool hit;RRVec3 point;} SegmentCase;
typedef struct CollisionFlightCase {int scenario;double dt;RRInput input;double expected[17];} CollisionFlightCase;
typedef struct CourseShapeCase {int road,index;ShapeSpec shape;} CourseShapeCase;
#include "generated_physics_reference.h"
static RRWorld world;static RRTrack track;static RRCorridor corridor;static RRCityPlan city;
static int failures;static double worst;
#define CHECK(x) do{if(!(x)){if(failures<12)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&e<1e-8);}
static void vec(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
static RRShape make(ShapeSpec s){return s.kind==0?rr_box(s.center,s.half,s.yaw):s.kind==1?rr_cylinder(s.center.x,s.center.z,s.radius,s.y0,s.y1):rr_torus(s.center,s.yaw,s.radius,s.tube);}
static void fixture_world(int id)
{
    const int ids[4][4]={{0,3,-1,-1},{1,4,-1,-1},{5,6,-1,-1},{7,0,3,-1}};world.count=0;
    for(int i=0;i<4 && ids[id][i]>=0;i++)CHECK(rr_world_add(&world,make(physics_specs[ids[id][i]])));
}
int main(void)
{
    for(size_t i=0;i<sizeof(shape_cases)/sizeof(shape_cases[0]);i++){
        const ShapeCase *f=&shape_cases[i];RRShape shape=make(physics_specs[f->shape]);RRContact c=rr_shape_resolve(&shape,f->pos,f->radius);
        if(f->nonfinite && rr_length(rr_sub(f->pos,shape.center))==0){CHECK(!c.hit);continue;}
        CHECK(c.hit==f->hit);if(c.hit){near(c.depth,f->depth);if(f->nonfinite){CHECK(isfinite(c.normal.x)&&isfinite(c.normal.y)&&isfinite(c.normal.z));near(rr_length(c.normal),1);}else vec(c.normal,f->normal);}
    }
    for(size_t i=0;i<sizeof(world_cases)/sizeof(world_cases[0]);i++){
        const WorldCase *f=&world_cases[i];fixture_world(f->world);RRVec3 p=f->pos;RRContact c=rr_world_collide(&world,&p,f->radius);
        CHECK(c.hit==f->hit);vec(p,f->result);if(c.hit){vec(c.normal,f->normal);near(c.depth,f->depth);}
    }
    for(size_t i=0;i<sizeof(segment_cases)/sizeof(segment_cases[0]);i++){
        const SegmentCase *f=&segment_cases[i];fixture_world(f->world);RRVec3 point={0};bool hit=rr_world_segment(&world,f->a,f->b,.6,&point);CHECK(hit==f->hit);if(hit)vec(point,f->point);
    }
    RRCar car={0};int scenario=-1;
    for(size_t i=0;i<sizeof(collision_flight_cases)/sizeof(collision_flight_cases[0]);i++){
        const CollisionFlightCase *f=&collision_flight_cases[i];
        if(f->scenario!=scenario){scenario=f->scenario;world.count=0;rr_car_reset(&car,rr_v3(0,118,0),0,60);
            if(scenario==0)rr_world_add(&world,rr_box(rr_v3(0,118,-20),rr_v3(20,20,2),0));
            if(scenario==1)rr_world_add(&world,rr_cylinder(0,-20,6,100,140));
            if(scenario==2)rr_world_add(&world,rr_torus(rr_v3(0,118,-20),0,15.5,2.45));
        }
        rr_car_step_world(&car,f->input,f->dt,&world);
        const double values[]={car.pos.x,car.pos.y,car.pos.z,car.vel.x,car.vel.y,car.vel.z,car.yaw,car.pitch,car.roll,car.speed,car.boost,car.roll_time,car.surge,car.thrust,car.nitro,car.stun,car.impact};
        for(int j=0;j<17;j++)near(values[j],f->expected[j]);
    }
    CHECK(rr_corridor_init_all(&corridor,&track));rr_city_plan(&city,&corridor);
    for(int road=1;road<=RR_ROAD_COUNT;road++){
        CHECK(rr_track_init_road(&track,road));CHECK(rr_world_plan(&world,&city,&track,false));CHECK(world.count==city.count+9+track.gate_count*3);
        for(size_t i=0;i<sizeof(course_shape_cases)/sizeof(course_shape_cases[0]);i++){
            const CourseShapeCase *f=&course_shape_cases[i];if(f->road!=road)continue;
            RRShape expected=make(f->shape),*actual=&world.shapes[city.count+f->index];CHECK(actual->kind==expected.kind);vec(actual->center,expected.center);vec(actual->half,expected.half);
            near(actual->cosine,expected.cosine);near(actual->sine,expected.sine);near(actual->radius,expected.radius);near(actual->tube,expected.tube);near(actual->y0,expected.y0);near(actual->y1,expected.y1);
        }
        /* A legal gate-center crossing remains clear; the frame is solid. */
        for(int i=0;i<track.gate_count;i++){
            RRShape ring=world.shapes[city.count+9+i*3];CHECK(!rr_shape_resolve(&ring,track.gates[i].pos,2.1).hit);
            RRVec3 right=rr_v3(cos(track.gates[i].yaw),0,-sin(track.gates[i].yaw));CHECK(rr_shape_resolve(&ring,rr_add(track.gates[i].pos,rr_scale(right,14)),2.1).hit);
        }
        CHECK(rr_world_plan(&world,&city,&track,true));CHECK(!world.shapes[city.count].off);CHECK(world.shapes[city.count+2].off);CHECK(!world.shapes[city.count+3].off);
        for(int i=city.count+9;i<world.count;i++)CHECK(world.shapes[i].off);
        /* Countdown/active/finished all use the same optional collision world. */
        RRSession session;rr_session_init(&session);rr_session_start(&session,&car,RR_RACE_TWO_LAPS);session.world=&world;
        fixture_world(3);session.world=&world;car.pos=rr_v3(0,118,-17.9);car.vel=rr_v3(0,0,-60);car.speed=60;
        rr_session_step(&session,&car,track.gates,track.gate_count,(RRInput){0},1.0/30);CHECK(car.impact>0);CHECK(car.pos.z>=-15.9-1e-8);
        session.phase=RR_SESSION_FINISHED;car.pos=rr_v3(0,118,-17.9);car.vel=rr_v3(0,0,-60);car.speed=60;
        rr_session_step(&session,&car,track.gates,track.gate_count,(RRInput){0},1.0/30);CHECK(car.impact>0);CHECK(session.lap_count==0);
        rr_session_dispose(&session);
    }
    fixture_world(0);for(int i=0;i<world.count;i++)world.shapes[i].off=true;RRVec3 p=rr_v3(4,0,0);CHECK(!rr_world_collide(&world,&p,.1).hit);
    printf("Browser collision: %zu shapes, %zu ordered cell queries, %zu segments, 720 car steps, %zu course shapes across nine roads; finite torus-center guard; worst %.3g; %d failures.\n",sizeof(shape_cases)/sizeof(shape_cases[0]),sizeof(world_cases)/sizeof(world_cases[0]),sizeof(segment_cases)/sizeof(segment_cases[0]),sizeof(course_shape_cases)/sizeof(course_shape_cases[0]),worst,failures);
    return failures?1:0;
}

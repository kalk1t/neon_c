#include "rr_canonical_city.h"
#include "rr_city_crown_data.h"
#include "rr_world_plan.h"
#include "rr_shared_scenery.h"
#include <stdio.h>
typedef struct CanonicalCollisionFixture {RRVec3 pos,result;bool hit;RRVec3 normal;double depth;} CanonicalCollisionFixture;
typedef struct CrownMatrixFixture {int index;RRVec3 pos,right,up,back;} CrownMatrixFixture;
#include "generated_canonical_city_reference.h"
#include "generated_city_crown_reference.h"
static RRCorridor corridor;static RRTrack track;static RRCityPlan prototype,city;static RRWorld world;static RRCourse course;static RRSharedScenery shared;
static int failures;static double worst,crown_worst;
#define CHECK(x) do{if(!(x)){if(failures<10)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&e<1e-8);}
static void vec(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
static void matrix_vec(RRVec3 a,RRVec3 b){double e=rr_length(rr_sub(a,b));if(e>crown_worst)crown_worst=e;CHECK(e<.0002);}
int main(void)
{
    CHECK(rr_corridor_init_all(&corridor,&track));CHECK(rr_shared_scenery_plan(&shared,&track,&corridor));rr_city_plan(&prototype,&corridor);CHECK(rr_canonical_city_plan(&city,&corridor));
    CHECK(rr_canonical_city_count()==413 && rr_canonical_hero_count()==115 && city.canonical_count==413);
    CHECK(city.prototype_kept+city.prototype_removed==prototype.count);CHECK(city.count==city.prototype_kept+city.canonical_count && city.count<=RR_CITY_MAX_BUILDINGS);
    for(int i=0;i<prototype.count;i++){
        const RRPlannedBuilding *old=&prototype.buildings[i];int matches=0;
        for(int j=0;j<city.count;j++)if(city.buildings[j].canonical_index<0 && city.buildings[j].x==old->x && city.buildings[j].z==old->z){matches++;near(city.buildings[j].top,old->top);near(city.buildings[j].width,old->width);near(city.buildings[j].depth,old->depth);}
        CHECK(matches==(rr_canonical_city_contains(old->x,old->z)?0:1));
    }
    world.count=0;unsigned int vertices=0,max_vertices=0;bool mesh_ids[700]={0};
    for(int i=0;i<rr_canonical_city_count();i++){
        const RRCanonicalBuilding *b=rr_canonical_city_building(i);RRShape shape;CHECK(rr_canonical_city_collider(i,&shape));CHECK(rr_world_add(&world,shape));
        CHECK(b->mesh_id>=0 && b->mesh_id<rr_canonical_city_count());
        if(b->mesh_id>=0 && b->mesh_id<700){CHECK(!mesh_ids[b->mesh_id]);mesh_ids[b->mesh_id]=true;}
        CHECK(rr_canonical_city_contains(b->center.x,b->center.z));near(b->center.y-b->half.y,-260);
        CHECK(b->gx>=17&&b->gx<=47&&b->gz>=21&&b->gz<=44);vertices+=b->vertices;if(b->vertices>max_vertices)max_vertices=b->vertices;
    }
    CHECK(vertices==275520 && max_vertices==2208);
    for(size_t i=0;i<sizeof(canonical_collision_fixtures)/sizeof(canonical_collision_fixtures[0]);i++){
        const CanonicalCollisionFixture *f=&canonical_collision_fixtures[i];RRVec3 p=f->pos;int before=failures;RRContact hit=rr_world_collide(&world,&p,2.1);CHECK(hit.hit==f->hit);vec(p,f->result);if(hit.hit){vec(hit.normal,f->normal);near(hit.depth,f->depth);}if(failures!=before)printf("Collision case %zu pos %.17g %.17g %.17g result got %.17g %.17g %.17g expected %.17g %.17g %.17g normal %.17g %.17g %.17g expected %.17g %.17g %.17g depth %.17g / %.17g\n",i,f->pos.x,f->pos.y,f->pos.z,p.x,p.y,p.z,f->result.x,f->result.y,f->result.z,hit.normal.x,hit.normal.y,hit.normal.z,f->normal.x,f->normal.y,f->normal.z,hit.depth,f->depth);
    }
    int intersections=0;for(int i=0;i<corridor.point_count;i++){RRVec3 p=corridor.points[i];if(rr_world_collide(&world,&p,2.1).hit)intersections++;}CHECK(intersections==RR_CANONICAL_ROUTE_INTERSECTIONS && intersections==0);
    for(size_t i=0;i<sizeof(crown_matrices)/sizeof(crown_matrices[0]);i++){
        const CrownMatrixFixture *f=&crown_matrices[i];RRHeroPlacement p;CHECK(rr_canonical_city_hero_placement(f->index,rr_city_crown_model_size,&p));
        matrix_vec(p.pos,f->pos);matrix_vec(rr_v3(cos(p.yaw)*p.scale,0,-sin(p.yaw)*p.scale),f->right);matrix_vec(rr_v3(0,p.scale,0),f->up);matrix_vec(rr_v3(sin(p.yaw)*p.scale,0,cos(p.yaw)*p.scale),f->back);
        RRHeroBounds bounds=rr_hero_world_bounds(rr_city_crown_bounds,p);CHECK(bounds.maximum.y>bounds.minimum.y);
    }
    int max_world=0;
    for(int road=1;road<=RR_ROAD_COUNT;road++){
        CHECK(rr_track_init_road(&track,road));CHECK(rr_course_plan(&course,&track));CHECK(rr_world_plan_shared(&world,&city,&course,&shared,false));
        CHECK(world.count==city.count+9+shared.collider_count+course.collider_count && world.count<=RR_PHYSICS_MAX_SHAPES);if(world.count>max_world)max_world=world.count;
    }
    CHECK(!rr_canonical_city_building(-1)&&!rr_canonical_city_building(413));CHECK(!rr_canonical_city_hero(115));
    printf("Canonical fixed core: kept=%d removed=%d source=%d total=%d; 1239 browser collision queries, 18052 route samples (%d contacts), 115 source crown matrices; max world=%d/1024, city=%d/700; resident mesh worst 64*%u*36 bytes; errors %.3g / Float32 crown %.3g; %d failures.\n",city.prototype_kept,city.prototype_removed,city.canonical_count,city.count,intersections,max_world,city.count,max_vertices,worst,crown_worst,failures);return failures?1:0;
}

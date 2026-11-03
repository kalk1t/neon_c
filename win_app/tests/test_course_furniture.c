#include "rr_course_plan.h"
#include "rr_world_plan.h"
#include <stdio.h>
typedef struct ShapeSpec {int kind;RRVec3 center,half;double yaw,radius,tube,y0,y1;} ShapeSpec;
typedef struct CourseCountFixture {int road,dots,kinds[5],colliders;} CourseCountFixture;
typedef struct CourseDotFixture {int road,index;RRVec3 pos;} CourseDotFixture;
typedef struct CourseItemFixture {int road,kind,index;RRVec3 pos,right,up,back;} CourseItemFixture;
typedef struct CourseColliderFixture {int road,index;ShapeSpec shape;} CourseColliderFixture;
#include "generated_course_furniture_reference.h"
static RRTrack track;static RRCourse course,again;static RRWorld world;static RRCityPlan city;
static int failures;static double worst,dot_worst;
#define CHECK(x) do{if(!(x)){if(failures<10)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&e<1e-8);}
static void vec(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
static RRShape make(ShapeSpec s){return s.kind==0?rr_box(s.center,s.half,s.yaw):s.kind==1?rr_cylinder(s.center.x,s.center.z,s.radius,s.y0,s.y1):rr_torus(s.center,s.yaw,s.radius,s.tube);}
int main(void)
{
    for(int road=1;road<=RR_ROAD_COUNT;road++){
        CHECK(rr_track_init_road(&track,road));CHECK(rr_course_plan(&course,&track));const CourseCountFixture *count=&course_counts[road-1];
        CHECK(count->road==road && course.dot_count==count->dots && course.collider_count==count->colliders);
        int kinds[5]={0};for(int i=0;i<course.item_count;i++)kinds[course.items[i].kind]++;
        for(int i=0;i<5;i++)CHECK(kinds[i]==count->kinds[i]);
        for(size_t i=0;i<sizeof(course_dots)/sizeof(course_dots[0]);i++){
            const CourseDotFixture *f=&course_dots[i];if(f->road!=road)continue;
            double e=rr_length(rr_sub(course.dots[f->index],f->pos));if(e>dot_worst)dot_worst=e;CHECK(e<.0005); /* browser instance matrices are Float32 */
        }
        for(size_t i=0;i<sizeof(course_items)/sizeof(course_items[0]);i++){
            const CourseItemFixture *f=&course_items[i];if(f->road!=road)continue;int at=0;const RRCourseItem *item=NULL;
            for(int j=0;j<course.item_count;j++)if((int)course.items[j].kind==f->kind && at++==f->index){item=&course.items[j];break;}
            CHECK(item!=NULL);if(item){vec(item->pos,f->pos);vec(item->right,f->right);vec(item->up,f->up);vec(item->back,f->back);}
        }
        for(size_t i=0;i<sizeof(course_colliders)/sizeof(course_colliders[0]);i++){
            const CourseColliderFixture *f=&course_colliders[i];if(f->road!=road)continue;
            RRShape expected=make(f->shape),actual=course.colliders[f->index];CHECK(actual.kind==expected.kind);
            vec(actual.center,expected.center);vec(actual.half,expected.half);near(actual.cosine,expected.cosine);near(actual.sine,expected.sine);near(actual.radius,expected.radius);near(actual.tube,expected.tube);near(actual.y0,expected.y0);near(actual.y1,expected.y1);
            /* Every new furniture collider actually participates in world contact. */
            if(actual.kind==RR_SHAPE_BOX){RRVec3 point=actual.center;CHECK(rr_shape_resolve(&actual,point,2.1).hit);}
            if(actual.kind==RR_SHAPE_CYLINDER){RRVec3 point=rr_v3(actual.center.x+actual.radius-.1,(actual.y0+actual.y1)/2,actual.center.z);CHECK(rr_shape_resolve(&actual,point,2.1).hit);}
        }
        CHECK(rr_world_plan(&world,&city,&track,false));CHECK(world.count==9+course.collider_count);
        for(int i=0;i<course.collider_count;i++){CHECK(!world.shapes[i+9].off);vec(world.shapes[i+9].center,course.colliders[i].center);}
        CHECK(rr_world_plan(&world,&city,&track,true));for(int i=9;i<world.count;i++)CHECK(world.shapes[i].off);
        CHECK(rr_course_plan(&again,&track));CHECK(again.item_count==course.item_count && again.collider_count==course.collider_count);
        /* Starting another route replaces, rather than appends to, furniture. */
        CHECK(rr_track_init_road(&track,road==9?1:road+1));CHECK(rr_course_plan(&again,&track));CHECK(rr_world_plan(&world,&city,&track,false));CHECK(world.count==9+again.collider_count);
    }
    printf("Nine-road furniture: %zu Float32 dot matrices, %zu prefab transforms, %zu ordered colliders; road replacement/roam/contact; worst %.3g, dot quantization %.3g; %d failures.\n",sizeof(course_dots)/sizeof(course_dots[0]),sizeof(course_items)/sizeof(course_items[0]),sizeof(course_colliders)/sizeof(course_colliders[0]),worst,dot_worst,failures);
    return failures?1:0;
}

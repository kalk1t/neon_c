#include "rr_world_plan.h"
#include <stdlib.h>
bool rr_world_plan_course(RRWorld *w,const RRCityPlan *city,const RRCourse *course,bool roam)
{
    w->count=0;
    for(int i=0;i<city->count;i++){
        const RRPlannedBuilding *b=&city->buildings[i];
        if(!rr_world_add(w,rr_box(rr_v3(b->x,b->top*.5,b->z),rr_v3(b->width*.5,b->top*.5,b->depth*.5),0)))return false;
    }
    if(!rr_world_add(w,rr_box(rr_v3(0,107,20),rr_v3(50,4,80),0)))return false;
    if(!rr_world_add(w,rr_box(rr_v3(0,102,20),rr_v3(36,3,62),0)))return false;
    for(int side=-1;side<=1;side+=2){
        RRShape post=rr_box(rr_v3(side*26,131,0),rr_v3(3.4,20,3.6),0);post.off=roam;if(!rr_world_add(w,post))return false;
        if(!rr_world_add(w,rr_box(rr_v3(side*22,-89,20),rr_v3(6,190,6),0)))return false;
    }
    RRShape bar=rr_box(rr_v3(0,146,0),rr_v3(30,4.4,3.4),0);bar.off=roam;if(!rr_world_add(w,bar))return false;
    if(!rr_world_add(w,rr_cylinder(-80,10,7,-400,155)))return false;
    if(!rr_world_add(w,rr_cylinder(-80,10,16.8,152,179)))return false;
    for(int i=0;i<course->collider_count;i++){RRShape shape=course->colliders[i];shape.off=roam;if(!rr_world_add(w,shape))return false;}
    return true;
}

bool rr_world_plan(RRWorld *w,const RRCityPlan *city,const RRTrack *track,bool roam)
{
    RRCourse *course=malloc(sizeof(*course));if(!course)return false;
    bool ready=rr_course_plan(course,track)&&rr_world_plan_course(w,city,course,roam);
    free(course);return ready;
}

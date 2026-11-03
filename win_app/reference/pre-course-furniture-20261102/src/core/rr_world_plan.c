#include "rr_world_plan.h"
bool rr_world_plan(RRWorld *w,const RRCityPlan *city,const RRTrack *track,bool roam)
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
    for(int i=0;i<track->gate_count;i++){
        const RRGate *gate=&track->gates[i];double base=gate->pos.y-22;
        RRShape shapes[]={rr_torus(gate->pos,gate->yaw,15.5,2.45),rr_cylinder(gate->pos.x,gate->pos.z,13.7,base-12,base+2.6),rr_cylinder(gate->pos.x,gate->pos.z,5.2,base-480,base-12)};
        for(int j=0;j<3;j++){shapes[j].off=roam;if(!rr_world_add(w,shapes[j]))return false;}
    }
    return true;
}

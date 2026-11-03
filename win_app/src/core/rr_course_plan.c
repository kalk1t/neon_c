#include "rr_course_plan.h"
#include <stdint.h>
static double random_unit(uint32_t *state)
{
    *state+=0x6d2b79f5u;uint32_t a=*state,t=(a^(a>>15))*(1u|a);
    t=(t+((t^(t>>7))*(61u|t)))^t;return (double)(t^(t>>14))/4294967296.0;
}
static RRVec3 cross(RRVec3 a,RRVec3 b){return rr_v3(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x);}
static RRVec3 right_of(RRVec3 t){return rr_normalize(cross(t,rr_v3(0,1,0)));}
static double turn(const RRTrack *t,int i){int n=t->sample_count;RRVec3 a=t->tangents[(i-12+n)%n],b=t->tangents[(i+12)%n];return -atan2(a.x*b.z-a.z*b.x,a.x*b.x+a.z*b.z);}
static bool near_checkpoint(const RRTrack *t,RRVec3 p,double distance)
{
    if(rr_length(rr_sub(p,rr_v3(0,118,0)))<distance+40)return true;
    for(int i=0;i<t->gate_count;i++)if(rr_length(rr_sub(p,t->gates[i].pos))<distance)return true;
    return false;
}
static bool collider(RRCourse *c,RRShape s){if(c->collider_count>=RR_COURSE_MAX_COLLIDERS)return false;c->colliders[c->collider_count++]=s;return true;}
static bool item(RRCourse *c,RRCourseKind kind,RRVec3 p,double yaw)
{
    if(c->item_count>=RR_COURSE_MAX_ITEMS)return false;
    RRCourseItem *it=&c->items[c->item_count++];*it=(RRCourseItem){0};it->kind=kind;it->pos=p;it->yaw=yaw;
    it->right=rr_v3(cos(yaw),0,-sin(yaw));it->up=rr_v3(0,1,0);it->back=rr_v3(sin(yaw),0,cos(yaw));return true;
}
bool rr_course_plan(RRCourse *c,const RRTrack *t)
{
    c->dot_count=c->item_count=c->collider_count=c->beacons=c->barriers=c->signs=c->chevrons=0;
    if(t->sample_count<=0)return false;
    for(int i=0;i<t->gate_count;i++){
        const RRGate *g=&t->gates[i];double base=g->pos.y-22;
        if(!collider(c,rr_torus(g->pos,g->yaw,15.5,2.45))||!collider(c,rr_cylinder(g->pos.x,g->pos.z,13.7,base-12,base+2.6))||!collider(c,rr_cylinder(g->pos.x,g->pos.z,5.2,base-480,base-12)))return false;
        for(int side=-1;side<=1;side+=2){double lx=side*16.7*sqrt(.5)*.97,py=22-16.7*sqrt(.5);
            if(!collider(c,rr_box(rr_v3(g->pos.x+lx*cos(g->yaw),base+py/2+1.5,g->pos.z-lx*sin(g->yaw)),rr_v3(2,py/2+2.6,3.4),g->yaw)))return false;
        }
    }
    c->dot_count=(int)floor(t->length/7);if(c->dot_count>RR_COURSE_MAX_DOTS)return false;
    for(int i=0;i<c->dot_count;i++){c->dots[i]=rr_track_point_at(t,(double)i/c->dot_count);c->dots[i].y-=2.5;}
    for(double distance=60;distance<t->length-60;distance+=84){
        double u=distance/t->length;RRVec3 p=rr_track_point_at(t,u),tan=rr_track_tangent_at(t,u);if(near_checkpoint(t,p,45))continue;
        RRVec3 right=right_of(tan),up=cross(right,tan);p=rr_sub(p,rr_scale(up,3));
        if(!item(c,RR_COURSE_CHEVRON,p,0))return false;
        RRCourseItem *it=&c->items[c->item_count-1];it->right=right;it->up=up;it->back=rr_scale(tan,-1);c->chevrons++;
    }
    uint32_t rng=(uint32_t)(2024+t->road->id);
    for(double distance=140;distance<t->length-80;distance+=125){
        int i=(int)floor(distance/t->length*t->sample_count);RRVec3 p=t->samples[i],right=right_of(t->tangents[i]);
        if(near_checkpoint(t,p,70)||fabs(turn(t,i))>.3)continue;
        for(int side=-1;side<=1;side+=2){RRVec3 q=rr_add(p,rr_scale(right,side*31));q.y-=9.5;
            if(!item(c,RR_COURSE_BEACON,q,random_unit(&rng)*6.28)||!collider(c,rr_cylinder(q.x,q.z,2.4,q.y,q.y+14)))return false;c->beacons++;
        }
    }
    int last_sign=-999;
    for(int i=0;i<t->sample_count;i+=8){
        double bend=turn(t,i);if(fabs(bend)<.4)continue;
        RRVec3 p=t->samples[i],tan=t->tangents[i],right=right_of(tan);if(near_checkpoint(t,p,55))continue;
        int outside=bend>0?1:-1;RRVec3 q=rr_add(p,rr_scale(right,outside*36));q.y+=1;double yaw=atan2(-tan.z,tan.x);
        if(!item(c,RR_COURSE_BARRIER,q,yaw)||!collider(c,rr_box(q,rr_v3(11,3,1.8),yaw)))return false;c->barriers++;
        if(i-last_sign>60){int si=(i-32+t->sample_count)%t->sample_count;RRVec3 st=t->tangents[si],sq=rr_add(t->samples[si],rr_scale(right_of(st),outside*27));sq.y+=9;double syaw=atan2(-st.x,-st.z);
            if(!near_checkpoint(t,sq,50)){if(!item(c,bend>0?RR_COURSE_SIGN_NEG:RR_COURSE_SIGN_POS,sq,syaw)||!collider(c,rr_box(sq,rr_v3(8.6,3.8,1.2),syaw)))return false;c->signs++;}
        }
        /* Browser updates this after every accepted barrier, not only signs. */
        last_sign=i;
    }
    return true;
}

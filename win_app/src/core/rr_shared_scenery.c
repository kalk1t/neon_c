#include "rr_shared_scenery.h"
#include <stdint.h>
static double random_unit(uint32_t *state){*state+=0x6d2b79f5u;uint32_t a=*state,t=(a^(a>>15))*(1u|a);t=(t+((t^(t>>7))*(61u|t)))^t;return (double)(t^(t>>14))/4294967296.0;}
static bool prop(RRSharedScenery *s,RRVec3 pos,int kind,double yaw,double scale){if(s->prop_count>=64)return false;s->props[s->prop_count++]=(RRSharedProp){pos,yaw,scale,kind};return true;}
static bool cylinder(RRSharedScenery *s,RRVec3 p,double r,double y0,double y1){if(s->collider_count>=128)return false;s->colliders[s->collider_count++]=rr_cylinder(p.x,p.z,r,y0,y1);return true;}
static bool tree(RRSharedScenery *s,RRVec3 p,bool cyan,double scale,uint32_t *rng)
{
    if(!prop(s,p,cyan?RR_SHARED_TREE_C:RR_SHARED_TREE_M,random_unit(rng)*6.28,scale)||!cylinder(s,p,4*scale,p.y,p.y+11*scale))return false;s->trees++;return true;
}
static bool near_checkpoint(const RRCorridor *c,RRVec3 p,double d)
{
    if(rr_length(rr_sub(p,rr_v3(0,118,0)))<d+40)return true;
    for(int i=0;i<c->gate_count;i++)if(rr_length(rr_sub(p,c->gates[i].pos))<d)return true;
    return false;
}
bool rr_shared_scenery_plan(RRSharedScenery *s,const RRTrack *base,const RRCorridor *corridor)
{
    *s=(RRSharedScenery){0};if(!base->road||base->road->id!=1)return false;uint32_t rng=2024;
    for(int k=0;k<26;k++){
        RRVec3 pos;
        if(k<16){
            int i=(int)floor(((k+.5)/16)*base->sample_count);RRVec3 p=base->samples[i],t=base->tangents[i];
            RRVec3 right=rr_normalize(rr_v3(-t.z,0,t.x));double side=random_unit(&rng)<.5?-1:1;
            pos=rr_add(p,rr_scale(right,side*(65+random_unit(&rng)*70)));pos.y-=20+random_unit(&rng)*40;
        }else{
            RRVec3 p=base->samples[(int)floor(random_unit(&rng)*base->sample_count)];
            double x=p.x+(random_unit(&rng)-.5)*700,y=70+random_unit(&rng)*180,z=p.z+(random_unit(&rng)-.5)*700;pos=rr_v3(x,y,z);
        }
        if(near_checkpoint(corridor,pos,70)||!rr_corridor_clear(corridor,pos.x,pos.z,17,pos.y+14,40))continue;
        bool column=random_unit(&rng)<.5;
        if(!prop(s,pos,column?RR_SHARED_COLUMN:RR_SHARED_PLATFORM,random_unit(&rng)*6.28,1)||!cylinder(s,pos,16.6,pos.y-8,pos.y+.2))return false;
        s->platforms++;if(column){s->columns++;if(!cylinder(s,pos,4.6,pos.y-430,pos.y-8))return false;}
        if(random_unit(&rng)<.75){double angle=random_unit(&rng)*6.28;RRVec3 at=rr_add(pos,rr_v3(cos(angle)*8,0,sin(angle)*8));bool cyan=random_unit(&rng)<.5;if(!tree(s,at,cyan,1.1,&rng))return false;}
    }
    const double x[]={-40,40,-40,40},z[]={-48,-48,88,88};const bool cyan[]={false,true,true,false};
    for(int i=0;i<4;i++)if(!tree(s,rr_v3(x[i],111,z[i]),cyan[i],1.15,&rng))return false;
    return true;
}

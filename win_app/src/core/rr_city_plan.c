#include "rr_city_plan.h"
RRCityFit rr_city_fit(const RRCorridor *c,double x,double z,double w,double d,double top,RRCityRandom random,void *context)
{
    RRCityFit fit={false,top,0};double radius=hypot(w,d)/2;
    RRNearestXZ nearest=rr_corridor_nearest_xz(c,x,z);
    if(fabs(x)<120 && z>-140 && z<180)fit.top=fmin(fit.top,55);
    for(int i=0;i<c->gate_count;i++)if(hypot(c->gates[i].pos.x-x,c->gates[i].pos.z-z)<radius+26)return fit;
    if(!rr_corridor_clear(c,x,z,radius,fit.top+70,68)){
        fit.top=fmin(fit.top,nearest.height-60-random(context)*50);
        if(!rr_corridor_clear(c,x,z,radius,fit.top+40,50))fit.top=fmin(fit.top,nearest.height-110);
        while(fit.steps<40 && fit.top>=45 && !rr_corridor_clear(c,x,z,radius,fit.top+40,50)){
            fit.top-=15;fit.steps++;
        }
    }
    fit.accepted=fit.top>=45;return fit;
}
typedef struct CityRandom {uint32_t state;} CityRandom;
static float unit(CityRandom *r){r->state=r->state*1664525u+1013904223u;return (float)(r->state>>8)/16777216.0f;}
static double lower_random(void *context){return unit((CityRandom *)context);}
void rr_city_plan(RRCityPlan *plan,const RRCorridor *corridor)
{
    *plan=(RRCityPlan){0};CityRandom random={2024};
    for(int x=-12;x<=12;x++)for(int z=-12;z<=12;z++){
        float px=x*160.0f+(unit(&random)-.5f)*65,pz=z*160.0f+(unit(&random)-.5f)*65;
        float w=32+unit(&random)*36,d=28+unit(&random)*48,h=65+unit(&random)*300;
        RRCityFit fit=rr_city_fit(corridor,px,pz,w,d,h,lower_random,&random);
        if(!fit.accepted){plan->rejected++;continue;}
        if(plan->count>=RR_CITY_MAX_BUILDINGS)return;
        if(fit.top<h)plan->lowered++;
        RRPlannedBuilding *b=&plan->buildings[plan->count++];
        b->canonical_index=-1;b->x=px;b->z=pz;b->width=w;b->depth=d;b->top=(float)fit.top;
        b->pattern=(int)(unit(&random)*3);
        b->red=(uint8_t)(18+unit(&random)*10);b->green=(uint8_t)(17+unit(&random)*10);b->blue=(uint8_t)(38+unit(&random)*18);
    }
}

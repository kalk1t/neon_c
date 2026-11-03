#include "rr_canonical_city.h"
#include <stddef.h>
#include "rr_canonical_city_data.h"
int rr_canonical_city_count(void){return RR_CANONICAL_COUNT;}
int rr_canonical_hero_count(void){return RR_CANONICAL_HERO_COUNT;}
bool rr_canonical_city_contains(double x,double z){return x>=RR_CANONICAL_MIN_X&&x<RR_CANONICAL_MAX_X&&z>=RR_CANONICAL_MIN_Z&&z<RR_CANONICAL_MAX_Z;}
const RRCanonicalBuilding *rr_canonical_city_building(int index){return index>=0&&index<RR_CANONICAL_COUNT?&rr_canonical_buildings[index]:NULL;}
const RRCanonicalHero *rr_canonical_city_hero(int index){return index>=0&&index<RR_CANONICAL_HERO_COUNT?&rr_canonical_heroes[index]:NULL;}
bool rr_canonical_city_collider(int index,RRShape *shape)
{
    const RRCanonicalBuilding *b=rr_canonical_city_building(index);if(!b||!shape)return false;*shape=rr_box(b->center,b->half,b->yaw);
    /* Preserve the browser's basis rounding at diagonal nearest-face ties. */
    shape->cosine=b->cosine;shape->sine=b->sine;return true;
}
bool rr_canonical_city_plan(RRCityPlan *plan,const RRCorridor *corridor)
{
    rr_city_plan(plan,corridor);int kept=0;
    for(int i=0;i<plan->count;i++)if(!rr_canonical_city_contains(plan->buildings[i].x,plan->buildings[i].z))plan->buildings[kept++]=plan->buildings[i];
    plan->prototype_removed=plan->count-kept;plan->prototype_kept=kept;plan->canonical_count=0;plan->count=kept;
    if(kept+RR_CANONICAL_COUNT>RR_CITY_MAX_BUILDINGS)return false;
    for(int i=0;i<RR_CANONICAL_COUNT;i++){
        const RRCanonicalBuilding *source=&rr_canonical_buildings[i];RRPlannedBuilding *b=&plan->buildings[plan->count++];
        *b=(RRPlannedBuilding){0};b->x=(float)source->center.x;b->z=(float)source->center.z;b->width=(float)(source->half.x*2);b->depth=(float)(source->half.z*2);b->top=(float)(source->center.y+source->half.y);
        b->canonical_index=i;b->red=13;b->green=11;b->blue=36;plan->canonical_count++;
    }
    return true;
}

bool rr_canonical_city_hero_placement(int index,RRVec3 model_size,RRHeroPlacement *p)
{
    const RRCanonicalHero *h=rr_canonical_city_hero(index);double width=fmax(model_size.x,model_size.z);
    if(!h||!p||!isfinite(width)||width<=0)return false;p->pos=h->pos;p->yaw=h->yaw;p->scale=h->width/width;return true;
}

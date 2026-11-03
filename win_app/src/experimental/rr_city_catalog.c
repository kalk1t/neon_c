#include "rr_city_catalog.h"
#include <string.h>
#include "generated_city_catalog.h"
int rr_catalog_count(void){return (int)(sizeof(catalog_entries)/sizeof(catalog_entries[0]));}
int rr_catalog_hero_count(void){return (int)(sizeof(catalog_heroes)/sizeof(catalog_heroes[0]));}
const RRCatalogEntry *rr_catalog_entry(int i){return i>=0&&i<rr_catalog_count()?&catalog_entries[i]:NULL;}
const RRCatalogHero *rr_catalog_hero(int i){return i>=0&&i<rr_catalog_hero_count()?&catalog_heroes[i]:NULL;}
size_t rr_catalog_metadata_bytes(void){return sizeof(catalog_entries)+sizeof(catalog_heroes);}
bool rr_catalog_visible(int index,const RRCatalogView *view)
{
    const RRCatalogEntry *e=rr_catalog_entry(index);if(!e||!view)return false;
    RRVec3 center=rr_scale(rr_add(e->minimum,e->maximum),.5);double radius=rr_length(rr_sub(e->maximum,center));
    for(int i=0;i<6;i++){const double *p=view->planes[i];if(p[0]*center.x+p[1]*center.y+p[2]*center.z+p[3]<-radius)return false;}
    return true;
}
bool rr_catalog_plan(const RRCatalogView *view,RRCatalogPlan *plan)
{
    if(!view||!plan)return false;memset(plan,0,sizeof(*plan));plan->visible_hash=2166136261u;
    for(int j=0;j<6;j++)for(int k=0;k<4;k++)if(!isfinite(view->planes[j][k]))return false;
    if(!isfinite(view->eye.x)||!isfinite(view->eye.y)||!isfinite(view->eye.z))return false;
    double distances[RR_CATALOG_RESIDENT_MAX];
    for(int i=0;i<rr_catalog_count();i++)if(rr_catalog_visible(i,view)){
        const RRCatalogEntry *e=rr_catalog_entry(i);plan->visible++;plan->visible_hash=(plan->visible_hash^(uint32_t)i)*16777619u;if(!e->geometry_available)plan->unavailable++;
        RRVec3 center=rr_scale(rr_add(e->minimum,e->maximum),.5),offset=rr_sub(center,view->eye);double distance=rr_dot(offset,offset);int slot=plan->count;
        if(slot==RR_CATALOG_RESIDENT_MAX){if(distance>=distances[slot-1])continue;slot--;}else plan->count++;
        while(slot>0&&distance<distances[slot-1]){distances[slot]=distances[slot-1];plan->selected[slot]=plan->selected[slot-1];slot--;}
        distances[slot]=distance;plan->selected[slot]=i;
    }
    for(int i=0;i<plan->count;i++)plan->vertices+=rr_catalog_entry(plan->selected[i])->vertices;
    while(plan->count&&plan->vertices>RR_CATALOG_VERTEX_BUDGET)plan->vertices-=rr_catalog_entry(plan->selected[--plan->count])->vertices;
    plan->budget_skipped=plan->visible-plan->count;plan->complete=!plan->unavailable&&!plan->budget_skipped;return true;
}
static bool contains(const int *ids,int count,int id){for(int i=0;i<count;i++)if(ids[i]==id)return true;return false;}
RRCatalogStep rr_catalog_page_step(RRCatalogCache *cache,const RRCatalogPlan *plan,int *uploads)
{
    if(uploads)*uploads=0;
    if(!cache||!plan||!plan->complete||plan->count<0||plan->count>RR_CATALOG_RESIDENT_MAX||plan->visible!=plan->count||plan->unavailable||plan->budget_skipped||cache->count<0||cache->count>RR_CATALOG_RESIDENT_MAX)return RR_CATALOG_BLOCKED;
    unsigned int vertices=0;
    for(int i=0;i<plan->count;i++){const RRCatalogEntry *e=rr_catalog_entry(plan->selected[i]);if(!e||!e->geometry_available||contains(plan->selected,i,plan->selected[i]))return RR_CATALOG_BLOCKED;vertices+=e->vertices;}
    if(vertices>RR_CATALOG_VERTEX_BUDGET||vertices!=plan->vertices)return RR_CATALOG_BLOCKED;
    for(int i=0;i<cache->count;i++)if(!rr_catalog_entry(cache->ids[i])||contains(cache->ids,i,cache->ids[i]))return RR_CATALOG_BLOCKED;
    int kept=0;cache->vertices=0;
    for(int i=0;i<cache->count;i++)if(contains(plan->selected,plan->count,cache->ids[i])){cache->ids[kept++]=cache->ids[i];cache->vertices+=rr_catalog_entry(cache->ids[i])->vertices;}
    cache->count=kept;int added=0;
    for(int i=0;i<plan->count&&added<RR_CATALOG_UPLOADS;i++)if(!contains(cache->ids,cache->count,plan->selected[i])){cache->ids[cache->count++]=plan->selected[i];cache->vertices+=rr_catalog_entry(plan->selected[i])->vertices;added++;}
    if(uploads)*uploads=added;return cache->count==plan->count?RR_CATALOG_READY:RR_CATALOG_PENDING;
}
bool rr_catalog_collide(RRVec3 *point,double radius,RRWorld *scratch,RRContact *contact)
{
    if(!point||!scratch||!contact||!isfinite(radius)||radius<0||!isfinite(point->x)||!isfinite(point->y)||!isfinite(point->z))return false;
    double cx=floor(point->x/120),cz=floor(point->z/120);scratch->count=0;
    for(int i=0;i<rr_catalog_count();i++){
        const RRShape *s=&catalog_entries[i].collider;
        if(cx>=floor(s->bounds[0]/120)&&cx<=floor(s->bounds[2]/120)&&cz>=floor(s->bounds[1]/120)&&cz<=floor(s->bounds[3]/120))if(!rr_world_add(scratch,*s))return false;
    }
    *contact=rr_world_collide(scratch,point,radius);return true;
}

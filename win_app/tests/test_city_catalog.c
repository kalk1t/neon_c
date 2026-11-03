#include "rr_city_catalog.h"
#include <stdio.h>
#include <string.h>
typedef struct CatalogCollisionFixture {RRVec3 point,result;bool hit;RRVec3 normal;double depth;} CatalogCollisionFixture;
typedef struct CatalogViewFixture {RRCatalogView view;int count;uint32_t hash;} CatalogViewFixture;
#include "generated_city_catalog_reference.h"
static RRWorld scratch;
static int failures;static double worst;
#define CHECK(x) do{if(!(x)){if(failures<12)printf("Failed line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&e<1e-8);}
static void vec(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
int main(void)
{
    CHECK(rr_catalog_count()==2741&&rr_catalog_hero_count()==450);int kinds[3]={0},known=0,available=0;unsigned int vertices=0;
    for(int i=0;i<rr_catalog_count();i++){
        const RRCatalogEntry *e=rr_catalog_entry(i);CHECK(e&&e->kind>=0&&e->kind<3);if(!e)continue;
        kinds[e->kind]++;vertices+=e->vertices;if(e->mesh_id>=0)known++;if(e->geometry_available)available++;
        CHECK(e->collider.kind==RR_SHAPE_BOX && e->vertices>0 && e->vertices%3==0);
        CHECK(isfinite(e->minimum.x)&&e->minimum.x<=e->maximum.x&&e->minimum.y<=e->maximum.y&&e->minimum.z<=e->maximum.z);
        if(e->geometry_available)CHECK(e->mesh_id>=0 && e->poster_vertices==0);
    }
    CHECK(kinds[0]==2360&&kinds[1]==380&&kinds[2]==1&&known==413&&available==316&&vertices==1695222);
    for(int i=0;i<rr_catalog_hero_count();i++){const RRCatalogHero *h=rr_catalog_hero(i);CHECK(h&&isfinite(h->position.x)&&h->width>0&&isfinite(h->yaw));}
    CHECK(!rr_catalog_entry(-1)&&!rr_catalog_entry(2741)&&!rr_catalog_hero(450));int max_candidates=0;
    for(size_t i=0;i<sizeof(catalog_collision_fixtures)/sizeof(catalog_collision_fixtures[0]);i++){
        const CatalogCollisionFixture *f=&catalog_collision_fixtures[i];RRVec3 p=f->point;RRContact hit;
        CHECK(rr_catalog_collide(&p,2.1,&scratch,&hit));CHECK(hit.hit==f->hit);vec(p,f->result);if(hit.hit){vec(hit.normal,f->normal);near(hit.depth,f->depth);}
        if(scratch.count>max_candidates)max_candidates=scratch.count;CHECK(scratch.count<=6);
    }
    RRVec3 point=rr_v3(3,4,5);RRContact contact;CHECK(!rr_catalog_collide(&point,NAN,&scratch,&contact));vec(point,rr_v3(3,4,5));
    int blocked=0,max_visible=0,max_skipped=0,max_unavailable=0;
    for(size_t i=0;i<sizeof(catalog_view_fixtures)/sizeof(catalog_view_fixtures[0]);i++){
        const CatalogViewFixture *f=&catalog_view_fixtures[i];RRCatalogPlan a,b;CHECK(rr_catalog_plan(&f->view,&a));CHECK(rr_catalog_plan(&f->view,&b));CHECK(memcmp(&a,&b,sizeof(a))==0);
        CHECK(a.visible==f->count&&a.visible_hash==f->hash);CHECK(a.count<=64&&a.vertices<=RR_CATALOG_VERTEX_BUDGET&&a.budget_skipped==a.visible-a.count);
        CHECK(a.complete==(!a.unavailable&&!a.budget_skipped));
        if(a.visible>max_visible)max_visible=a.visible;if(a.budget_skipped>max_skipped)max_skipped=a.budget_skipped;if(a.unavailable>max_unavailable)max_unavailable=a.unavailable;
        if(!a.complete){RRCatalogCache c={0},old=c;int uploads=-1;CHECK(rr_catalog_page_step(&c,&a,&uploads)==RR_CATALOG_BLOCKED&&uploads==0&&memcmp(&c,&old,sizeof(c))==0);blocked++;}
    }
    RRCatalogPlan ready={0};ready.complete=true;
    for(int i=0;i<rr_catalog_count()&&ready.count<16;i++)if(rr_catalog_entry(i)->geometry_available){ready.selected[ready.count++]=i;ready.vertices+=rr_catalog_entry(i)->vertices;}
    ready.visible=ready.count;CHECK(ready.count==16);RRCatalogCache cache={0};int steps=0;
    for(;steps<10;steps++){int uploads=-1;RRCatalogStep result=rr_catalog_page_step(&cache,&ready,&uploads);CHECK(result!=RR_CATALOG_BLOCKED&&uploads>=0&&uploads<=2&&cache.count<=64&&cache.vertices<=RR_CATALOG_VERTEX_BUDGET);if(result==RR_CATALOG_READY)break;}
    CHECK(steps==7&&cache.count==16);RRCatalogCache previous=cache;RRCatalogPlan invalid=ready;invalid.vertices++;
    CHECK(rr_catalog_page_step(&cache,&invalid,NULL)==RR_CATALOG_BLOCKED&&memcmp(&cache,&previous,sizeof(cache))==0);
    invalid=ready;invalid.selected[0]=invalid.selected[1];CHECK(rr_catalog_page_step(&cache,&invalid,NULL)==RR_CATALOG_BLOCKED&&memcmp(&cache,&previous,sizeof(cache))==0);
    ready.count=ready.visible=4;ready.vertices=0;for(int i=0;i<4;i++)ready.vertices+=rr_catalog_entry(ready.selected[i])->vertices;
    int uploads=-1;CHECK(rr_catalog_page_step(&cache,&ready,&uploads)==RR_CATALOG_READY&&uploads==0&&cache.count==4&&cache.vertices==ready.vertices);
    RRCatalogView invalid_view=catalog_view_fixtures[0].view;invalid_view.planes[0][0]=NAN;CHECK(!rr_catalog_plan(&invalid_view,&invalid));
    CHECK(blocked>0);
    printf("HEADLESS catalog only:2741 bodies/450 crowns;2360grid+380ring+1skyway;413 existing meshes,316 complete geometry/97 missing posters;5482 browser collision queries,max city candidates=%d,error=%.3g;108 Three sphere-view sets,max visible=%d,blocked=%d,max budget skipped=%d,max unavailable=%d; deterministic planning,64 meshes/141312 vertices/2 uploads,transactional rejection; metadata=%zu bytes; %d failures.\n",max_candidates,worst,max_visible,blocked,max_skipped,max_unavailable,rr_catalog_metadata_bytes(),failures);
    return failures?1:0;
}

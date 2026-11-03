#ifndef RR_CITY_CATALOG_H
#define RR_CITY_CATALOG_H
#include "rr_physics.h"
#include <stddef.h>
#include <stdint.h>
/* Experimental HEADLESS model only; never linked into RoadstersRace. */
#define RR_CATALOG_RESIDENT_MAX 64
#define RR_CATALOG_VERTEX_BUDGET (64u*2208u)
#define RR_CATALOG_UPLOADS 2
typedef struct RRCatalogEntry {RRVec3 minimum,maximum;RRShape collider;int kind;unsigned int vertices;int mesh_id;bool geometry_available;unsigned int poster_vertices;} RRCatalogEntry;
typedef struct RRCatalogHero {RRVec3 position;double width,yaw;} RRCatalogHero;
typedef struct RRCatalogView {RRVec3 eye;double planes[6][4];} RRCatalogView;
typedef struct RRCatalogPlan {int selected[RR_CATALOG_RESIDENT_MAX];int count,visible,unavailable,budget_skipped;unsigned int vertices;uint32_t visible_hash;bool complete;} RRCatalogPlan;
typedef struct RRCatalogCache {int ids[RR_CATALOG_RESIDENT_MAX];int count;unsigned int vertices;} RRCatalogCache;
typedef enum RRCatalogStep {RR_CATALOG_BLOCKED,RR_CATALOG_PENDING,RR_CATALOG_READY} RRCatalogStep;
int rr_catalog_count(void);
int rr_catalog_hero_count(void);
const RRCatalogEntry *rr_catalog_entry(int index);
const RRCatalogHero *rr_catalog_hero(int index);
size_t rr_catalog_metadata_bytes(void);
bool rr_catalog_visible(int index,const RRCatalogView *view);
bool rr_catalog_plan(const RRCatalogView *view,RRCatalogPlan *plan);
RRCatalogStep rr_catalog_page_step(RRCatalogCache *cache,const RRCatalogPlan *plan,int *uploads);
bool rr_catalog_collide(RRVec3 *point,double radius,RRWorld *scratch,RRContact *contact);
#endif

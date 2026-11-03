#ifndef RR_CANONICAL_CITY_H
#define RR_CANONICAL_CITY_H
#include "rr_city_plan.h"
#include "rr_hero_asset.h"
#include "rr_physics.h"
typedef struct RRCanonicalBuilding {RRVec3 center,half;double yaw,cosine,sine;RRHeroBounds mesh_bounds;int gx,gz,hero;unsigned int vertices;int mesh_id;} RRCanonicalBuilding;
typedef struct RRCanonicalHero {RRVec3 pos;double width,yaw;} RRCanonicalHero;
bool rr_canonical_city_plan(RRCityPlan *plan,const RRCorridor *corridor);
bool rr_canonical_city_contains(double x,double z);
int rr_canonical_city_count(void);
int rr_canonical_hero_count(void);
const RRCanonicalBuilding *rr_canonical_city_building(int index);
const RRCanonicalHero *rr_canonical_city_hero(int index);
bool rr_canonical_city_hero_placement(int index,RRVec3 model_size,RRHeroPlacement *placement);
bool rr_canonical_city_collider(int index,RRShape *shape);
#endif

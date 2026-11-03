#ifndef RR_WORLD_PLAN_H
#define RR_WORLD_PLAN_H
#include "rr_physics.h"
#include "rr_city_plan.h"
/* Native prototype city bodies + matching browser start/ring structures only.
 * Decorative beacon/barrier/sign/platform/traffic layouts remain unported. */
bool rr_world_plan(RRWorld *world,const RRCityPlan *city,const RRTrack *track,bool roam);
#endif

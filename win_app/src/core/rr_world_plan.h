#ifndef RR_WORLD_PLAN_H
#define RR_WORLD_PLAN_H
#include "rr_physics.h"
#include "rr_city_plan.h"
#include "rr_course_plan.h"
#include "rr_shared_scenery.h"
/* Native prototype city bodies + matching browser start/ring structures only.
 * Road furniture includes guidance/beacons/barriers/signs and ring supports.
 * Shared platforms/trees are provided through rr_world_plan_shared; traffic
 * and full city layout remain unported. */
bool rr_world_plan_shared(RRWorld *world,const RRCityPlan *city,const RRCourse *course,const RRSharedScenery *shared,bool roam);
bool rr_world_plan_course(RRWorld *world,const RRCityPlan *city,const RRCourse *course,bool roam);
bool rr_world_plan(RRWorld *world,const RRCityPlan *city,const RRTrack *track,bool roam);
#endif

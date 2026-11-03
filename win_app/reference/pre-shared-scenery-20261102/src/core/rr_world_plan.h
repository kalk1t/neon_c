#ifndef RR_WORLD_PLAN_H
#define RR_WORLD_PLAN_H
#include "rr_physics.h"
#include "rr_city_plan.h"
#include "rr_course_plan.h"
/* Native prototype city bodies + matching browser start/ring structures only.
 * Road furniture includes guidance/beacons/barriers/signs and ring supports.
 * Shared service platforms/trees, traffic and full city layout remain unported. */
bool rr_world_plan_course(RRWorld *world,const RRCityPlan *city,const RRCourse *course,bool roam);
bool rr_world_plan(RRWorld *world,const RRCityPlan *city,const RRTrack *track,bool roam);
#endif

#ifndef RR_CITY_PLAN_H
#define RR_CITY_PLAN_H
#include <stdint.h>
#include "rr_corridor.h"
enum { RR_CITY_MAX_BUILDINGS=700 };
typedef double (*RRCityRandom)(void *context);
typedef struct RRCityFit { bool accepted;double top;int steps; } RRCityFit;
/* Browser city.js candidate footprint/start/checkpoint/height-fitting block.
 * random is called only when initial corridor clearance fails.
 */
RRCityFit rr_city_fit(const RRCorridor *corridor,double x,double z,double width,double depth,double top,RRCityRandom random,void *context);
typedef struct RRPlannedBuilding {float x,z,width,depth,top;int pattern,canonical_index;uint8_t red,green,blue;} RRPlannedBuilding;
typedef struct RRCityPlan {RRPlannedBuilding buildings[RR_CITY_MAX_BUILDINGS];int count,lowered,rejected,prototype_kept,prototype_removed,canonical_count;} RRCityPlan;
/* Retains the prototype's grid/seed/dimensions/styles, applying browser union
 * clearance and fitting. This is not the complete browser city generator.
 */
void rr_city_plan(RRCityPlan *plan,const RRCorridor *corridor);
#endif

#ifndef RR_ROADS_H
#define RR_ROADS_H
#include "rr_math.h"
#define RR_ROAD_COUNT 9
typedef struct RRRoadDefinition {
    int id;
    const char *name;
    double ai_speed,fire_interval;
    const RRVec3 *control;
    int control_count;
    const double *checkpoints;
    int checkpoint_count,rings;
} RRRoadDefinition;
/* Matches browser roadById: invalid integer IDs fall back to road 1. */
const RRRoadDefinition *rr_road_by_id(int id);
#endif

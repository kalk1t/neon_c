#ifndef RR_CITY_BATCHING_H
#define RR_CITY_BATCHING_H
#include "rr_math.h"
#include <stdbool.h>
/* Test-only exact bound math; not a runtime renderer or changed resource policy. */
typedef struct RRBatchVolume {RRVec3 minimum,maximum,center;double radius;unsigned int vertices;int material;} RRBatchVolume;
typedef struct RRBatchScheme {const RRBatchVolume *items;int count;} RRBatchScheme;
bool rr_batch_sphere_visible(const RRBatchVolume *volume,const double planes[6][4]);
bool rr_batch_box_visible(const RRBatchVolume *volume,const double planes[6][4]);
#endif

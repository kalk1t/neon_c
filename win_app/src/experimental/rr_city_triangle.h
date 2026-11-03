#ifndef RR_CITY_TRIANGLE_H
#define RR_CITY_TRIANGLE_H
#include "rr_math.h"
#include <stdbool.h>
bool rr_triangle_front(RRVec3 a,RRVec3 b,RRVec3 c,RRVec3 eye);
bool rr_triangle_frustum(RRVec3 a,RRVec3 b,RRVec3 c,const double planes[6][4],double margin);
#endif

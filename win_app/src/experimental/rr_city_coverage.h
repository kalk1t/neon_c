#ifndef RR_CITY_COVERAGE_H
#define RR_CITY_COVERAGE_H
#include "rr_city_occlusion.h"
/* Test-only continuous CPU geometry proof, not GPU/pixel certification. */
bool rr_coverage_union(const RROcclusionPoint *p,int count,const RROcclusionPoint (*blockers)[3],int number,unsigned int *peak);
#endif

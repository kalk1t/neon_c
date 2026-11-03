#ifndef RR_CITY_OCCLUSION_H
#define RR_CITY_OCCLUSION_H
#include "rr_math.h"
#include <stdbool.h>
#define RR_OCCLUSION_W 128
#define RR_OCCLUSION_H 96
typedef struct RROcclusionPoint {double x,y,depth;} RROcclusionPoint;
typedef struct RROcclusion {double depth[RR_OCCLUSION_W*RR_OCCLUSION_H];unsigned int covered;} RROcclusion;
void rr_occlusion_clear(RROcclusion *buffer);
int rr_occlusion_clip(RRVec3 a,RRVec3 b,RRVec3 c,const double planes[6][4],double margin,RRVec3 polygon[16]);
bool rr_occlusion_project(RRVec3 p,const double m[16],RROcclusionPoint *out);
bool rr_occlusion_tile_depth(const RROcclusionPoint triangle[3],double x0,double y0,double x1,double y1,double *maximum);
void rr_occlusion_raster(RROcclusion *buffer,const RROcclusionPoint triangle[3]);
bool rr_occlusion_hidden(const RROcclusion *buffer,const RROcclusionPoint *polygon,int count);
#endif

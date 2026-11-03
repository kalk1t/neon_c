#ifndef RR_CITY_WITNESS_H
#define RR_CITY_WITNESS_H
#include "rr_city_repacking.h"
typedef struct RRWitnessNode {RRVec3 minimum,maximum;int left,right,start,count;} RRWitnessNode;
typedef struct RRWitnessScene {const unsigned char *geometry;unsigned int *parts;RRWitnessNode *nodes;int count,capacity;} RRWitnessScene;
typedef struct RRWitnessView {const double *mvp;uint32_t lo,hi;} RRWitnessView;
typedef struct RRWitnessHit {bool hit,ambiguous;double distance;unsigned int triangle;} RRWitnessHit;
bool rr_witness_scene_init(RRWitnessScene *scene,const unsigned char *geometry);
void rr_witness_scene_free(RRWitnessScene *scene);
bool rr_witness_triangle(RRVec3 origin,RRVec3 direction,RRVec3 a,RRVec3 b,RRVec3 c,bool double_sided,double *distance,bool *ambiguous);
RRWitnessHit rr_witness_cast(const RRWitnessScene *scene,const RRWitnessView *view,RRVec3 origin,RRVec3 direction,double maximum,unsigned int exclude,bool brute_force);
#endif

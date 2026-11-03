#ifndef RR_SHARED_SCENERY_H
#define RR_SHARED_SCENERY_H
#include "rr_corridor.h"
#include "rr_physics.h"
enum {RR_SHARED_PLATFORM,RR_SHARED_COLUMN,RR_SHARED_TREE_M,RR_SHARED_TREE_C};
typedef struct RRSharedProp {RRVec3 pos;double yaw,scale;int kind;} RRSharedProp;
typedef struct RRSharedScenery {RRSharedProp props[64];RRShape colliders[128];int prop_count,collider_count,platforms,columns,trees;} RRSharedScenery;
bool rr_shared_scenery_plan(RRSharedScenery *scenery,const RRTrack *skyline,const RRCorridor *corridor);
#endif

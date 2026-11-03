#ifndef RR_HERO_ASSET_H
#define RR_HERO_ASSET_H
#include "rr_math.h"
#include <stdbool.h>
typedef struct RRHeroBounds {RRVec3 minimum,maximum;} RRHeroBounds;
typedef struct RRHeroPlacement {RRVec3 pos;double scale,yaw;} RRHeroPlacement;
static inline bool rr_control_tower_placement(RRHeroBounds old,RRVec3 model_size,RRHeroPlacement *out)
{
    double height=old.maximum.y-old.minimum.y;
    if(!out||!isfinite(height)||height<=0||!isfinite(model_size.y)||model_size.y<=0||!isfinite(old.minimum.x)||!isfinite(old.maximum.x)||!isfinite(old.minimum.z)||!isfinite(old.maximum.z))return false;
    out->pos=rr_v3((old.minimum.x+old.maximum.x)/2,old.minimum.y,(old.minimum.z+old.maximum.z)/2);
    out->scale=height*1.05/model_size.y;out->yaw=RR_PI/2;return true;
}
static inline RRHeroBounds rr_hero_world_bounds(RRHeroBounds model,RRHeroPlacement p)
{
    RRHeroBounds result={rr_v3(HUGE_VAL,HUGE_VAL,HUGE_VAL),rr_v3(-HUGE_VAL,-HUGE_VAL,-HUGE_VAL)};
    for(int i=0;i<8;i++){
        double x=(i&1?model.maximum.x:model.minimum.x)*p.scale,y=(i&2?model.maximum.y:model.minimum.y)*p.scale,z=(i&4?model.maximum.z:model.minimum.z)*p.scale;
        RRVec3 point=rr_v3(p.pos.x+cos(p.yaw)*x+sin(p.yaw)*z,p.pos.y+y,p.pos.z-sin(p.yaw)*x+cos(p.yaw)*z);
        result.minimum=rr_v3(fmin(result.minimum.x,point.x),fmin(result.minimum.y,point.y),fmin(result.minimum.z,point.z));
        result.maximum=rr_v3(fmax(result.maximum.x,point.x),fmax(result.maximum.y,point.y),fmax(result.maximum.z,point.z));
    }
    return result;
}
#endif

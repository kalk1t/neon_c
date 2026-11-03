#ifndef RR_MINIMAP_H
#define RR_MINIMAP_H
#include "rr_track.h"
typedef struct RRMinimap {double center_x,center_z,span;} RRMinimap;
static inline void rr_minimap_init(RRMinimap *map,const RRTrack *track)
{
    double min_x=HUGE_VAL,min_z=HUGE_VAL,max_x=-HUGE_VAL,max_z=-HUGE_VAL;
    for(int i=0;i<track->sample_count;i++){
        RRVec3 p=track->samples[i];min_x=fmin(min_x,p.x);max_x=fmax(max_x,p.x);min_z=fmin(min_z,p.z);max_z=fmax(max_z,p.z);
    }
    map->center_x=(min_x+max_x)/2;map->center_z=(min_z+max_z)/2;
    map->span=fmax(max_x-min_x,max_z-min_z)*1.22;
}
/* Browser HUD.setTrack projection, normalized from its 440px canvas. */
static inline void rr_minimap_point(const RRMinimap *map,double x,double z,double *px,double *py)
{
    *px=.5+((x-map->center_x)/map->span)*.82;
    *py=.5+((z-map->center_z)/map->span)*.82;
}
#endif

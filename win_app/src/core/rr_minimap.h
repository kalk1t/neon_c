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
/* Actual HUD.drawMap player path in its440px Canvas, transformed to viewport.
 * Points are tip/right/notch/left; native fill uses two reversed-winding triangles. */
static inline void rr_minimap_player_points(const RRMinimap *map,double x,double z,double yaw,
    double vx,double vy,double width,double height,double points[4][2])
{
    double px,py;rr_minimap_point(map,x,z,&px,&py);double c=cos(-yaw),s=sin(-yaw);
    const double local[4][2]={{0,-16},{11,12},{0,6},{-11,12}};
    for(int i=0;i<4;i++){
        points[i][0]=vx+px*width+(local[i][0]*c-local[i][1]*s)*width/440;
        points[i][1]=vy+py*height+(local[i][0]*s+local[i][1]*c)*height/440;
    }
}
#endif

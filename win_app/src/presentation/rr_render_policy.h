#ifndef RR_RENDER_POLICY_H
#define RR_RENDER_POLICY_H

#include <stdbool.h>
#include <stdint.h>
#include "rr_math.h"

enum { RR_RENDER_WIDTH=960, RR_RENDER_HEIGHT=600, RR_BUILDING_BUDGET=64 };
typedef struct RRRenderSize { int width,height; } RRRenderSize;

/* Bound render work independently of desktop size, preserving aspect ratio. */
static inline RRRenderSize rr_render_size(int width,int height)
{
    RRRenderSize size={0,0};
    if(width<=0 || height<=0)return size;
    if(width<=RR_RENDER_WIDTH && height<=RR_RENDER_HEIGHT){size.width=width;size.height=height;return size;}
    if((int64_t)width*RR_RENDER_HEIGHT>(int64_t)height*RR_RENDER_WIDTH){
        size.width=RR_RENDER_WIDTH;size.height=(int)((int64_t)height*RR_RENDER_WIDTH/width);
    }else{
        size.height=RR_RENDER_HEIGHT;size.width=(int)((int64_t)width*RR_RENDER_HEIGHT/height);
    }
    if(size.width<1)size.width=1;
    if(size.height<1)size.height=1;
    return size;
}

/* A pause is recoverable; this cannot interrupt a blocking driver call. */
static inline bool rr_render_overloaded(double seconds,int *slow_frames)
{
    *slow_frames=seconds>.15?*slow_frames+1:0;
    return *slow_frames>=10;
}

/* Camera basis must be orthonormal. Plane margins keep intersecting objects. */
static inline bool rr_render_sphere_visible(RRVec3 center,double radius,RRVec3 eye,
    RRVec3 forward,RRVec3 right,RRVec3 up,double tan_half_fov,double aspect,double far_plane)
{
    RRVec3 delta=rr_sub(center,eye);
    double z=rr_dot(delta,forward),x=rr_dot(delta,right),y=rr_dot(delta,up);
    double tx=tan_half_fov*aspect,ty=tan_half_fov;
    if(z+radius<.1 || z-radius>far_plane)return false;
    if(fabs(x)>z*tx+radius*sqrt(1+tx*tx))return false;
    if(fabs(y)>z*ty+radius*sqrt(1+ty*ty))return false;
    return true;
}
#endif

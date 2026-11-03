#ifndef RR_TARGET_GUIDANCE_H
#define RR_TARGET_GUIDANCE_H
#include "rr_math.h"
#include <stdbool.h>
typedef struct RRTargetGuidance {bool on;double x,y,angle,distance;} RRTargetGuidance;
/* HUD.update consumes projected NDC; x/y returned in physical screen pixels.
 * CSS zoom affects the source arrow offsets, not its screen-space radius. */
static inline RRTargetGuidance rr_target_guidance(double nx,double ny,double nz,double w,double h,double zoom,double distance)
{
    RRTargetGuidance t={0};bool behind=nz>1;
    t.on=!behind&&fabs(nx)<.92&&fabs(ny)<.9;
    t.distance=floor(distance+.5);
    if(t.on){t.x=(nx*.5+.5)*w;t.y=(-ny*.5+.5)*h;}
    else{if(behind){nx=-nx;ny=-ny;}double a=atan2(ny,nx),r=fmin(w,h)*.38;
        t.x=w*.5+cos(a)*r-20*zoom;t.y=h*.5-sin(a)*r-25*zoom;t.angle=-a;}
    return t;
}
#endif

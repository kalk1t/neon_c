#ifndef RR_NITRO_CALL_H
#define RR_NITRO_CALL_H
#include <stdbool.h>
#include <math.h>
typedef struct RRNitroCall {double started;bool active;} RRNitroCall;
typedef struct RRNitroStyle {bool shown;double opacity,scale,spacing,translate_y;} RRNitroStyle;
static inline void rr_nitro_call_start(RRNitroCall *call,double now){call->started=now;call->active=true;}
/* CSS ease-out cubic-bezier(0,0,.58,1), per property/keyframe interval. */
static inline double rr_nitro_ease(double x)
{
    if(x<=0)return 0;if(x>=1)return 1;
    double lo=0,hi=1,t=0;for(int i=0;i<36;i++){t=(lo+hi)*.5;double at=1.74*t*t-.74*t*t*t;if(at<x)lo=t;else hi=t;}
    t=(lo+hi)*.5;return 3*t*t-2*t*t*t;
}
/* CSS timeline follows wall time, including pause/menu and scheduler gaps.
 * Animation has no fill, so after750ms underlying opacity0 applies. */
static inline RRNitroStyle rr_nitro_call_sample(const RRNitroCall *call,double now)
{
    RRNitroStyle s={false,0,1,14,-50};double t=(now-call->started)/.75;
    if(!call->active||!isfinite(t)||t<0||t>=1)return s;s.shown=true;
    if(t<.18){double u=rr_nitro_ease(t/.18);s.opacity=u;s.scale=1.6-.6*u;s.spacing=30-16*u;}
    else{double u=rr_nitro_ease((t-.18)/.82);s.scale=1-.05*u;s.translate_y=-50-10*u;
        s.opacity=t<.7?1-.1*rr_nitro_ease((t-.18)/.52):.9*(1-rr_nitro_ease((t-.7)/.3));}
    return s;
}
#endif

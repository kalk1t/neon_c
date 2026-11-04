#ifndef RR_BOOST_METER_H
#define RR_BOOST_METER_H
#include "rr_math.h"
#include <stdbool.h>
enum {RR_BOOST_STATIC,RR_BOOST_BURNING,RR_BOOST_LOW};
typedef struct RRBoostMeter {int animation;bool seen,on,low,rendered;double started,fill;} RRBoostMeter;
typedef struct RRBoostStyle {double opacity,brightness;} RRBoostStyle;
/* Animation identity, not the boosting class alone, controls CSS restart. */
static inline void rr_boost_meter_observe(RRBoostMeter *m,double boost,bool on,double now)
{
    bool low=boost<.2;int animation=low?RR_BOOST_LOW:on?RR_BOOST_BURNING:RR_BOOST_STATIC;
    if(!m->seen||animation!=m->animation)m->started=now;
    m->seen=true;m->animation=animation;m->on=on;m->low=low;m->fill=rr_clamp(boost,0,1);
}
/* CSS default ease cubic-bezier(.25,.1,.25,1). */
static inline double rr_boost_ease(double x)
{
    if(x<=0)return 0;if(x>=1)return 1;
    double lo=0,hi=1,t=0;for(int i=0;i<36;i++){t=(lo+hi)*.5;double at=.75*t-.75*t*t+t*t*t;if(at<x)lo=t;else hi=t;}
    t=(lo+hi)*.5;return .3*t+2.4*t*t-1.7*t*t*t;
}
static inline RRBoostStyle rr_boost_meter_style(const RRBoostMeter *m,double now)
{
    RRBoostStyle s={1,1};if(!m->seen||m->animation==RR_BOOST_STATIC)return s;
    double duration=m->animation==RR_BOOST_LOW?.4:.22,elapsed=now-m->started;
    if(!isfinite(elapsed)||elapsed<0)elapsed=0;
    double progress=fmod(elapsed/duration,2);if(progress>1)progress=2-progress;
    double u=rr_boost_ease(progress);
    if(m->animation==RR_BOOST_LOW)s.opacity=1-.75*u;else s.brightness=1+.8*u;
    return s;
}
/* display:none cancels descendant CSS animations. Re-render restarts the
 * selected animation even when its on/low class identity did not change. */
static inline void rr_boost_meter_render(RRBoostMeter *m,bool rendered,double now)
{if(rendered&&!m->rendered)m->started=now;m->rendered=rendered;}
static inline RRBoostStyle rr_boost_meter_present_style(const RRBoostMeter *m,double now)
{return m->rendered?rr_boost_meter_style(m,now):(RRBoostStyle){1,1};}
/* Source CSS border-box182x8, inner180x6, parent skewX(-25deg).
 * Output transformed child bounds relative to the enclosing speed panel. */
static inline void rr_boost_fill_bounds(double fill,double out[4])
{
    double skew=tan(25*RR_PI/180);out[0]=25-3*skew;out[1]=132;out[2]=180*fill+6*skew;out[3]=6;
}
#endif

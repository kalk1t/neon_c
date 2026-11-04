#ifndef RR_SPIN_INDICATOR_H
#define RR_SPIN_INDICATOR_H
#include "rr_flight.h"
#include <stdint.h>
#define RR_SPIN_INDICATOR_SEGMENTS 64
typedef struct RRSpinPoint {double x,y;} RRSpinPoint;
typedef struct RRSpinIndicator {double fill,opacity;bool cool;} RRSpinIndicator;
static inline RRSpinIndicator rr_spin_indicator(double cooldown)
{return (RRSpinIndicator){rr_clamp(1-cooldown/RR_SPIN_COOLDOWN,0,1),cooldown>0?.35:1,cooldown>0};}
static inline uint32_t rr_spin_indicator_color(RRSpinIndicator style,int layer)
{
    if(layer==0)return UINT32_C(0xff2bd640);
    if(layer==1)return UINT32_C(0x19e6ffff);
    if(layer==2)return UINT32_C(0x0a042000)|(uint32_t)floor(style.opacity*255+.5);
    if(layer==3)return UINT32_C(0xc4fbff00)|(uint32_t)floor(style.opacity*255+.5);
    return 0;
}
static inline int rr_spin_indicator_sectors(RRSpinIndicator style,int layer)
{if(layer==0||layer==2)return RR_SPIN_INDICATOR_SEGMENTS;if(layer==1&&style.fill>0)return (int)ceil(style.fill*RR_SPIN_INDICATOR_SEGMENTS);return 0;}
/* On the opaque UI backbuffer, adding opacity*(text-background)*glyph-alpha
 * replaces covered background inside the CSS opacity group. All channel deltas
 * are positive for these source colors. This avoids applying child opacity
 * twice to text over its already composited dark background. */
static inline uint32_t rr_spin_indicator_text_delta(RRSpinIndicator style,bool shadow)
{return (shadow?UINT32_C(0x0fe2df00):UINT32_C(0xbaf7df00))|(uint32_t)floor((shadow?.12:1)*style.opacity*255+.5);}
/* Background disk, clockwise cyan progress, then child dark disk. Drawing the
 * inner disk at child opacity exposes the conic sweep during cooldown, as CSS.
 * Inscribed64-gon arcs; screen-space front-face winding, no texture/heap. */
static inline bool rr_spin_indicator_triangle(RRSpinIndicator style,int layer,int index,RRSpinPoint out[3])
{
    int count=rr_spin_indicator_sectors(style,layer);if(!out||index<0||index>=count)return false;
    double span=layer==1?style.fill:1,radius=layer==2?31:37;
    double a=-RR_PI*.5+2*RR_PI*span*index/count,b=-RR_PI*.5+2*RR_PI*span*(index+1)/count;
    out[0]=(RRSpinPoint){0,0};out[1]=(RRSpinPoint){cos(b)*radius,sin(b)*radius};out[2]=(RRSpinPoint){cos(a)*radius,sin(a)*radius};return true;
}
#endif

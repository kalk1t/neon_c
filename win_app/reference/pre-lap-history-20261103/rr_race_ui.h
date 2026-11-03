#ifndef RR_RACE_UI_H
#define RR_RACE_UI_H
#include "rr_ui_layout.h"
#include "rr_session.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
typedef struct RRUIHudLayout { float scale; RRUIRect timer,map,speed,spin,history; } RRUIHudLayout;
typedef struct RRUIOverlayLayout { RRUIRect panel,actions[3],left,right; } RRUIOverlayLayout;
static inline RRUIHudLayout rr_ui_hud_layout(float w,float h)
{
    float z=(w<=1180||h<=720)?.8f:1.f;RRUIHudLayout l={0};l.scale=z;
    l.timer=(RRUIRect){w*.5f-150*z,22*z,300*z,142*z};
    l.map=(RRUIRect){w-262*z,22*z,240*z,240*z};
    l.speed=(RRUIRect){26*z,h-26*z-210*z,230*z,210*z};
    l.spin=(RRUIRect){w-26*z-142*z,h-26*z-144*z,142*z,144*z};
    l.history=(RRUIRect){22*z,108*z,210*z,146*z};return l;
}
static inline RRUIOverlayLayout rr_ui_pause_layout(float w,float h)
{
    RRUIOverlayLayout l={0};l.panel=(RRUIRect){(w-380)*.5f,(h-390)*.5f,380,390};
    for(int i=0;i<3;i++)l.actions[i]=(RRUIRect){l.panel.x+30,l.panel.y+108+(float)i*66,320,52};return l;
}
static inline RRUIOverlayLayout rr_ui_results_layout(float w,float h)
{
    RRUIOverlayLayout l={0};l.panel=(RRUIRect){(w-680)*.5f,(h-480)*.5f,680,480};
    l.left=(RRUIRect){l.panel.x+30,l.panel.y+172,296,118};l.right=(RRUIRect){l.panel.x+354,l.panel.y+172,296,118};
    for(int i=0;i<2;i++)l.actions[i]=(RRUIRect){l.panel.x+30,l.panel.y+326+(float)i*66,620,52};return l;
}
/* Exact positive binary-double rounding used by HUD fmt's seconds.toFixed(2),
 * including 59.995 -> 59.99 and its source-defined 00:60.00 minute-edge form.
 * Integer significand arithmetic avoids printf's ties-to-even and *100 rounding.
 */
static inline void rr_ui_time(char out[32],double t)
{
    if(!isfinite(t)||t<0||t>1e12){snprintf(out,32,"--:--.--");return;}
    uint64_t mins=(uint64_t)floor(t/60);double seconds=t-(double)mins*60;
    uint64_t bits=0;memcpy(&bits,&seconds,sizeof(bits));int exponent=(int)((bits>>52)&2047);
    uint64_t mantissa=(bits&UINT64_C(0xfffffffffffff))|(exponent?UINT64_C(1)<<52:0);
    uint64_t numerator=mantissa*100,cents=0;int shift=1075-exponent;
    if(shift>0&&shift<64)cents=(numerator+(UINT64_C(1)<<(shift-1)))>>shift;
    snprintf(out,32,"%02llu:%02llu.%02llu",(unsigned long long)mins,(unsigned long long)(cents/100),(unsigned long long)(cents%100));
}
static inline int rr_ui_speed(double speed){return isfinite(speed)?(int)fmin(99999,floor(fabs(speed)*3.6+.5)):0;}
static inline float rr_ui_spin_fill(double cd){return (float)rr_clamp(1-cd/RR_SPIN_COOLDOWN,0,1);}
/* Browser indexOf(min): retain the first tied best, updated only on a lap event. */
static inline size_t rr_ui_best_lap_on_append(const double *laps,size_t count,size_t previous)
{
    if(!count||!laps)return 0;
    if(count==1||previous>=count-1)return 0;
    return laps[count-1]<laps[previous]?count-1:previous;
}
/* Clip segments to circular CSS-style minimap without allocating a render target. */
static inline bool rr_ui_circle_segment(double *x1,double *y1,double *x2,double *y2,double cx,double cy,double r)
{
    double dx=*x2-*x1,dy=*y2-*y1,fx=*x1-cx,fy=*y1-cy;
    double a=dx*dx+dy*dy,b=2*(fx*dx+fy*dy),c=fx*fx+fy*fy-r*r;
    if(a<1e-18)return c<=0;
    double disc=b*b-4*a*c;if(disc<0)return false;
    double root=sqrt(disc),lo=fmax(0,(-b-root)/(2*a)),hi=fmin(1,(-b+root)/(2*a));if(lo>hi)return false;
    double ox=*x1,oy=*y1;*x1=ox+dx*lo;*y1=oy+dy*lo;*x2=ox+dx*hi;*y2=oy+dy*hi;return true;
}
#endif

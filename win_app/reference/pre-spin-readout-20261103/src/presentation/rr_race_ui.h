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
    l.timer=(RRUIRect){w*.5f-150*z,(z<1?17.59375f:22.f),300*z,z<1?121.5625f:153.f};
    l.map=(RRUIRect){w-262*z,22*z,240*z,240*z};
    l.speed=(RRUIRect){(z<1?20.796875f:26.f),h-(z<1?20.796875f:26.f)-(z<1?160.546875f:201.f),230*z,z<1?160.546875f:201.f};
    l.spin=(RRUIRect){w-26*z-142*z,h-26*z-144*z,142*z,144*z};
    l.history=(RRUIRect){22*z,140*z,210*z,146*z};return l;
}
typedef struct RRUIHudBoxes {RRUIRect lap_row,time,checkpoint_row,speed_label,speed_value,unit;} RRUIHudBoxes;
static inline RRUIHudBoxes rr_ui_hud_boxes(RRUIHudLayout l)
{
    float z=l.scale;RRUIHudBoxes b={0};
    b.lap_row=(RRUIRect){l.timer.x+24*z,l.timer.y+14*z,252*z,42*z};
    b.time=(RRUIRect){l.timer.x+24*z,l.timer.y+60*z,252*z,50*z};
    b.checkpoint_row=(RRUIRect){l.timer.x+24*z,l.timer.y+114*z,252*z,25*z};
    b.speed_label=(RRUIRect){l.speed.x+24*z,l.speed.y+14*z,182*z,19*z};
    b.speed_value=(RRUIRect){l.speed.x+24*z,l.speed.y+37*z,182*z,64*z};
    b.unit=(RRUIRect){l.speed.x+24*z,l.speed.y+101*z,182*z,22*z};
    /* Verified DPR1 Chromium compact CSS zoom rounds borders/normal line boxes;
     * it is not a uniform transform of desktop font line heights. */
    if(z<1){
        b.lap_row=(RRUIRect){l.timer.x+18.59375f,l.timer.y+10.59375f,202.8125f,34};
        b.time=(RRUIRect){l.timer.x+18.59375f,l.timer.y+47.78125f,202.8125f,40};
        b.checkpoint_row=(RRUIRect){l.timer.x+18.59375f,l.timer.y+90.96875f,202.8125f,20};
        b.speed_label=(RRUIRect){l.speed.x+18.59375f,l.speed.y+10.59375f,146.8125f,15};
        b.speed_value=(RRUIRect){l.speed.x+18.59375f,l.speed.y+28.78125f,146.8125f,51.203125f};
        b.unit=(RRUIRect){l.speed.x+18.59375f,l.speed.y+79.984375f,146.8125f,18};
    }return b;
}
/* Source later #laps-panel overrides top108 with140. The more-specific
 * #laps-panel .best also applies font600 13px to the best LI, not just footer.
 * Normal LI is700 16px: supplied desktop normal line boxes are20 vs16.
 * Compact scales nominal desktop boxes; DPR/zoom line rounding remains open. */
typedef struct RRUIHistory {RRUIRect panel;size_t visible_rows,best_index;float best_y,scale;bool best_visible;} RRUIHistory;
static inline float rr_ui_history_row_y(RRUIHistory h,size_t row)
{return h.panel.y+(float)(35.0+20.0*(double)row-(row>h.best_index?4.0:0))*h.scale;}
static inline float rr_ui_history_row_font(RRUIHistory h,size_t row){return (row==h.best_index?13:16)*h.scale;}
static inline float rr_ui_history_row_height(RRUIHistory h,size_t row){return (row==h.best_index?16:20)*h.scale;}
static inline RRUIHistory rr_ui_history_layout(RRUIRect anchor,float scale,float viewport_h,size_t count,size_t best_index)
{
    RRUIHistory h={0};h.panel=anchor;h.scale=scale;h.best_index=best_index<count?best_index:0;
    double list_height=count?20.0*(double)count-4.0:0;
    h.panel.h=(float)fmin((67.0+list_height)*scale,fmax(0,(double)viewport_h-anchor.y));
    /* Only viewport-visible prefix rows are painted; no count-sized traversal.
     * SIZE_MAX is safe: conversion to double occurs before multiplication. */
    double available=(scale>0?(viewport_h-anchor.y)/scale:0)-35;
    size_t cap=available>0?(size_t)ceil((available+4)/20):0;
    h.visible_rows=count<cap?count:cap;
    while(h.visible_rows&&rr_ui_history_row_y(h,h.visible_rows-1)>=viewport_h)h.visible_rows--;
    double best=anchor.y+(41.0+list_height)*scale;
    h.best_visible=best<viewport_h&&best+16*scale>0;
    h.best_y=h.best_visible?(float)best:0;return h;
}
static inline RRUIOverlayLayout rr_ui_pause_layout(float w,float h)
{
    /* Verified v3 desktop/compact card: border2 + padding30, brand38,
     * button61 with gap14; source #pause is left6vw and vertically centered. */
    RRUIOverlayLayout l={0};l.panel=(RRUIRect){w*.06f,(h-394)*.5f,380,394};
    for(int i=0;i<3;i++)l.actions[i]=(RRUIRect){l.panel.x+32,l.panel.y+84+(float)i*75,316,61};return l;
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

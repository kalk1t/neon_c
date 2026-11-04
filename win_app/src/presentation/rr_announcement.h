#ifndef RR_ANNOUNCEMENT_H
#define RR_ANNOUNCEMENT_H
#include "rr_css_font_metrics.h"
#include <stdbool.h>
#include <stdint.h>
typedef struct RRNoticeFade {double from,to,started,duration,reverse_start,factor;bool initialized,rendered;} RRNoticeFade;
/* CSS default ease = cubic-bezier(.25,.1,.25,1), not nitro's ease-out. */
static inline double rr_notice_ease(double x)
{
    if(x<=0)return 0;if(x>=1)return 1;
    double lo=0,hi=1,t=0;for(int i=0;i<40;i++){t=(lo+hi)*.5;double at=.75*t-.75*t*t+t*t*t;if(at<x)lo=t;else hi=t;}
    t=(lo+hi)*.5;return .3*t+2.4*t*t-1.7*t*t*t;
}
static inline double rr_notice_progress(const RRNoticeFade *f,double now)
{return f->duration>0?fmax(0,fmin(1,(now-f->started)/f->duration)):1;}
static inline double rr_notice_value(const RRNoticeFade *f,double now)
{return f->from+(f->to-f->from)*rr_notice_ease(rr_notice_progress(f,now));}
/* CSS Transitions1 reversal shortening; zero delay and binary opacity only.
 * Display:none cancels transitions; re-render has no before-change style. */
static inline double rr_notice_observe(RRNoticeFade *f,bool shown,bool rendered,double now)
{
    double target=shown?1:0;
    if(!f->initialized||!f->rendered||!rendered){*f=(RRNoticeFade){target,target,now,0,target,1,true,rendered};return rendered?target:0;}
    double progress=rr_notice_progress(f,now),value=rr_notice_value(f,now);
    if(target!=f->to){
        double factor=1,reverse=value;
        if(progress<1&&f->duration>0&&target==f->reverse_start){factor=fmin(1,fabs(rr_notice_ease(progress)*f->factor+1-f->factor));reverse=f->to;}
        if(value==target)factor=0;
        *f=(RRNoticeFade){value,target,now,.25*factor,reverse,factor,true,true};
    }
    return rr_notice_value(f,now);
}
typedef struct RRAnnouncementLayout {double x,y,width,height,cx,baseline,font,tracking;uint32_t color,near_glow,far_glow;} RRAnnouncementLayout;
static inline RRAnnouncementLayout rr_announcement_layout(double w,double h,double z,bool warn)
{
    RRAnnouncementLayout a={0};a.y=h*.24;a.width=w;a.height=105*z;a.cx=w*.5;a.font=84*z;a.tracking=6*z;
    a.baseline=rr_css_line_baseline(a.y,a.height,a.font,900);
    a.color=warn?UINT32_C(0xffd0f6ff):UINT32_C(0xffffffff);a.near_glow=warn?UINT32_C(0xff2bd6ff):UINT32_C(0x19e6ffff);a.far_glow=UINT32_C(0xff2bd6ff);return a;
}
#endif

#ifndef RR_TARGET_PULSE_H
#define RR_TARGET_PULSE_H
#include "rr_announcement.h"
typedef struct RRTargetPulse {double started;bool rendered;} RRTargetPulse;
/* Source pulse .8s infinite alternate, default ease; parent/target display:none
 * cancels it. Wall time is independent from gameplay dt and camera movement. */
static inline double rr_target_pulse_observe(RRTargetPulse *p,bool rendered,double now)
{
    if(!rendered){p->rendered=false;return 1;}
    if(!p->rendered){p->started=now;p->rendered=true;}
    double phase=fmod(fmax(0,now-p->started),1.6);
    double directed=phase<=.8?phase/.8:(1.6-phase)/.8;
    return 1+.25*rr_notice_ease(directed);
}
typedef struct RRTargetMarker {double cx,cy,radius,thickness,text_top,text_baseline,font;} RRTargetMarker;
static inline RRTargetMarker rr_target_marker(double x,double y,double zoom,double pulse)
{
    /* Container46 = diamond22 + span margin8 + span normal line16.
     * translate(-50%,-50%) centers the entire group on projected target. */
    RRTargetMarker s={x,y-12*zoom,22/sqrt(2.0)*zoom*pulse,3*zoom*pulse,y+7*zoom,0,13*zoom};
    s.text_baseline=rr_css_line_baseline(s.text_top,16*zoom,s.font,700);return s;
}
#endif

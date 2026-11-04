#ifndef RR_MOUSE_WIDGETS_H
#define RR_MOUSE_WIDGETS_H
#include "rr_ui_layout.h"
#include "rr_mouse_aim.h"
#include <stdint.h>
typedef struct RRMouseWidgetBoxes {RRUIRect slider,checkbox,invert_hit;float label_x,label_baseline,readout_x;} RRMouseWidgetBoxes;
typedef struct RRMouseWidgetEvent {double x,y;bool active,focused,pressed,released,held;} RRMouseWidgetEvent;
typedef struct RRMouseWidgetInput {double grab;bool dragging,check_armed;} RRMouseWidgetInput;
static inline RRMouseWidgetBoxes rr_mouse_widget_boxes(float x,float row_top,float invert_width)
{
    RRMouseWidgetBoxes b={0};b.slider=(RRUIRect){x+120.46875f,row_top+2,96,16};b.checkbox=(RRUIRect){b.slider.x+116,b.slider.y+2.5f,13,13};
    b.invert_hit=(RRUIRect){b.checkbox.x,row_top,17+invert_width,20};b.label_x=b.checkbox.x+17;b.label_baseline=(float)rr_css_line_baseline(row_top,20,12,700);b.readout_x=x+380;return b;
}
static inline bool rr_mouse_widget_hit(RRUIRect box,double x,double y)
{return x>=box.x&&x<box.x+box.w&&y>=box.y&&y<box.y+box.h;}
static inline double rr_mouse_widget_thumb(RRUIRect box,double value)
{return box.x+8+rr_clamp((value-.3)/2.2,0,1)*(box.w-16);}
static inline void rr_mouse_widgets_cancel(RRMouseWidgetInput *g){g->dragging=false;g->check_armed=false;g->grab=0;}
/* Native pointer adapter, not an assertion about every browser/OS range event.
 * Source45-step mapping and values remain shared. Keep thumb grab offset and
 * sample an owned release, including press+release coalesced in one frame. */
static inline double rr_mouse_widget_slider(RRMouseWidgetInput *g,RRUIRect box,double value,RRMouseWidgetEvent e)
{
    if(!e.active||!e.focused||!isfinite(e.x)||!isfinite(e.y)||box.w<=16){g->dragging=false;return value;}
    if(e.pressed){g->dragging=rr_mouse_widget_hit(box,e.x,e.y);if(g->dragging){double dx=e.x-rr_mouse_widget_thumb(box,value);g->grab=fabs(dx)<=8?dx:0;}}
    if(!e.held&&!e.pressed&&!e.released){g->dragging=false;return value;}
    if(g->dragging)value=rr_mouse_sensitivity_at((e.x-g->grab-box.x-8)/(box.w-16));
    if(!e.held)g->dragging=false;return value;
}
static inline bool rr_mouse_widget_checkbox(RRMouseWidgetInput *g,RRUIRect label,RRMouseWidgetEvent e)
{
    if(!e.active||!e.focused||!isfinite(e.x)||!isfinite(e.y)){g->check_armed=false;return false;}
    bool inside=rr_mouse_widget_hit(label,e.x,e.y);if(e.pressed)g->check_armed=inside;
    if(e.released){bool changed=g->check_armed&&inside;g->check_armed=false;return changed;}
    if(!e.held)g->check_armed=false;return false;
}
/* Default colors sampled from supplied Chromium Windows widget references. */
static inline uint32_t rr_mouse_widget_rgba(int index)
{
    static const uint32_t colors[]={UINT32_C(0x3b3b3bff),UINT32_C(0x858585ff),UINT32_C(0xff2bd6ff),UINT32_C(0xffffffff),UINT32_C(0x767676ff),UINT32_C(0xffffffff)};
    return index>=0&&index<6?colors[index]:0;
}
#endif

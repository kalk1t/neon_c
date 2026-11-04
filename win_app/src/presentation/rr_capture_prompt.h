#ifndef RR_CAPTURE_PROMPT_H
#define RR_CAPTURE_PROMPT_H
#include "rr_session.h"
#include "rr_css_font_metrics.h"
#define RR_CAPTURE_HINT_TEXT "CLICK TO CAPTURE MOUSE"
typedef struct RRMouseClickCapture {bool armed;} RRMouseClickCapture;
static inline bool rr_capture_phase(RRSessionPhase phase)
{return phase==RR_SESSION_ACTIVE||phase==RR_SESSION_COUNTDOWN;}
/* Native adapter for a canvas click: both down/up must belong to the focused
 * client gesture. Menu/pause/focus changes cancel ownership. No held-button
 * polling or automatic retries; source Input.lock ignores already-locked. */
static inline bool rr_capture_click_step(RRMouseClickCapture *c,RRSessionPhase phase,bool focused,bool captured,bool pressed,bool released,bool inside)
{
    if(!rr_capture_phase(phase)||!focused){c->armed=false;return false;}
    if(pressed)c->armed=inside;
    bool request=released&&c->armed&&inside&&!captured;
    if(released)c->armed=false;return request;
}
static inline bool rr_capture_hint_visible(RRSessionPhase phase,bool captured,bool touch)
{return phase==RR_SESSION_ACTIVE&&!captured&&!touch;}
typedef struct RRCaptureHintLayout {double cx,top,height,baseline,font,spacing;} RRCaptureHintLayout;
static inline RRCaptureHintLayout rr_capture_hint_layout(double w,double h,double zoom)
{
    RRCaptureHintLayout l={w*.5,h*.62,0,0,14*zoom,3*zoom};
    /* Nominal hhea normal-line model; no supplied visible hint box exists. */
    l.height=floor(rr_css_font_size(l.font,700)+.5);l.baseline=rr_css_line_baseline(l.top,l.height,l.font,700);return l;
}
#endif


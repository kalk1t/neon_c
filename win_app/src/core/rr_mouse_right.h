#ifndef RR_MOUSE_RIGHT_H
#define RR_MOUSE_RIGHT_H
#include <stdbool.h>
#include "rr_session.h"
/* Host viewport policy: visible flight canvas, excluding full-window overlays.
 * Actual native event delivery/DOM target hit-testing remain platform checks. */
static inline bool rr_mouse_right_canvas(RRSessionPhase phase,double x,double y,double width,double height)
{return (phase==RR_SESSION_ACTIVE||phase==RR_SESSION_COUNTDOWN||phase==RR_SESSION_FINISHED)&&x>=0&&y>=0&&x<width&&y<height;}
typedef struct RRMouseRight {bool held;} RRMouseRight;
/* Input pointerlockchange/blur release the logical button, independently of
 * physical state. Capture acquisition alone never synthesizes a new press. */
static inline void rr_mouse_right_release(RRMouseRight *s){s->held=false;}
/* Unlock while already unlocked and capture acquisition preserve a canvas press. */
static inline void rr_mouse_right_capture_change(RRMouseRight *s,bool was_captured,bool captured)
{if(was_captured&&!captured)rr_mouse_right_release(s);}
/* Native frame adapter: eligible means captured or a flight canvas press.
 * Polling reconciles missed releases; only a fresh eligible edge can rearm.
 * Press+release coalesced in a frame ends released. No mouse-delta filtering. */
static inline bool rr_mouse_right_step(RRMouseRight *s,bool focused,bool eligible,bool pressed,bool physical_held)
{
    if(!focused||!physical_held)s->held=false;
    else if(eligible&&pressed)s->held=true;
    return s->held;
}
#endif

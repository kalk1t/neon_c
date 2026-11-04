#ifndef RR_VIEW_CONTROLS_H
#define RR_VIEW_CONTROLS_H
#include "rr_session.h"
#include "rr_first_person.h"
typedef struct RRViewControls {int mode;bool interior;} RRViewControls;
/* Browser keeps state=finished after its results overlay opens. */
static inline bool rr_view_flight_phase(RRSessionPhase phase)
{return phase==RR_SESSION_ACTIVE||phase==RR_SESSION_COUNTDOWN||phase==RR_SESSION_FINISHED||phase==RR_SESSION_RESULTS;}
/* Call before pause/resume and simulation, once per frame. C is global; I is
 * flight-only and remains gated by the native cockpit candidate. No storage. */
static inline unsigned rr_view_controls_step(RRViewControls *view,RRSessionPhase phase,bool focused,bool interior_enabled,bool camera_hit,bool interior_hit)
{
    if(!focused||phase==RR_SESSION_ERROR)return 0;
    unsigned changes=0;
    if(camera_hit){if(view->interior){view->interior=false;changes++;}else view->mode=rr_camera_next(view->mode);}
    if(interior_enabled&&interior_hit&&rr_view_flight_phase(phase)){view->interior=!view->interior;changes++;}
    return changes;
}
#endif

#ifndef RR_AIM_CAMERA_H
#define RR_AIM_CAMERA_H
#include "rr_session.h"
/* Browser g.camAim is latched by updateAim, not inferred from current capture
 * at the camera call. Finished/results/countdown use car-follow; no autopilot. */
static inline bool rr_aim_camera_active(bool requested,RRSessionPhase phase,bool autopilot)
{return requested&&phase==RR_SESSION_ACTIVE&&!autopilot;}
#endif

#ifndef RR_PAUSE_SCENE_H
#define RR_PAUSE_SCENE_H
#include "rr_session.h"
/* Native deferred-start and focus safeguards surround the source pause showcase. */
typedef struct RRSceneClock {double last;bool initialized;} RRSceneClock;
static inline bool rr_scene_phase(RRSessionPhase phase)
{return phase==RR_SESSION_ACTIVE||phase==RR_SESSION_COUNTDOWN||phase==RR_SESSION_FINISHED||phase==RR_SESSION_RESULTS||phase==RR_SESSION_PAUSED;}
static inline bool rr_scene_frame_requested(RRSessionPhase phase,bool focused,bool smoke,bool ready,bool overloaded)
{return ready&&(focused||smoke)&&rr_scene_phase(phase)&&!(phase==RR_SESSION_PAUSED&&overloaded);}
/* Browser skips course/scenery/ads updates during pause. Resume uses current
 * absolute time, without subtracting pause duration. Camera clock stays live. */
static inline double rr_scene_frame_time(RRSceneClock *clock,RRSessionPhase phase,double now)
{if(!clock->initialized||phase!=RR_SESSION_PAUSED){clock->last=now;clock->initialized=true;}return clock->last;}
#endif

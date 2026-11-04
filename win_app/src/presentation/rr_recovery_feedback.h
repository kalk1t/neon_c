#ifndef RR_RECOVERY_FEEDBACK_H
#define RR_RECOVERY_FEEDBACK_H
#include "rr_recovery.h"
#include "rr_session.h"
#include "rr_hud_message.h"
/* Browser raceRules continues after completeLap, and state stays finished while
 * the results overlay is visible. Countdown-to-GO still skips rules that frame. */
static inline bool rr_recovery_playing(RRSessionPhase phase)
{return phase==RR_SESSION_ACTIVE||phase==RR_SESSION_FINISHED||phase==RR_SESSION_RESULTS;}
static inline void rr_recovery_announce(RRHUDMessage *message,unsigned events)
{
    if(events&RR_RECOVERY_WRONG_WAY)rr_hud_message_set(message,"WRONG WAY",.8,true);
    if(events&RR_RECOVERY_SIGNAL_LOST)rr_hud_message_set(message,"SIGNAL LOST",1.2,true);
}
/* Rules first, announcements in source order, then caller performs any respawn.
 * Signal lost may replace FINISH or WRONG WAY; wrong way waits for HUD expiry. */
static inline unsigned rr_recovery_feedback_conditions(RRSessionPhase phase,RRRecovery *state,RRHUDMessage *message,double along,double off,double height,double floor_height,double dt,bool roam)
{
    if(!rr_recovery_playing(phase))return 0;
    unsigned events=rr_recovery_conditions(state,along,off,height,floor_height,dt,message->timer,roam);
    rr_recovery_announce(message,events);return events;
}
static inline unsigned rr_recovery_feedback_update(RRSessionPhase phase,RRRecovery *state,const RRTrack *track,const RRCar *car,RRHUDMessage *message,double floor_height,double dt,bool roam)
{
    if(!rr_recovery_playing(phase))return 0;
    unsigned events=rr_recovery_update(state,track,car->pos,car->vel,floor_height,dt,message->timer,roam);
    rr_recovery_announce(message,events);return events;
}
#endif

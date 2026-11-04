#ifndef RR_PLAYER_CONTROLS_H
#define RR_PLAYER_CONTROLS_H
#include "rr_session.h"
#include "rr_recovery.h"
#include "rr_spin_control.h"
typedef struct RRPlayerControlState {
    RRSession *session;RRCar *car;RRRecovery *recovery;const RRTrack *track;
    double *aim_yaw,*aim_pitch,*spin_cd,*camera_kick;
} RRPlayerControlState;
typedef struct RRPlayerControlInput {double dt,steer;bool spin,reset,alive,reset_blocked;} RRPlayerControlInput;
typedef struct RRPlayerControlEvents {RRSpinResult spin;bool reset,reset_blocked;} RRPlayerControlEvents;
/* Source player respawn: place/rebase/centre aim only. Camera/FOV/look history,
 * aim direction until next updateAim, race progress and resource cooldowns stay.
 * No camera pointer is accepted, preventing the prototype's recovery snap. */
static inline void rr_player_controls_respawn(RRPlayerControlState *state)
{
    rr_session_respawn(state->session,state->car);
    rr_recovery_rebase(state->recovery,state->track,state->car->pos);
    *state->aim_yaw=state->session->race.safe_yaw;*state->aim_pitch=0;
}
/* After movement/rules/recovery and Nitro/impact: source playerWeapons spin,
 * then manual reset. Flags are host-supplied; no account/profile calls here. */
static inline RRPlayerControlEvents rr_player_controls_step(RRPlayerControlState *state,RRPlayerControlInput input,RRArsenalRandom random,void *context)
{
    RRPlayerControlEvents events={0};bool active=state->session->phase==RR_SESSION_ACTIVE;
    events.spin=rr_spin_control_step(state->spin_cd,active&&input.alive,input.dt,input.spin,input.steer,random,context);
    rr_spin_control_apply(events.spin,state->car,state->camera_kick);
    if(active&&input.alive&&input.reset){
        if(input.reset_blocked)events.reset_blocked=true;
        else{rr_player_controls_respawn(state);events.reset=true;}
    }
    return events;
}
#endif

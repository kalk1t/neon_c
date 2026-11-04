#ifndef RR_PLAYER_PHASE_H
#define RR_PLAYER_PHASE_H
#include "rr_battle_current.h"
#include "rr_local_run.h"
/* Explicit current offline full-pack player boundary. Current battle must be
 * started with a valid player/track/pose. Caller supplies processed controls,
 * simulation dt<=.05 and already advanced race clock/phase. Call BEFORE the
 * current battle tail; finish event suppresses its active flag that same tick.
 * Source movement -> rules/recovery -> smoke is synchronous. No rewards,
 * account storage, results scheduling, native input or production activation. */
bool rr_player_phase_step(RRBattleCurrent *current,RRInput control,double dt,
 double race_time,const struct RRWorld *world,RRCarStepReport *motion,unsigned *events);
/* Countdown uses idle FLIGHT; paused showcase advances time/local pose only. */
bool rr_player_phase_countdown(RRBattleCurrent *current,double dt,const struct RRWorld *world,RRCarStepReport *motion);
bool rr_player_phase_idle(RRBattleCurrent *current,double dt);
/* Source race-mode player respawn: no random jitter, no cooldown refill. Host
 * uses this after manual reset at its later post-weapons boundary too. */
bool rr_player_phase_respawn(RRBattleCurrent *current);
#endif

#ifndef RR_LOCAL_RUN_H
#define RR_LOCAL_RUN_H
#include "rr_battle.h"
#include "rr_session.h"
typedef struct {
    RRInput control;RRArsenalInput weapons;RRVec3 aim;
    bool manual_reset,manual_reset_allowed;
} RRRunInput;
typedef struct {
    RRLocalBattle *battle;RRSessionPhase phase,resume_phase;
    double time,race_time,countdown,last_nitro;int last_count;
    RRRunInput current_input;unsigned current_events;bool error;
} RRLocalRun;
enum {RR_RUN_GO=8,RR_RUN_RESPAWN=16,RR_RUN_MANUAL=32,RR_RUN_SMOKE=64,
      RR_RUN_IMPACT=128,RR_RUN_NITRO=256,RR_RUN_ERROR=512};
typedef struct {unsigned player;RRCarStepReport motion;RRPackEvents pack;} RRRunEvents;
/* Zero-init run/battle once. Caller initializes persistent cars/drivers and battle shot/FX/random/cue storage.
 * Source offline race reset (including player's two constructor-state reset draws);
 * retains global time, rank cadence, car/FX history and cannon side. No tickets,
 * best-lap storage, rewards/account operations or production activation. */
bool rr_local_run_start(RRLocalRun *run,RRLocalBattle *battle,const RRTrack *track,RRLocalRacer *player);
bool rr_local_run_pause(RRLocalRun *run);
bool rr_local_run_resume(RRLocalRun *run);
/* Complete local simulation tick with supplied processed control and aim.
 * Native mouse/touch/settings adapter and results scheduling remain host-owned.
 * Countdown GO retains entered branch; pause advances global/player idle time,
 * while movement, weapons and FX lifetimes remain frozen.
 * Real world is used for car collision; optional segment callback overrides the
 * default rr_world_segment(world,...,.6). No city data is fabricated. */
bool rr_local_run_tick(RRLocalRun *run,RRRunInput input,double raw_dt,
    const struct RRWorld *world,RRShotWorld segment,void *segment_context,RRRunEvents *events);
#endif

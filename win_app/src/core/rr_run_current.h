#ifndef RR_RUN_CURRENT_H
#define RR_RUN_CURRENT_H
#include "rr_player_phase.h"
/* Borrowed current graph, clock/input/event state. Zero once; no GPU/account or
 * production activation. Prepare/reset the current graph BEFORE begin. Global
 * time, FX and car/pose constructor history remain caller/graph-owned. */
typedef struct RRRunCurrent {RRLocalRun host;RRBattleCurrent *current;} RRRunCurrent;
/* Takes ownership of battle.after_weapons while attached; rejects a foreign
 * hook. Resets race/countdown/nitro clocks, preserves global host.time. */
bool rr_run_current_begin(RRRunCurrent *run,RRBattleCurrent *current);
void rr_run_current_detach(RRRunCurrent *run);
bool rr_run_current_pause(RRRunCurrent *run);
bool rr_run_current_resume(RRRunCurrent *run);
/* Full current CPU tick. Entered countdown branch survives GO. Pause advances
 * global/player idle time only; finished continues flight/clock/FX/coins with
 * combat damage gated. Nitro/impact feedback is emitted before rival dispatch.
 * Native processed input/aim/autopilot, reset preparation, actual audio/camera
 * application and results presentation remain external. Null world is explicit
 * free space; supplied world defaults projectile segments to radius .6. */
bool rr_run_current_tick(RRRunCurrent *run,RRRunInput input,double raw_dt,
 const struct RRWorld *world,RRShotWorld segment,void *context,RRRunEvents *events);
#endif

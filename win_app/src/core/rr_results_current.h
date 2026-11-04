#ifndef RR_RESULTS_CURRENT_H
#define RR_RESULTS_CURRENT_H
#include "rr_run_current.h"
/* Current two-lap display snapshot, fixed <=11 racers and <=2 player laps.
 * Identity indices address caller-owned names/paint; no HTML or service data.
 * player_takedowns is copied at publication, never read live during repaint.
 * Estimated time is display pace, not a finish/ranking/progression mutation. */
typedef struct RRResultsTime {int racer;double seconds;bool estimated;} RRResultsTime;
typedef struct RRResultsCoin {int racer,coins;} RRResultsCoin;
typedef struct RRResultsCurrent {double total,laps[2];size_t lap_count,best_lap,time_count,coin_count;RRResultsTime times[RR_LOCAL_MAX_CARS];RRResultsCoin coins[RR_LOCAL_MAX_CARS];int player_time_rank,player_coin_rank,finish_position,player_takedowns;} RRResultsCurrent;
/* Source showResults phase guard, stable time and coin sorts, first best-lap
 * tie. Empty history has best_lap=SIZE_MAX. Solo standings are source-empty.
 * All inputs stay untouched; failure leaves output untouched. This contains no
 * bonus/bank calculation, account call, unlock, telemetry, timer or GUI draw. */
bool rr_results_current_build(const RRLocalRacer *const *racers,size_t count,size_t player,int samples,double race_time,int finish_position,RRSessionPhase phase,RRResultsCurrent *out);
/* Borrowed current pack adapter; only the initialized two-lap race profile.
 * Caller decides when the source's two-second presentation callback is due. */
bool rr_run_results_build(const RRRunCurrent *run,RRResultsCurrent *out);
#endif

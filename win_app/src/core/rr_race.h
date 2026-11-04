#ifndef RR_RACE_H
#define RR_RACE_H
#include "rr_flight.h"
#include "rr_track.h"

typedef enum RRRaceMode { RR_RACE_TWO_LAPS, RR_RACE_TIME_ATTACK, RR_RACE_FREE_ROAM } RRRaceMode;
typedef enum RRRaceEvent { RR_RACE_NONE=0, RR_RACE_CHECKPOINT=1, RR_RACE_LAP=2, RR_RACE_FINISH=4 } RRRaceEvent;
typedef struct RRRace {
    RRRaceMode mode;
    int lap, next_checkpoint, completed_laps;
    double lap_time, last_lap_time, best_lap_time, finish_time;
    RRVec3 safe_position;
    double safe_yaw;
    bool finished;
} RRRace;

void rr_race_reset(RRRace *race, RRRaceMode mode, RRVec3 spawn, double yaw);
/* Call once after flight, with the same dt/race clock as the browser. Caller
 * handles countdown/pause, warnings, respawn, rewards and lap-history storage.
 * Gates must be ordered, with unit forward vectors. Supports all road sizes.
 * This intentionally preserves browser endpoint aperture tests, not swept tests.
 */
unsigned rr_race_step(RRRace *race, const RRGate *gates, int count,
                      RRCar *car, RRVec3 previous, double dt, double race_time);
/* Current source rivals use strict45m aperture; player/legacy stays15m. */
unsigned rr_race_step_rival_current(RRRace *race,const RRGate *gates,int count,RRCar *car,RRVec3 previous,double dt,double race_time);
#endif

#ifndef RR_AI_H
#define RR_AI_H
#include "rr_flight.h"
#include "rr_track.h"
#include <stddef.h>
#include <stdint.h>
#define RR_AI_RIVALS 10
typedef struct RRAILivery {const char *name;uint32_t paint,accent;double skill,metal;bool has_metal;} RRAILivery;
typedef struct RRAIDriver {int sample;double seed,lane,lane_y,dodge,skill;} RRAIDriver;
typedef struct RRAIPeer {RRVec3 pos;bool alive;} RRAIPeer;
const RRAILivery *rr_ai_livery(int index);
/* CPU controller only. Peers exclude self, retain source racer iteration order.
 * time/dt are simulation values already prepared by the caller; no extra cap.
 * Runtime spawning/combat/rewards and graphics are deliberately separate. */
bool rr_ai_controls(RRAIDriver *driver,const RRCar *car,const RRTrack *track,
    const RRAIPeer *peers,size_t count,RRVec3 target,double time,double dt,
    const RRVec3 *coin,RRInput *out);
double rr_ai_progress(int sample,int lap,int next_checkpoint,int track_samples);
double rr_ai_speed_factor(double player_progress,double rival_progress,int track_samples,double road_factor);
#endif

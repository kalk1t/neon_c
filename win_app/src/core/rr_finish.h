#ifndef RR_FINISH_H
#define RR_FINISH_H
#include "rr_track.h"
#include "rr_flight.h"
#include <stddef.h>
/* Browser's post-finish line follower; hint is the prior recovery sample. */
RRInput rr_finish_controls(const RRTrack *track,const RRCar *car,int sample);
/* Source autopilot also copies pre-movement orientation into the aim state.
 * Retain the exact forward vector: source copies it without normalization. */
typedef struct RRFinishAim {double yaw,pitch;RRVec3 direction;} RRFinishAim;
RRInput rr_finish_player_controls(const RRTrack *track,const RRCar *car,int sample,RRFinishAim *aim);
typedef struct RRResultSummary {double total;size_t best;} RRResultSummary;
RRResultSummary rr_finish_summary(const double *laps,size_t count);
#endif

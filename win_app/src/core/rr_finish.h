#ifndef RR_FINISH_H
#define RR_FINISH_H
#include "rr_track.h"
#include "rr_flight.h"
#include <stddef.h>
/* Browser's post-finish line follower; hint is the prior recovery sample. */
RRInput rr_finish_controls(const RRTrack *track,const RRCar *car,int sample);
typedef struct RRResultSummary {double total;size_t best;} RRResultSummary;
RRResultSummary rr_finish_summary(const double *laps,size_t count);
#endif

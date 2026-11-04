#ifndef RR_RESULTS_TRACKS_DRAW_H
#define RR_RESULTS_TRACKS_DRAW_H
#include "rr_results_bound_flow.h"
#include "rr_results_tracks.h"
/* Frozen bound view -> copied strings/styles -> computed bounded source tracks
 * -> existing complete native body/fragment preflight. Profile track values and
 * row pointers/counts are not consumed. Other min-content bounds, caption height
 * and other slot metrics remain explicit qualified inputs. No main activation.
 * Same disjoint/stable caller storage/lifetime contract as bound-flow adapter. */
RRResultsNameStatus rr_ui_results_tracks_draw(RRUI *ui,RRResultsArt *art,
 const RRResultsBoundView *view,const RRResultsBodyInput *profile,const double other_bounds[2],
 int device_scale,RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row);
#endif

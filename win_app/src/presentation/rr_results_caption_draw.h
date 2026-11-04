#ifndef RR_RESULTS_CAPTION_DRAW_H
#define RR_RESULTS_CAPTION_DRAW_H
#include "rr_results_caption.h"
#include "rr_results_tracks_draw.h"
/* Frozen view -> canonical display caption/height + guarded source tracks ->
 * existing complete-board native ROW draw adapter. Caller profile caption and
 * tracks/row pointers/counts are not consumed. Other slot metrics remain inputs.
 * Caption text is returned for future caption painting, not painted here. Out
 * updates only if full native adapter succeeds; no stale output paint on error.
 * Disjoint stable caller storage/lifetimes as existing adapters; no main or
 * reward/account activation, and no cold texture load/graphics creation. */
RRResultsNameStatus rr_ui_results_caption_flow_draw(RRUI *ui,RRResultsArt *art,
 const RRResultsBoundView *view,const RRResultsBodyInput *profile,int device_scale,
 RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row,
 RRResultsCaption *caption);
#endif

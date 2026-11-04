#ifndef RR_RESULTS_SLOTS_DRAW_H
#define RR_RESULTS_SLOTS_DRAW_H
#include "rr_results_slots.h"
#include "rr_results_caption_draw.h"
/* Explicit empty-account source candidate only. Frozen view + viewport/scroll
 * derive slot/caption/track/body metrics and existing native ROW commands.
 * No measured card/slot/caption/track dimensions are supplied. Same disjoint,
 * stable storage/lifetime and failure contracts as caption adapter. Account
 * profile is not auto-detected. Does not paint header/caption/card decoration,
 * activate main, load cold resources or create a graphics/audio context. */
RRResultsNameStatus rr_ui_results_empty_profile_draw(RRUI *ui,RRResultsArt *art,
 const RRResultsBoundView *view,double width,double height,double scroll,int scale,
 RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row,
 RRResultsCaption *caption);
#endif

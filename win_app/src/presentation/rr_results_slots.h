#ifndef RR_RESULTS_SLOTS_H
#define RR_RESULTS_SLOTS_H
#include "rr_results_card.h"
#include "rr_results_current.h"
#include "rr_results_name_flow.h"
/* Explicit CAPTURE EMPTY-ACCOUNT profile, not inferred account status: online
 * visible/empty; unlock/next hidden. Exactly2 laps0..3600s each, count1..11,
 * valid frozen finish/coin ranks and takedowns0..9. Other states NEEDS_FLOW or
 * INVALID, never truncate/wrap/fabricate account text. Caller must select this
 * profile knowingly; not a substitute for live account/service integration.
 * Fixed source CSS/font metrics derive FINISH/place/total/laps/button heights.
 * COLUMNS=0 placeholder is overwritten by body solver. Output unchanged on
 * failure; no text painting, backend, reward, main or graphics activation. */
RRResultsNameStatus rr_results_empty_slots(const RRResultsCurrent *snapshot,
 RRResultsCardContent *out);
#endif

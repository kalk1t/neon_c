#ifndef RR_RESULTS_BOUND_FLOW_H
#define RR_RESULTS_BOUND_FLOW_H
#include "rr_results_flow_draw.h"
#include "rr_results_label.h"
typedef struct RRResultsFlowBinding {
 RRResultsLabel labels[2][RR_LOCAL_MAX_CARS];RRResultsFlowText rows[2][RR_LOCAL_MAX_CARS];
 bool player[2][RR_LOCAL_MAX_CARS];size_t counts[2],revision;
} RRResultsFlowBinding;
/* Caller-owned persistent binding. The frozen bound view is formatted in its
 * existing numeric display order; complete-board validation precedes commit.
 * Requires each expected list to cover the roster once. Failure leaves binding
 * unchanged; never paints an old binding after new-view failure. Strings are
 * copied; row pointers refer INTO binding, which must not be shallow-copied.
 * This pure function does not advance timers, look up identities or tick core.
 * Font/unsupported-text/whole-card/graphics qualification remains downstream. */
bool rr_results_flow_prepare(const RRResultsBoundView *view,RRResultsFlowBinding *binding);
/* Bound-draw callback adapter: profile supplies qualified tracks/caption/other
 * slots and viewport/scroll only. Its row pointers/counts are overwritten from
 * frozen binding. All inputs/work/binding/row storage must be disjoint and stable
 * for the call. A repaint failure never replays a simulation/account operation.
 * Does not activate native main or enable rivals/coins/combat/rewards. */
RRResultsNameStatus rr_ui_results_bound_flow_draw(RRUI *ui,RRResultsArt *art,
 const RRResultsBoundView *view,const RRResultsBodyInput *profile,int device_scale,
 RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row);
#endif

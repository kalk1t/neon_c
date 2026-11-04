#ifndef RR_RESULTS_BODY_FLOW_H
#define RR_RESULTS_BODY_FLOW_H
#include "rr_results_list_flow.h"
#include "rr_results_card.h"
/* Bounded source desktop profile. Width/height are integer logical CSS pixels;
 * qualified track widths and positive single-block caption height remain inputs.
 * Other content supplies measured intrinsic slot heights/visibility; COLUMNS is
 * overwritten from generated list flow, never consumed from that input. No font
 * shaping of labels/caption, track auto-minimum sizing or account calculation. */
typedef struct RRResultsBodyInput {
 double width,height,scroll,left_width,right_width,caption_height;
 const RRResultsFlowText *standings,*coins;size_t standings_count,coin_count;
 RRResultsCardContent other;
} RRResultsBodyInput;
typedef struct RRResultsBodyLayout {
 RRResultsCardLayout card;RRResultsGridMetrics metrics;
 RRResultsListSummary lists[2];RRUIRect labels[2],list_boxes[2],caption;
} RRResultsBodyLayout;
/* Fixed source label19, coin top margin6, caption top margin8, list indent26.
 * Parent natural heights include protruding floats independently of ordinary
 * list height. Native viewport boxes are float conversions of logical geometry.
 * Source-compatible floor1/64 centering and integer scrollHeight rounding are
 * separate from retained legacy card API. Scroll input is already logical CSS
 * pixels, not OS wheel/DPI policy. Whole output unchanged on failure; caller
 * work may change. No heap/large local scratch, sink/paint/main activation. */
RRResultsNameStatus rr_results_body_flow(const RRResultsBodyInput *input,
 RRResultsListWork *work,RRResultsBodyLayout *out);
#endif

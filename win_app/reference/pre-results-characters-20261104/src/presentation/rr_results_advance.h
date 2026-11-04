#ifndef RR_RESULTS_ADVANCE_H
#define RR_RESULTS_ADVANCE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef enum RRResultsAdvanceStatus {RR_ADVANCE_INVALID,RR_ADVANCE_NEEDS_SHAPING,RR_ADVANCE_READY} RRResultsAdvanceStatus;
typedef struct RRResultsAdvance {uint64_t ticks;size_t codepoints;double advance,range_width;} RRResultsAdvance;
/* Bound to verified15CSSpx/700 Orbitron metrics and existing approx/queen
 * fallback advances. Plain supported unkerned single-line runs only: printable
 * primary cmap excluding missing caret, plus em dash/approx/queen. Nonzero GPOS
 * pairs or leading/trailing/repeated spaces return NEEDS_SHAPING, never a guess.
 * Max128 codepoints/<512UTF8bytes; strict decoding, no '?' substitution. Exact
 * integer ticks use1/64000CSSpx; range_width is outward1/64 width envelope.
 * This is NOT per-character shaping, a float-fit epsilon, min-content/grid sizing,
 * line breaking or a new glyph painter. Failure leaves out untouched. No heap. */
RRResultsAdvanceStatus rr_results_advance(const char *text,RRResultsAdvance *out);
#endif

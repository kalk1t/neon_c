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
typedef struct RRResultsCharacterRange {uint32_t codepoint;double pen,left,right;} RRResultsCharacterRange;
typedef struct RRResultsCharacters {RRResultsCharacterRange ranges[128];size_t count;double advance,range_width;} RRResultsCharacters;
/* Supported single-line15px/700 primary + queen character Range X envelopes.
 * Reuses all advance preflight/kerning/whitespace/UTF8 limits. Approx fallback
 * returns NEEDS_SHAPING: its captured range width is not an unrounded raw advance.
 * Float32 font scale/advance, double accumulation, outward1/64 Range bounds.
 * These are character selection envelopes, NOT glyph bitmap bounds/origins,
 * raster/shadow/vertical placement, float wrapping or painter activation.
 * Persistent caller output (~4KiB), no heap or large temporary. Immutable text
 * must not alias output or change during synchronous multi-pass validation.
 * Any supported-input failure leaves output untouched. */
RRResultsAdvanceStatus rr_results_characters(const char *text,RRResultsCharacters *out);
#endif

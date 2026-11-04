#ifndef RR_RESULTS_CAPTION_H
#define RR_RESULTS_CAPTION_H
#include "rr_results_current.h"
#include "rr_results_name_flow.h"
enum {RR_RESULTS_CAPTION_BYTES=64};
typedef struct RRResultsCaption {char text[RR_RESULTS_CAPTION_BYTES];double height;} RRResultsCaption;
/* Canonical res-bank display text and unwrapped Windows-source font profile.
 * Count/rank1..11 only; ordinal/bonus wording matches source expressions. This
 * is DISPLAY METADATA ONLY: no wallet amount, save, reward or unlock operation.
 * Source12px/600 selects verified Orbitron700; missing middle dot uses source
 * Arial Black numeric ascent/descent. Round normal metrics separately, union
 * fallback extents when dot appears:15px plain,17px bonus. Qualified against60
 * existing captures, not arbitrary fonts/captions/HTML or native raster.
 * All66 canonical text bounds fit below258.25; smaller width NEEDS_FLOW rather
 * than guessing wrapping. Input/output disjoint; unchanged out on failure. */
RRResultsNameStatus rr_results_caption_build(const RRResultsCurrent *snapshot,
 double column_width,RRResultsCaption *out);
#endif

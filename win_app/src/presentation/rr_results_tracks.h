#ifndef RR_RESULTS_TRACKS_H
#define RR_RESULTS_TRACKS_H
#include "rr_results_list_flow.h"
typedef struct RRResultsTracks {double minimum[2],width[2],overflow;} RRResultsTracks;
/* Fixed source616px grid /22px gap /1.3fr 1fr. Min-content from qualified body
 * name Latin/digit words, Latin-letter hyphens, source crown prefix and numeric
 * clock/coin/approx values, with26px list indent. Font/whitespace/GPOS rejects
 * follow character API. This is a BOUNDED source profile, not generic CSS/UAX.
 * Other content requires finite nonnegative qualified min-content UPPER bounds.
 * They must be nonbinding in the computed tracks; else NEEDS_FLOW without
 * guessing their actual min-content. Current fixed headings/canonical captions
 * have separately verified258.25px bounds, not arbitrary account/profile text.
 * Inputs/work/out disjoint/stable; scratch may change, output only on success.
 * No heap, layout pixels/draw or source measured track widths as inputs. */
RRResultsNameStatus rr_results_tracks(const RRResultsFlowText *standings,size_t standings_count,
 const RRResultsFlowText *coins,size_t coin_count,const double other_bounds[2],
 RRResultsCharacters *scratch,RRResultsTracks *out);
#endif

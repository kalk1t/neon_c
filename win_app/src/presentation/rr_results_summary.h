#ifndef RR_RESULTS_SUMMARY_H
#define RR_RESULTS_SUMMARY_H
#include "rr_results_current.h"
enum {RR_RESULTS_ORDINAL_BYTES=16,RR_RESULTS_PLACE_BYTES=96};
typedef struct RRResultsSummary {char place[RR_RESULTS_PLACE_BYTES],coin_ordinal[RR_RESULTS_ORDINAL_BYTES];} RRResultsSummary;
/* Source positive-integer ordinal suffix, including11/12/13 and111..113.
 * Caller owns a16-byte buffer; invalid input leaves it unchanged. No heap. */
bool rr_results_ordinal(int value,char out[RR_RESULTS_ORDINAL_BYTES]);
/* Source frozen place/takedown header and coin-position ordinal, display only.
 * Solo place text is empty. UTF8 middle dot is actual source text, not an ASCII
 * substitute. Does not compute bonus/bank amount, send a save or grant anything.
 * Use coin_ordinal as a value, not a complete res-bank/account message. Failure
 * leaves output untouched. Font/fallback measurement and painting are external. */
bool rr_results_summary_build(const RRResultsCurrent *snapshot,RRResultsSummary *out);
#endif

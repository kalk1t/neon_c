#ifndef RR_RESULTS_TIMES_H
#define RR_RESULTS_TIMES_H
#include "rr_results_current.h"
typedef struct RRResultsTimes {char total[32],laps[2][64];size_t count,best;} RRResultsTimes;
typedef struct RRResultsTimesLayout {float total_baseline,lap_baseline,left[2];} RRResultsTimesLayout;
/* Source display text, <=2 completed laps, first minimum wins. Finite positive
 * race-time profile <=1e12 seconds; fail atomically outside supported bounds.
 * Numeric snapshot and strings are copied; no borrowed live-racer data. */
bool rr_results_times_build(const double *laps,size_t count,RRResultsTimes *out);
bool rr_results_times_current(const RRResultsCurrent *snapshot,RRResultsTimes *out);
/* Caller supplies measured native bold label widths; preserve source 28px gap
 * and center the complete group, including the one/zero-lap cases. */
bool rr_results_times_layout(const RRResultsTimes *text,float center,float panel_y,
 const float widths[2],RRResultsTimesLayout *out);
#endif

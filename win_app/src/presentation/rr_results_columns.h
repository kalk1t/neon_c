#ifndef RR_RESULTS_COLUMNS_H
#define RR_RESULTS_COLUMNS_H
#include "rr_results_bind.h"
#include "rr_results_card.h"
#include "rr_results_text.h"
typedef enum RRResultsColumnsStatus {RR_RESULTS_COLUMNS_INVALID,RR_RESULTS_COLUMNS_NEEDS_HEIGHT,RR_RESULTS_COLUMNS_NEEDS_WRAP,RR_RESULTS_COLUMNS_READY} RRResultsColumnsStatus;
typedef struct RRResultsColumnsResult {RRResultsColumnsStatus status;RRResultsRowKind kind;size_t row,rows;} RRResultsColumnsResult;
/* List tops are qualified measured border-box tops, not guessed header heights.
 * Source ordered-list padding26 and15px*1.42 line boxes; widths come from the
 * card's actual columns. Preflight BOTH complete lists before any paint command.
 * Reject unsupported wrapping or insufficient intrinsic column height explicitly;
 * do not truncate, shrink, substitute or paint a partial board. Ordinal markers,
 * headings and wrapped HTML inline/float layout remain separate work.
 * scratch is persistent caller storage (one row), no heap or22-row stack array.
 * View/font/card/tops must stay stable through the synchronous paint boundary;
 * sink must not mutate them or scratch. Result.row is meaningful only on failure.
 * This can be used from a bound-draw callback; it does not activate native main. */
RRResultsColumnsResult rr_results_columns_emit(Font font,const RRResultsBoundView *view,
 const RRResultsCardLayout *card,const double list_top[2],int device_scale,
 RRResultsRow *scratch,RRResultsPaintSink sink,void *context);
#endif

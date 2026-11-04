#ifndef RR_RESULTS_LABEL_H
#define RR_RESULTS_LABEL_H
#include "rr_results_current.h"
typedef enum RRResultsLabelKind {RR_LABEL_STANDINGS,RR_LABEL_COINS} RRResultsLabelKind;
typedef struct RRResultsLabel {char name[512],value[64];size_t racer;bool player;} RRResultsLabel;
/* Source text composition only, shared by retained row renderer and new flow
 * binding. Snapshot numeric order and frozen names are inputs. No re-sort,
 * provider/account access, font/fit claim, truncation or replacement glyphs.
 * Invalid input leaves output unchanged; UTF8/font shaping is downstream. */
bool rr_results_label_build(const RRResultsCurrent *snapshot,RRResultsLabelKind kind,
 size_t row,const char *const *names,size_t count,size_t player,RRResultsLabel *out);
#endif

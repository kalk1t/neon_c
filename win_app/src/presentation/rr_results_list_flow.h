#ifndef RR_RESULTS_LIST_FLOW_H
#define RR_RESULTS_LIST_FLOW_H
#include "rr_results_name_flow.h"
typedef struct RRResultsNameTail {double y,advance;} RRResultsNameTail;
/* Current measured source one-line value float, relative to li. Prior right
 * floats must be inside width, line-aligned,21.296875 high and at/before li top.
 * Other contexts return NEEDS_FLOW. Tail is final NAME line-box y and raw pen
 * advance, not its outward Range width. Source cursor floors1/64; preferred
 * value width rounds outward. No global epsilon, shrink/truncate or repaint. */
RRResultsNameStatus rr_results_value_float(const char *value,double width,RRResultsNameTail tail,
 const RRResultsRightFloat *previous,size_t count,RRResultsCharacters *scratch,RRResultsRightFloat *out);
typedef enum RRResultsListKind {RR_FLOW_STANDINGS,RR_FLOW_COINS} RRResultsListKind;
typedef struct RRResultsFlowText {const char *name,*value;} RRResultsFlowText;
typedef struct RRResultsListSummary {size_t rows;double normal_height,float_bottom,height;} RRResultsListSummary;
typedef struct RRResultsListWork {RRResultsCharacters name_characters,value_characters;RRResultsNameFlow name;RRResultsRightFloat absolute[11],relative[11];} RRResultsListWork;
typedef struct RRResultsListRowView {size_t index;double y,height;const RRResultsNameFlow *name;const RRResultsCharacters *name_characters,*value_characters;RRResultsRightFloat value;} RRResultsListRowView;
typedef void (*RRResultsListSink)(void *context,const RRResultsListRowView *row);
/* Fully coupled layout from frozen strings and qualified content width only.
 * Ledger states are GENERATED, not measured inputs. Normal li flow advances by
 * name height even when current value floats below it. Source standings columns:1
 * contains floats; ordinary coins height remains normal flow. Both extents kept.
 * Preflight entire list before first synchronous sink. NULL sink measures only.
 * Borrowed views valid only during callback; sink must not mutate inputs/work or
 * retain pointers. Caller owns persistent work (sizes in tests), no heap/large
 * local plans. Work may change on failure; summary and external sinks do not.
 * No sorting/account/progression/main/painter/graphics activation. */
RRResultsNameStatus rr_results_list_flow(RRResultsListKind kind,const RRResultsFlowText *rows,size_t count,double width,
 RRResultsListWork *work,RRResultsListSink sink,void *context,RRResultsListSummary *out);
#endif

#ifndef RR_RESULTS_NAME_FLOW_H
#define RR_RESULTS_NAME_FLOW_H
#include "rr_results_advance.h"
enum {RR_RESULTS_NAME_LINES=128,RR_RESULTS_NAME_FLOATS=11};
typedef struct RRResultsRightFloat {double x,y,width,height;} RRResultsRightFloat;
typedef struct RRResultsNameLine {size_t first,count;double x,y,width,height;} RRResultsNameLine;
typedef struct RRResultsNameFlow {RRResultsNameLine lines[RR_RESULTS_NAME_LINES];size_t count;double height;} RRResultsNameFlow;
typedef enum RRResultsNameStatus {RR_NAME_INVALID,RR_NAME_NEEDS_FLOW,RR_NAME_READY} RRResultsNameStatus;
/* Source15px/700 NAME text only. Container width and preceding right-float
 * boxes are qualified inputs relative to this li top/left. Current measured
 * one-line floats must align to21.296875px line boxes and remain inside width;
 * other float geometry returns NEEDS_FLOW. No current-row float is placed.
 * Supports measured single spaces and Latin-letter hyphen breaks. Unqualified
 * kerning/collapsing whitespace/break contexts or unbroken full-width overflow
 * return NEEDS_FLOW; no truncation, tidy-row rewrite or universal fit epsilon.
 * Output line ranges use original codepoint offsets; collapsed break spaces
 * are omitted. y is selection-range top (line+1), h19; not a glyph baseline.
 * height includes leading float clearance even with only one visible text line.
 * Caller owns output and character scratch; no heap/large local. Scratch may
 * change on failure, output does not. Text/floats/scratch/out must not alias or
 * mutate across synchronous deterministic preflight/emission passes. */
RRResultsNameStatus rr_results_name_flow(const char *text,double width,
 const RRResultsRightFloat *previous,size_t float_count,
 RRResultsCharacters *scratch,RRResultsNameFlow *out);
#endif

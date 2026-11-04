#ifndef RR_RESULTS_FLOW_DRAW_H
#define RR_RESULTS_FLOW_DRAW_H
#include "rr_results_art.h"
#include "rr_results_body_flow.h"
/* Frozen source-order strings and player flags. Input/font/work/row/flags/out
 * must remain disjoint and stable through synchronous preflight and emission;
 * sinks may record/paint but must not mutate those inputs or retain views.
 * Caller owns persistent list work and one row scratch. No heap/large local
 * text plans. Geometry follows qualified body flow; primary/fallback pen origins
 * use verified raw font advances, not raylib atlas advance approximations.
 * Baselines/ink/shadow raster remain nominal native approximations. Ordinals,
 * headings, caption and card decoration are not painted by this content bridge. */
RRResultsNameStatus rr_results_flow_emit(Font font,const RRResultsBodyInput *input,
 const bool *standings_player,const bool *coin_player,int device_scale,
 RRResultsListWork *work,RRResultsRow *row,RRResultsPaintSink sink,void *context);
/* Loaded-resource metadata preflight before all commands. Does not load images,
 * create a graphics/audio context, tick gameplay or activate main/accounts. */
RRResultsNameStatus rr_ui_results_flow_draw(RRUI *ui,RRResultsArt *art,
 const RRResultsBodyInput *input,const bool *standings_player,const bool *coin_player,
 int device_scale,RRResultsListWork *work,RRResultsRow *row);
#endif

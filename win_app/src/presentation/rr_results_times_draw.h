#ifndef RR_RESULTS_TIMES_DRAW_H
#define RR_RESULTS_TIMES_DRAW_H
#include "rr_ui.h"
#include "rr_results_times.h"
/* Actual native UI sink. Preflights exact bold em dash and all measurements
 * before drawing; failure emits no timing text rather than '?' replacement.
 * Existing native glyph raster/glow approximation remains visually unqualified. */
bool rr_ui_results_times_draw(RRUI *ui,const RRResultsTimes *text,float center,float panel_y);
#endif

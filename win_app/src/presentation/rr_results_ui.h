#ifndef RR_RESULTS_UI_H
#define RR_RESULTS_UI_H
#include "rr_results_timer.h"
/* Synchronous, borrowed immutable display data. Callback must not mutate the
 * timer/run, retain the pointer across publication, or invoke account services.
 * Called every visible frame, including unchanged revisions (immediate mode). */
typedef bool (*RRResultsDraw)(void *context,const RRResultsCurrent *snapshot,size_t revision);
/* Native GetTime-style seconds, not simulation dt. Call once per UI frame,
 * AFTER any completed current tick and ordered UI transition. tick may be NULL
 * on menu/paused/UI-only frames; each non-NULL completed event is submitted
 * exactly once. hide models the source hidden-class action, not a phase change.
 * Order: queue finish, hide if requested, pump due callbacks, draw frozen board.
 * The caller must set the actual run phase before this boundary. Old callbacks
 * may republish after hide if the current phase is still finished, as in source.
 * Return false on invalid graph/clock/queue or draw failure. Draw failure leaves
 * published data/queue intact for inspection/repaint, never retries simulation.
 * This adapter does not launch graphics or activate a production full pack. */
bool rr_results_ui_frame(RRResultsTimer *timer,const RRRunCurrent *run,
 const RRRunEvents *tick,double wall_seconds,bool hide,RRResultsDraw draw,
 void *context,RRResultsPump *events);
#endif

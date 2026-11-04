#ifndef RR_RESULTS_TIMER_H
#define RR_RESULTS_TIMER_H
#include "rr_results_current.h"
enum {RR_RESULTS_TIMERS=16};
/* Zero once per application, not per race. Fixed display-only callback queue;
 * cached numeric snapshot remains immutable until another callback publishes.
 * Source start/menu hides the card but does not cancel already queued timers. */
typedef struct RRResultsTimer {double due[RR_RESULTS_TIMERS],last_ms;size_t pending,revision;bool clock_ready,visible,has_snapshot,error;RRResultsCurrent snapshot;} RRResultsTimer;
typedef struct RRResultsPump {unsigned callbacks,published;} RRResultsPump;
void rr_results_timer_hide(RRResultsTimer *timer);
/* Monotonic finite milliseconds, >=0. Source setTimeout(showResults,2000).
 * Queue/clock/model errors latch failure: stop, never silently drop callbacks. */
bool rr_results_timer_schedule(RRResultsTimer *timer,double now_ms);
bool rr_results_timer_pump(RRResultsTimer *timer,const RRRunCurrent *run,double now_ms,RRResultsPump *events);
/* Call after a completed current tick at a chosen UI task boundary, using its
 * finish timestamp. Real OS scheduling/RAF order and rendering remain external.
 * Does not alter run phase: source keeps finished motion under the results UI. */
bool rr_run_results_after_tick(RRResultsTimer *timer,const RRRunCurrent *run,const RRRunEvents *tick,double now_ms,RRResultsPump *events);
#endif

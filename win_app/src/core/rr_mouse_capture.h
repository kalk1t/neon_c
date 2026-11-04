#ifndef RR_MOUSE_CAPTURE_H
#define RR_MOUSE_CAPTURE_H
#include <stdbool.h>
#include <math.h>
typedef struct RRMouseCapture {double changed_ms;bool captured;} RRMouseCapture;
static inline void rr_mouse_capture_change(RRMouseCapture *state,bool captured,double now_ms)
{state->changed_ms=now_ms;state->captured=captured;}
/* Source Input mousemove guards: unlocked, then strict age<150ms.
 * Native host applies this eligibility to frame totals; it must not apply the
 * browser's separate per-event bogus-jump filter to those aggregates. */
static inline bool rr_mouse_capture_allows(const RRMouseCapture *state,double now_ms)
{return state->captured&&isfinite(now_ms)&&isfinite(state->changed_ms)&&now_ms-state->changed_ms>=150;}
#endif

#ifndef RR_PAUSE_UI_H
#define RR_PAUSE_UI_H
#include "rr_session.h"
#include "rr_showcase.h"
/* main.js pause()/Escape handler uses monotonic milliseconds and strict >400. */
static inline bool rr_ui_pause_stamp(RRSession *session,double *paused_ms,double now_ms)
{
    if(!rr_session_pause(session))return false;
    *paused_ms=now_ms;return true;
}
/* Source pause() assigns these on every successful transition. Native lifecycle
 * additionally cancels a stale pointer drag across focus/minimize/overload. */
static inline bool rr_ui_pause_showcase(RRSession *session,double *paused_ms,double now_ms,RRShowcase *showcase)
{
    if(!showcase||!rr_ui_pause_stamp(session,paused_ms,now_ms))return false;
    showcase->yaw=.6;showcase->pitch=.14;showcase->dist=9.5;rr_showcase_up(showcase);return true;
}
static inline bool rr_ui_resume_key(RRSession *session,double paused_ms,double now_ms,bool requested,bool focused)
{
    if(!requested||!focused||!isfinite(now_ms)||!isfinite(paused_ms)||now_ms-paused_ms<=400)return false;
    return rr_session_resume(session);
}
#endif

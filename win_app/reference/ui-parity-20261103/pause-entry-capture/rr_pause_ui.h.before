#ifndef RR_PAUSE_UI_H
#define RR_PAUSE_UI_H
#include "rr_session.h"
/* main.js pause()/Escape handler uses monotonic milliseconds and strict >400. */
static inline bool rr_ui_pause_stamp(RRSession *session,double *paused_ms,double now_ms)
{
    if(!rr_session_pause(session))return false;
    *paused_ms=now_ms;return true;
}
static inline bool rr_ui_resume_key(RRSession *session,double paused_ms,double now_ms,bool requested,bool focused)
{
    if(!requested||!focused||!isfinite(now_ms)||!isfinite(paused_ms)||now_ms-paused_ms<=400)return false;
    return rr_session_resume(session);
}
#endif

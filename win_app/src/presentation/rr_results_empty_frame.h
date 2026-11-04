#ifndef RR_RESULTS_EMPTY_FRAME_H
#define RR_RESULTS_EMPTY_FRAME_H
#include "rr_results_slots_draw.h"
/* Explicit EMPTY-ACCOUNT source candidate, not inferred live service state.
 * Zero once per application, persistent address, never shallow-copy or reset on
 * race/menu changes: queued callbacks and internal owner/string pointers live
 * across transitions. No heap/cold resource ownership in this state. */
typedef struct RRResultsEmptyFrame {
 RRResultsTimer timer;RRResultsBinding identity;RRResultsFlowBinding flow;
 RRResultsListWork work;RRResultsRow row;RRResultsCaption caption;
 RRResultsNameStatus draw_status;bool draw_attempted,caption_ready;
} RRResultsEmptyFrame;
/* Call exactly once AFTER a completed current tick/UI transition with monotonic
 * wall seconds. A tick is submitted once; NULL for UI-only frames. Delegate
 * source queue/hide/pump/frozen identity capture order to existing bind_frame.
 * draw_attempted disambiguates draw_status on hidden/bind-error frames. Caption
 * is exposed only when caption_ready; stale stored bytes are not a paint request.
 * Failure preserves existing published numeric/identity data/queue per their
 * contracts; caller must inspect/report, never replay simulation/account work.
 * Inputs, state, font/texture metadata remain disjoint/stable synchronously.
 * ROWS only: headers/caption/card not painted; main/full-pack/rewards inactive.
 * Does not load textures/open audio/create graphics or change actual run phase. */
bool rr_results_empty_frame(RRResultsEmptyFrame *state,const RRRunCurrent *run,
 const RRRunEvents *tick,double wall_seconds,bool hide,RRResultsNameLookup names,
 void *names_context,RRUI *ui,RRResultsArt *art,double width,double height,
 double scroll,int device_scale,RRResultsPump *events);
#endif

#ifndef RR_CURRENT_EMPTY_HOST_H
#define RR_CURRENT_EMPTY_HOST_H
#include "rr_run_input.h"
#include "rr_results_empty_frame.h"
typedef struct RRCurrentEmptyRequest {
 RRRunInputFrame input;RRMouseSettings mouse;double dt,wall_seconds,width,height,scroll;
 int device_scale;bool simulate,hide;
} RRCurrentEmptyRequest;
typedef struct RRCurrentEmptyEvents {
 RRRunEvents tick;RRResultsPump results;
 bool tick_attempted,tick_ok,presentation_attempted,presentation_ok;
} RRCurrentEmptyEvents;
/* Candidate application boundary, explicit EMPTY-ACCOUNT ROW profile only.
 * simulate=true calls run_input_tick exactly once then submits its completed
 * event to results frame; simulate=false is UI-only (no input/aim/clock/RNG tick).
 * Simulation failure skips presentation and clears exposed per-frame draw flags.
 * A successful tick followed by paint
 * failure returns false BUT tick_ok remains true: do not replay that tick. Use
 * a later UI-only request to retry the frozen presentation instead. Existing
 * partial-failure/queue/clock latches remain authoritative, no rollback fiction.
 * Caller prepares/owns graph, normalized input, stable frame storage and loaded
 * resource metadata; source phase/transitions/reset remain caller responsibilities.
 * Borrowed const world/segment semantics preserved; NULL world means explicit
 * free space. No account/audio/graphics creation or production main activation. */
bool rr_current_empty_host(RRResultsEmptyFrame *results,RRRunInputState *input_state,
 RRRunCurrent *run,const RRCurrentEmptyRequest *request,const struct RRWorld *world,
 RRShotWorld segment,void *segment_context,RRResultsNameLookup names,void *names_context,
 RRUI *ui,RRResultsArt *art,RRCurrentEmptyEvents *events);
#endif

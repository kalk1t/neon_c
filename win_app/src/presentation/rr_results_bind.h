#ifndef RR_RESULTS_BIND_H
#define RR_RESULTS_BIND_H
#include "rr_results_ui.h"
enum {RR_RESULTS_NAME_BYTES=512};
typedef struct RRResultsIdentities {char names[RR_LOCAL_MAX_CARS][RR_RESULTS_NAME_BYTES];size_t count,revision;} RRResultsIdentities;
typedef enum RRResultsBindStatus {RR_RESULTS_BIND_IDLE,RR_RESULTS_BIND_READY,RR_RESULTS_BIND_IDENTITY_ERROR,RR_RESULTS_BIND_DRAW_ERROR,RR_RESULTS_BIND_OWNER_ERROR,RR_RESULTS_BIND_CORE_ERROR} RRResultsBindStatus;
/* Zero once beside its timer, preferably in persistent/caller-owned storage.
 * Two bounded name buffers allow atomic visible commit without a large stack
 * object or heap. Reinitializing a timer requires explicitly resetting binding.
 * Do not reset on race/menu transitions: old queued callbacks are preserved. */
typedef struct RRResultsBinding {const RRResultsTimer *owner;RRResultsIdentities buffers[2];size_t attempted_revision;unsigned active;bool ready;RRResultsBindStatus status;} RRResultsBinding;
typedef struct RRResultsBoundView {const RRResultsCurrent *results;const char *names[RR_LOCAL_MAX_CARS];size_t count,player,revision;} RRResultsBoundView;
/* Pure local metadata lookup, no network/account/progression operation. Return
 * a NUL-terminated UTF8 name, <512bytes, readable until the next lookup. Strings
 * are copied immediately; a shared transient return buffer is permitted.
 * Provider/run/roster must remain stable through the synchronous UI boundary. */
typedef const char *(*RRResultsNameLookup)(void *context,size_t index,const RRLocalRacer *racer);
/* Borrowed view/font-independent names valid until next publication/reset.
 * Consumer must not retain the stack view, mutate binding/timer/run/font data,
 * or resubmit a simulation tick if painting fails. */
typedef bool (*RRResultsBoundDraw)(void *context,const RRResultsBoundView *view);
bool rr_results_bind_view(const RRResultsBinding *binding,RRResultsBoundView *out);
/* Same queue/hide/pump order as rr_results_ui_frame. Capture identities only
 * when final visible publication revision changes, not at finish or repaint.
 * Due callbacks in a single boundary read the same pure local metadata; only
 * the final published display is exposed. Capture failure invalidates the new
 * view, never shows old names with new numbers, and is not retried until a new
 * publication. Draw failure keeps a valid frozen view available for repaint.
 * Current full-pack profile fixes player identity at roster0. No main activation. */
bool rr_results_bind_frame(RRResultsBinding *binding,RRResultsTimer *timer,
 const RRRunCurrent *run,const RRRunEvents *tick,double wall_seconds,bool hide,
 RRResultsNameLookup names,void *names_context,RRResultsBoundDraw draw,
 void *draw_context,RRResultsPump *events);
#endif

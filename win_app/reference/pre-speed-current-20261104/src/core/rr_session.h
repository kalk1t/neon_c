#ifndef RR_SESSION_H
#define RR_SESSION_H
#include <stddef.h>
#include "rr_race.h"
/* First three values retain the prototype's diagnostic mode numbers. */
typedef enum RRSessionPhase {
    RR_SESSION_MENU, RR_SESSION_ACTIVE, RR_SESSION_PAUSED,
    RR_SESSION_COUNTDOWN, RR_SESSION_FINISHED, RR_SESSION_RESULTS, RR_SESSION_ERROR
} RRSessionPhase;
enum { RR_SESSION_GO=8, RR_SESSION_SHOW_RESULTS=16 };
typedef struct RRSession {
    RRSessionPhase phase, resume_phase;
    RRRace race;
    const struct RRWorld *world; /* optional collision context; set after start */
    double race_time, countdown, finish_wait;
    RRCarStepReport motion;bool motion_stepped; /* Current call only; cleared even for paused/zero dt. */
    double *lap_times;
    size_t lap_count, lap_capacity;
    int last_count;
} RRSession;
void rr_session_init(RRSession *session);
void rr_session_dispose(RRSession *session);
void rr_session_start(RRSession *session, RRCar *car, RRRaceMode mode);
void rr_session_menu(RRSession *session);
bool rr_session_pause(RRSession *session);
bool rr_session_resume(RRSession *session);
void rr_session_respawn(const RRSession *session, RRCar *car);
/* raw_dt: monotonic frame elapsed; simulation caps it to browser 1/20 s.
 * Presentation's two-second results delay uses raw elapsed, not simulation dt.
 * Finished/results phases continue the supplied autopilot input and race clock;
 * lap/finish times remain frozen. Pause also supports these phases for native
 * focus/renderer safety. Menu/start cancel pending results synchronously.
 * Caller must not double-step car, lap clock or race clock. No online services.
 */
unsigned rr_session_step(RRSession *session, RRCar *car, const RRGate *gates,
                         int gate_count, RRInput input, double raw_dt);
/* Race HUD guidance continues after finish/results, as source targetOf does.
 * Menu/error and CSS-hidden free-roam guidance remain absent. */
bool rr_session_target(const RRSession *session, const RRGate *gates,
                       int gate_count, RRVec3 *target);
#endif

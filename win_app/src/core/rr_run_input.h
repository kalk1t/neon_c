#ifndef RR_RUN_INPUT_H
#define RR_RUN_INPUT_H
#include "rr_run_current.h"
#include "rr_input_mix.h"
#include "rr_mouse_aim.h"
/* Native-ready aggregate boundary, not OS polling. dx/dy are already eligible
 * accumulated mouse totals; do not apply a per-event jump filter here. Fire
 * hold and weapon/reset edges are platform-normalized. First pad supplied by
 * host; its axes/triggers are raw normalized values, before deadzone/mixing. */
typedef struct RRRunInputFrame {unsigned keys;RRInputPad pad;double dx,dy;bool captured,look,fire,missile,spin,emp,reset,reset_allowed,autopilot,center;} RRRunInputFrame;
typedef struct RRRunInputState {RRMouseAim aim;bool initialized,mouse_camera;} RRRunInputState;
/* Source centreAim resets yaw/pitch only, preserves direction/look history.
 * Host also drops external stale mouse movement at start/capture events. */
bool rr_run_input_center(RRRunInputState *state,const RRCar *car);
/* Source Input.state mix -> active updateAim or finished/debug autopilot ->
 * current clock/player/pack tick. Explicit center discards this frame's stale
 * totals. Respawn centers yaw/pitch AFTER tick, retaining premovement aimDir
 * used by source lock/fire; aimDir/look are not silently reset on GO. */
bool rr_run_input_tick(RRRunInputState *state,RRRunCurrent *run,RRRunInputFrame frame,
 RRMouseSettings settings,double raw_dt,const struct RRWorld *world,RRShotWorld segment,void *context,RRRunEvents *events);
#endif

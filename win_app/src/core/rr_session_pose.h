#ifndef RR_SESSION_POSE_H
#define RR_SESSION_POSE_H
#include "rr_session.h"
#include "rr_car_pose.h"
/* Adopt an initial or placed car. Preserve existing local bob/lift history.
 * Call after every host reset/recovery/manual respawn; not after dodge/contact.
 * This does not alter existing session start/reset semantics. */
bool rr_session_pose_place(RRCarPose *pose,const RRCar *car);
/* Explicit current solo player host. Session owns movement/clocks/rules/results;
 * pose samples pre-flight speed, then final moved state. Menu/pause advance only
 * car.t and local idle pose. Never call an additional flight/idle step that tick.
 * Error/invalid pose returns false: stop the host, do not render a stale fallback.
 * No renderer, account, rival, weapon or GPU activation. */
bool rr_session_pose_step(RRSession *session,RRCar *car,RRCarPose *pose,
 const RRGate *gates,int count,RRInput input,double raw_dt,unsigned *events);
#endif

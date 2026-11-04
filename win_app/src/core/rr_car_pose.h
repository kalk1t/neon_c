#ifndef RR_CAR_POSE_H
#define RR_CAR_POSE_H
#include "rr_fire.h"
/* Source root/model groups only: unit scale, default model Y translation and X
 * lift. Sculpted child fit/scale/pod transforms are separate. Historical root
 * is cached because contacts, damage and dodge do not call source Car.sync. */
typedef struct RRCarPose {double root[16],bob,lift;bool ready;} RRCarPose;
bool rr_car_pose_init(RRCarPose *pose,const RRCar *car);
/* Source Car.sync/Car.place: refresh root, preserve local bob/lift. */
bool rr_car_pose_sync(RRCarPose *pose,const RRCar *car);
/* Call once after current flight. speed_before is from BEFORE that flight,
 * as source sf is sampled before acceleration/braking; post-step speed is wrong.
 * If rules respawned a live car after flight, final car state is valid here:
 * place preserves t/nitro/kick used by local bob/lift and updates the root. */
bool rr_car_pose_after_step(RRCarPose *pose,const RRCar *car,double speed_before);
/* Source idle changes local transform only; caller already advanced car.t. */
bool rr_car_pose_idle(RRCarPose *pose,double time);
/* Pure column-major model.matrixWorld; no resync from current car values. */
bool rr_car_pose_matrix(const RRCarPose *pose,double matrix[16]);
bool rr_car_pose_binding(const RRCarPose *pose,int model,unsigned accent,RRFireBinding *binding);
/* False means invalid pose: stop/mark unqualified, never use it as fallback. */
#endif

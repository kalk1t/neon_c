#ifndef RR_FLIGHT_H
#define RR_FLIGHT_H

#include <stdbool.h>
#include "rr_math.h"

/* Frozen baseline entry points retain captured pre-v0.2.15 fixtures. */
#define RR_CAR_MAX 95.0
#define RR_CURRENT_CAR_MAX 135.0
#define RR_CURRENT_CAR_SCALE (RR_CURRENT_CAR_MAX/95.0)
#define RR_CURRENT_CAR_BOOST_MAX (138.0*RR_CURRENT_CAR_SCALE)
#define RR_CAR_BOOST_MAX 138.0
#define RR_STRAFE_SPEED 25.0
#define RR_STRAFE_RESPONSE 6.0
#define RR_STRAFE_LEAN (42.0*RR_PI/180.0)
#define RR_SPIN_ROLL 0.55
#define RR_SPIN_COOLDOWN 6.0

typedef struct RRInput { double throttle, brake, steer, climb; bool boost; double strafe; bool aim_steer; } RRInput;
typedef struct RRCar {
    RRVec3 pos, vel, fwd;
    double yaw, pitch, roll, speed, boost;
    double side, pod_pitch[4], pod_roll[4];
    double t, stun, roll_time, surge, max_multiplier;
    double thrust, nitro, kick, sputter, impact;
    int roll_direction;
    bool boosting;
} RRCar;

void rr_car_reset(RRCar *car, RRVec3 pos, double yaw, double speed);
/* Browser Car.place: respawn placement preserves boost and effect history. */
void rr_car_place(RRCar *car, RRVec3 pos, double yaw, double speed);
void rr_car_dodge(RRCar *car, int side);
typedef struct {double impact;RRVec3 normal;bool collided;} RRCarStepReport;
struct RRWorld;
void rr_car_step_world_report(RRCar *car,RRInput input,double dt,const struct RRWorld *world,RRCarStepReport *report);
void rr_car_step_world(RRCar *car, RRInput input, double dt,const struct RRWorld *world);
void rr_car_step(RRCar *car, RRInput input, double dt);
static inline double rr_car_display_speed(const RRCar *car){return hypot(car->speed,car->side);}

/* Explicit v0.2.15 flight; same collision path and state, acceleration also
 * scales with max_multiplier. No gameplay feature activation or network effects. */
void rr_car_step_current_world_report(RRCar *car,RRInput input,double dt,const struct RRWorld *world,RRCarStepReport *report);
void rr_car_step_current(RRCar *car,RRInput input,double dt);
void rr_car_dodge_current(RRCar *car,int side);
double rr_frame_dt(double elapsed);

#endif

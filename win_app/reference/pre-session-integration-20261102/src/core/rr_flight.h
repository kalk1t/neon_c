#ifndef RR_FLIGHT_H
#define RR_FLIGHT_H

#include <stdbool.h>
#include "rr_math.h"

#define RR_CAR_MAX 95.0
#define RR_CAR_BOOST_MAX 138.0
#define RR_SPIN_ROLL 0.55
#define RR_SPIN_COOLDOWN 6.0

typedef struct RRInput { double throttle, brake, steer, climb; bool boost; } RRInput;
typedef struct RRCar {
    RRVec3 pos, vel, fwd;
    double yaw, pitch, roll, speed, boost;
    double t, stun, roll_time, surge, max_multiplier;
    double thrust, nitro, kick, sputter;
    int roll_direction;
    bool boosting;
} RRCar;

void rr_car_reset(RRCar *car, RRVec3 pos, double yaw, double speed);
void rr_car_dodge(RRCar *car, int side);
void rr_car_step(RRCar *car, RRInput input, double dt);
double rr_frame_dt(double elapsed);

#endif

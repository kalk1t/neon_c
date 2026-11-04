#ifndef RR_ARSENAL_H
#define RR_ARSENAL_H
#include <stdbool.h>
/* Offline playerWeapons resource/trigger stage only. Host must gate race/alive,
 * resolve edge-triggered input, and apply outputs between ranking and projectiles.
 * dt is the source frame dt (finite, nonnegative). RNG returns [0,1).
 * No projectile, dodge motion, EMP effects, audio or account operation is applied. */
typedef struct {
    double heat, bolt_cd, missiles, spin_cd, emp_cd;
    bool overheated;
    int side;
} RRArsenal;
typedef struct {
    bool fire, missile_hit, spin_hit, emp_hit, has_lock;
    double steer, lock_charge;
} RRArsenalInput;
typedef double (*RRArsenalRandom)(void *context);
enum { RR_ARSENAL_BOLT=1, RR_ARSENAL_OVERHEAT, RR_ARSENAL_MISSILE,
       RR_ARSENAL_DENY, RR_ARSENAL_SPIN, RR_ARSENAL_EMP };
typedef struct {
    unsigned count, order[6], random_draws;
    int bolt_side, spin_side;
    bool missile_homing;
    double spread_x, spread_y;
} RRArsenalEvents;
typedef void (*RRArsenalNotice)(void *context,unsigned event,const RRArsenalEvents *events);
/* Synchronous source-order dispatch: callbacks may consume the same RNG stream
 * before subsequent arsenal draws. Do not reenter or mutate arsenal state. */
void rr_arsenal_step_dispatch(RRArsenal *state,double dt,RRArsenalInput input,
    RRArsenalRandom random,void *random_context,RRArsenalNotice notice,void *notice_context,RRArsenalEvents *events);
void rr_arsenal_init(RRArsenal *state);
/* startRace resets resources but source g.side persists across race restarts. */
void rr_arsenal_restart(RRArsenal *state);
/* rng must be supplied when firing a bolt or performing an unsteered spin. */
void rr_arsenal_step(RRArsenal *state, double dt, RRArsenalInput input,
                     RRArsenalRandom rng, void *context, RRArsenalEvents *events);
#endif

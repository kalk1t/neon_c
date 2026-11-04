#ifndef RR_TARGETING_H
#define RR_TARGETING_H
#include "rr_ai.h"
typedef struct RRLocalTarget {RRVec3 pos,vel;bool member,alive,finished;} RRLocalTarget;
typedef struct RRLocalLock {int target;double charge;} RRLocalLock;
enum {RR_LOCK_ACQUIRE=1,RR_LOCK_READY=2};
void rr_lock_init(RRLocalLock *state);
bool rr_target_angle(RRVec3 player,RRVec3 aim,const RRLocalTarget *target,double max_distance,double *angle);
/* Invalid acquisition angles use -1. Candidates retain stable local roster order. */
unsigned rr_lock_angles(RRLocalLock *state,const double *acquire,int count,
    bool keep_valid,double keep_angle,double dt);
unsigned rr_lock_update(RRLocalLock *state,RRVec3 player,RRVec3 aim,
    const RRLocalTarget *targets,int count,double dt);
/* Frozen earlier API uses400/460; current rules call this shared range form. */
unsigned rr_lock_update_ranges(RRLocalLock *state,RRVec3 player,RRVec3 aim,const RRLocalTarget *targets,int count,double dt,double acquire_range,double keep_range);
bool rr_target_lead(RRVec3 from,const RRLocalTarget *target,double speed,RRVec3 *direction);
#endif

#ifndef RR_RIVAL_CURRENT_H
#define RR_RIVAL_CURRENT_H
#include "rr_fire.h"
/* Explicit v0.2.15 resources, separate from legacy pack/rival fixtures. */
typedef struct RRRivalCurrent {double heat,bolt_cd;int side;bool overheated;} RRRivalCurrent;
/* Caller supplies source reset's random()*0.5; side persists as in Racer.reset.
 * This consumes no RNG and does not reset unrelated racer/progression state. */
void rr_rival_current_reset(RRRivalCurrent *state,double initial_bolt_cd);
/* Read-only source zap notification, after heat update; no simulation RNG/reentry. */
typedef void (*RRRivalCurrentCue)(void *context,const RRLocalRacer *rival);
/* Returns false only for invalid input/storage error, not for a skipped shot.
 * Exact post-movement model-group matrix/paint supplied by host. Source guards
 * leave all resource timers frozen before6s, outside race, dead or finished.
 * Successful output fired reports one source nonhoming bolt; no muzzle flash.
 * Warm Three mesh UUID draws follow three spread draws. Cold allocation/audio,
 * full pack and runtime rendering remain caller-owned/unqualified. */
bool rr_rival_current_step(RRFireLocal *local,RRLocalRacer *rival,RRRivalCurrent *state,
 double dt,double race_time,bool race_active,double road_fire,const RRFireBinding *binding,
 RRRivalCurrentCue cue,void *cue_context,bool *fired);
#endif

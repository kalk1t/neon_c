#ifndef RR_RIVAL_LIFECYCLE_H
#define RR_RIVAL_LIFECYCLE_H
#include "rr_ai_current.h"
#include "rr_rival_current.h"
/* Current Racer.reset fields, not an activated weapon state machine. Zero before
 * first use; bolt side persists across reset, matching source. Respawn/drive do
 * not receive this state, so ordinary recovery cannot refill weapons. */
typedef struct RRRivalLifeResources {RRRivalCurrent bolt;double missiles,seeker_cd,spin_cd,emp_cd;RRLocalRacer *locked;double lock_time;} RRRivalLifeResources;
bool rr_rival_lifecycle_reset(RRLocalRacer *rival,RRRivalLifeResources *resources,const RRTrack *track,RRVec3 position,double yaw,RRRaceMode mode,double random_bolt,double random_reaction);
/* Current per-rival boundary, called in roster order after the player: reaction
 * gate -> current AI (even if wrecked) -> step/45m rules/recovery -> smoke event.
 * Caller owns countdown/pause, supplied peers/target/coin and RNG/FX/combat tail.
 * Smoke must dispatch before that rival's combat. No network/account calls. */
bool rr_rival_lifecycle_drive(RRLocalRacer *rival,const RRTrack *track,const RRAICorners *corners,const struct RRWorld *world,const RRAIPeer *peers,size_t count,RRVec3 target,const RRVec3 *coin,double time,double dt,double race_time,double floor_height,RRRivalRandom random,void *context,RRInput *input,unsigned *events);
#endif

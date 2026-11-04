#ifndef RR_PACK_CURRENT_H
#define RR_PACK_CURRENT_H
#include "rr_pack.h"
#include "rr_rival_lifecycle.h"
/* Zero once; borrowed pack/player/track. Re-start preserves cannon sides and
 * constructor history, resets source race/weapon resources and prepares cache.
 * No production activation/account/reward/network calls. */
typedef struct RRPackCurrent {RRLocalPack *pack;RRAICorners corners;RRRivalLifeResources resources[RR_AI_RIVALS];} RRPackCurrent;
bool rr_pack_current_start(RRPackCurrent *current,RRLocalPack *pack,const RRTrack *track,RRLocalRacer *player,RRPackRandom random,void *context);
bool rr_pack_current_rank(RRPackCurrent *current);
/* Current phase1: sequential current rival AI/flight/rules/smoke hook, contacts,
 * ranking. Hook is synchronous before next rival: a qualified source combat
 * adapter may mutate scalar car/racer/weapon fields in place, but never resize,
 * replace or reenter the roster/storage/pack. Source FX/RNG/order is caller-owned.
 * Countdown uses current idle flight; paused has no tail. Other branches owe
 * exactly one rr_pack_coins_dispatch after player weapons/projectiles, even idle.
 * Combat-free fixture may use a read-only observer; no arsenal is implied. */
bool rr_pack_current_rivals_dispatch(RRPackCurrent *current,RRPackBranch branch,double dt,double time,double race_time,const struct RRWorld *world,RRPackRandom random,void *random_context,RRPackRivalNotice notice,void *notice_context,RRPackEvents *events);
#endif

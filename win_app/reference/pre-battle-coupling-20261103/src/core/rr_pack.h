#ifndef RR_PACK_H
#define RR_PACK_H
#include "rr_coins.h"
typedef enum RRPackBranch {RR_PACK_IDLE,RR_PACK_COUNTDOWN,RR_PACK_PLAYING,RR_PACK_PAUSED} RRPackBranch;
typedef bool (*RRPackRandom)(void *context,int racer_index,double values[2]);
typedef struct RRLocalPack {
    RRLocalRacer rivals[RR_AI_RIVALS];RRLocalRacer *player;const RRTrack *track;
    RRCoins coins;int ranking[RR_LOCAL_MAX_CARS],position,finish_position,previous_player_sample;
    double rank_timer,shake;bool previous_player_finished;
    bool coin_pending;RRPackBranch pending_branch;double pending_dt;
} RRLocalPack;
typedef struct RRPackEvents {unsigned rival[RR_AI_RIVALS];RRCarContacts contacts;RRCoinEvents coins;bool ranked,error;} RRPackEvents;
/* Zero-init once; caller prepares persistent rival car/driver constructor state.
 * Player is borrowed and already reset by its owner. Dispose never frees it. */
bool rr_pack_grid(int slot,RRVec3 *out);
bool rr_pack_start(RRLocalPack *pack,const RRTrack *track,RRLocalRacer *player,RRPackRandom random,void *context);
void rr_pack_dispose(RRLocalPack *pack);
void rr_pack_rank(RRLocalPack *pack);
/* Player/clock/phase are caller-owned. Stage1 advances rivals, contacts and
 * ranking; source lock/weapons/projectile work belongs between stages. Stage2
 * reads the resulting racer eligibility/positions and advances coins once.
 * A pending tail must be consumed before another stage1. Paused has no tail. */
bool rr_pack_rivals_after_player(RRLocalPack *pack,RRPackBranch branch,double dt,
    double time,double race_time,const struct RRWorld *world,RRPackRandom random,
    void *context,RRPackEvents *events);
bool rr_pack_coins_after_weapons(RRLocalPack *pack,RRCoinEvents *events);
/* Convenience combat-free projection used by existing fixtures, never enabled
 * in production. Real stateful weapons must use the two explicit phases. */
bool rr_pack_tick_after_player(RRLocalPack *pack,RRPackBranch branch,double dt,
    double time,double race_time,const struct RRWorld *world,RRPackRandom random,
    void *context,RRPackEvents *events);
#endif

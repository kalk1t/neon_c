#ifndef RR_PACK_H
#define RR_PACK_H
#include "rr_coins.h"
typedef enum RRPackBranch {RR_PACK_IDLE,RR_PACK_COUNTDOWN,RR_PACK_PLAYING,RR_PACK_PAUSED} RRPackBranch;
typedef bool (*RRPackRandom)(void *context,int racer_index,double values[2]);
typedef struct RRLocalPack {
    RRLocalRacer rivals[RR_AI_RIVALS];RRLocalRacer *player;const RRTrack *track;
    RRCoins coins;int ranking[RR_LOCAL_MAX_CARS],position,finish_position,previous_player_sample;
    double rank_timer,shake;bool previous_player_finished;
} RRLocalPack;
typedef struct RRPackEvents {unsigned rival[RR_AI_RIVALS];RRCarContacts contacts;RRCoinEvents coins;bool ranked,error;} RRPackEvents;
/* Zero-init once; caller prepares persistent rival car/driver constructor state.
 * Player is borrowed and already reset by its owner. Dispose never frees it. */
bool rr_pack_grid(int slot,RRVec3 *out);
bool rr_pack_start(RRLocalPack *pack,const RRTrack *track,RRLocalRacer *player,RRPackRandom random,void *context);
void rr_pack_dispose(RRLocalPack *pack);
void rr_pack_rank(RRLocalPack *pack);
/* AFTER caller's player phase (physics/rules/FX), BEFORE source weapons/FX update.
 * branch is the branch entered by source frame(), including the final idle GO
 * countdown tick. Caller owns clocks, countdown/pause transitions and player UI.
 * No combat/network/account operations; disabled in production. */
bool rr_pack_tick_after_player(RRLocalPack *pack,RRPackBranch branch,double dt,
    double time,double race_time,const struct RRWorld *world,RRPackRandom random,
    void *context,RRPackEvents *events);
#endif

#ifndef RR_COINS_H
#define RR_COINS_H
#include "rr_rival.h"
#define RR_COINS_PER_GAP 6
#define RR_COIN_MAX ((RR_TRACK_MAX_GATES+1)*RR_COINS_PER_GAP)
typedef struct RRLocalCoin {RRVec3 pos;int sample;bool active;double timer,pop;} RRLocalCoin;
typedef struct RRCoins {RRLocalCoin list[RR_COIN_MAX];int count;bool hidden;} RRCoins;
typedef struct RRCoinCollection {int coin,racer;} RRCoinCollection;
typedef struct RRCoinEvents {RRCoinCollection list[RR_COIN_MAX];int count;} RRCoinEvents;
/* Local simulation only: no rendering/assets, wallet/account or progression. */
bool rr_coins_init(RRCoins *coins,const RRTrack *track);
void rr_coins_reset(RRCoins *coins);
void rr_coins_visible(RRCoins *coins,bool visible);
/* -1 if no target. Source ties choose the later list entry; hidden is visual only. */
int rr_coins_ahead(const RRCoins *coins,const RRCar *car,double max_distance);
typedef void (*RRCoinNotice)(void *context,const RRCoinCollection *event);
/* Observer runs immediately after award, before the next coin. No roster/coin mutation. */
bool rr_coins_update_dispatch(RRCoins *coins,double dt,RRLocalRacer *racers,size_t count,
    bool collection_active,RRCoinNotice notice,void *context,RRCoinEvents *events);
bool rr_coins_update(RRCoins *coins,double dt,RRLocalRacer *racers,size_t count,
    bool collection_active,RRCoinEvents *events);
#endif

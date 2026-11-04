#ifndef RR_BATTLE_CURRENT_H
#define RR_BATTLE_CURRENT_H
#include "rr_battle.h"
#include "rr_pack_current.h"
#include "rr_rival_arsenal.h"
/* Supply the exact current source model-group matrix/paint at this boundary;
 * no GPU/pose generation is inferred. Index0 fetched before contacts; rival
 * indices1..10 fetched after their movement/smoke, before their arsenal. */
typedef bool (*RRBattleBinding)(void *context,int index,const RRLocalRacer *racer,RRFireBinding *binding);
typedef struct RRBattleCurrent {RRLocalBattle *battle;RRPackCurrent pack;RRBattleBinding binding;void *binding_context;RRRivalArsenalCue rival_cue;void *rival_cue_context;bool error;} RRBattleCurrent;
/* Zero once; call rr_arsenal_init(&battle->arsenal) once for source cannon side1.
 * Caller owns battle/pack/player/storage/random/cues. Resources and
 * shot clear mirror source reset; FX and cannon sides persist. No account calls. */
bool rr_battle_current_start(RRBattleCurrent *current,RRLocalBattle *battle,RRLocalPack *pack,const RRTrack *track,RRLocalRacer *player);
/* Explicit post-player current graph, not production activation. Native player
 * movement/clock/respawn supplied. Null world means qualified free-space only.
 * Invalid binding/capacity latches error; stop, do not reuse a failed run. */
bool rr_battle_current_after_player(RRBattleCurrent *current,RRPackBranch branch,bool race_active,double dt,double time,double race_time,const struct RRWorld *world,RRShotWorld segment,void *segment_context,RRArsenalInput input,RRPackEvents *events);
#endif

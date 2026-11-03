#ifndef RR_BATTLE_H
#define RR_BATTLE_H
#include "rr_pack.h"
#include "rr_fire.h"
typedef struct {
    RRLocalPack *pack;RRFireLocal fire;RRLocalLock lock;RRArsenal arsenal;
    RRLocalRacer *roster[RR_LOCAL_MAX_CARS];
} RRLocalBattle;
/* Numeric source Car.sync YXZ muzzle pose at this exact point in the frame. */
RRVec3 rr_combat_muzzle(const RRCar *car);
/* Source rivalCombat selection/fireCD; warm allocation/random timing matches fire
 * profile. No rewards, live audio, account or production activation. */
bool rr_battle_rival_fire(RRFireLocal *local,RRLocalRacer *rival,double dt,
    double race_time,bool race_active,double road_fire);
/* Borrow pack/racers; caller supplies owned shot/FX storage and shared random/cue
 * callbacks in battle.fire. Reset clears weapons/lock/resources, preserves FX and
 * alternating cannon side. Pack/player reset and clocks remain caller-owned. */
void rr_battle_bind(RRLocalBattle *battle,RRLocalPack *pack);
void rr_battle_restart(RRLocalBattle *battle);
/* Post-player boundary: sequential rival step/smoke/fire -> contacts/rank ->
 * player lock/weapons -> projectile damage/FX aging -> synchronous coin FX.
 * race_active is source g.state==='race' after supplied player phase. Muzzle,
 * side_axis and aim must be supplied from that phase (contacts do not resync
 * source player model). No player movement/input/session/respawn is inferred. */
bool rr_battle_after_player(RRLocalBattle *battle,RRPackBranch branch,bool race_active,
    double dt,double time,double race_time,const struct RRWorld *world,
    RRShotWorld segment,void *segment_context,RRArsenalInput input,RRPackEvents *events);
#endif

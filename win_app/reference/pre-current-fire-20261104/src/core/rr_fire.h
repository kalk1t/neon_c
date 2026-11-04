#ifndef RR_FIRE_H
#define RR_FIRE_H
#include "rr_arsenal.h"
#include "rr_combat.h"
#include "rr_targeting.h"
typedef struct {
    double speed,accel,max_speed,life,turn,damage,length,width,radius;
    bool has_speed,has_life,has_turn,has_damage,always,trail;
    RRShotKind kind;
} RRFireOptions;
typedef struct {RRShot shot;double length,width;} RRFireInit;
RRFireInit rr_fire_init(RRLocalRacer *owner,RRVec3 pos,RRVec3 dir,double inherit,
                        RRLocalRacer *target,RRFireOptions options);
typedef struct {double t,life;} RRFxLife;
typedef struct {RRFxLife *items;size_t count,capacity;bool error,three_warm_uuids;} RRFxLedger;
enum {RR_FX_FLASH,RR_FX_IMPACT,RR_FX_SPARK,RR_FX_WAVE,RR_FX_EXPLODE,RR_FX_SMOKE,RR_FX_TRAIL};
/* Numerical FX lifetime/RNG accounting only. Caller storage must cover requests;
 * shortage returns false and latches error, never silently substitutes a cap.
 * This is experimental offline storage, not a production budget allocation.
 * three_warm_uuids models pinned Three cached-texture/shared-Sprite-geometry
 * allocations:8 UUID draws/sprite,16/wave. Initialization/loading not covered. */
bool rr_fx_request(RRFxLedger *fx,unsigned kind,RRArsenalRandom random,void *ctx);
void rr_fx_tick(RRFxLedger *fx,double dt);
/* Audio calls occur in source order. Host owns audio-dependent RNG (including
 * unconditional Sound.blaster random draw). No silent RNG substitute is supplied. */
typedef void (*RRFireCue)(void *ctx,unsigned arsenal_event);
typedef struct {
    RRLocalRacer *player,*locked;
    RRLocalRacer **racers;size_t racer_count;
    RRVec3 muzzle,side_axis,aim;
    RRShot *shots;size_t count,capacity;
    RRFxLedger fx;
    RRArsenalRandom random;void *random_ctx;
    RRFireCue cue;void *cue_ctx;
    bool error;
    double *shake; /* Optional shared source camera shake state; no rendering. */
} RRFireLocal;
/* Source player weapon order. Call after lock/ranking and only when race/alive.
 * Muzzle/quaternion/aim supplied by host; rewards/account/network never applied.
 * Shot/FX capacity failure latches error; do not treat that run as qualified. */
void rr_fire_player_step(RRFireLocal *local,RRArsenal *arsenal,double dt,RRArsenalInput input,RRArsenalEvents *events);
/* Reverse projectile dispatch -> impact/damage/wreck -> FX aging. Caller then
 * consumes existing late-coin tail. No structurally mutating callbacks allowed. */
void rr_fire_projectiles_tick(RRFireLocal *local,double dt,double floor_height,RRShotWorld world,void *ctx);
/* Source Weapons.clear leaves existing FX alive; resource restart is separate. */
void rr_fire_clear(RRFireLocal *local);
#endif

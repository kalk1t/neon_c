#ifndef RR_WEAPON_CURRENT_H
#define RR_WEAPON_CURRENT_H
#include "rr_fire.h"
/* Captured current v0.2.14 offline rules. No network/production activation. */
typedef struct RRWeaponRules {double damage,period,speed,life,heat,acquire,keep,length,width;int source_protocol;} RRWeaponRules;
const RRWeaponRules *rr_weapon_current_rules(void);
void rr_weapon_current_arsenal(RRArsenal *s,double dt,RRArsenalInput input,RRArsenalRandom random,void *ctx,RRArsenalEvents *events);
void rr_weapon_current_arsenal_dispatch(RRArsenal *s,double dt,RRArsenalInput input,RRArsenalRandom random,void *ctx,RRArsenalNotice notice,void *notice_ctx,RRArsenalEvents *events);
unsigned rr_weapon_current_lock(RRLocalLock *s,RRVec3 player,RRVec3 aim,const RRLocalTarget *targets,int count,double dt);
RRFireOptions rr_weapon_current_bolt_options(void);
/* 0=procedural fallback,1=Roadster,2=Viper,3=Titan; does not grant/select a car. */
RRVec3 rr_weapon_current_muzzle(int model,int side,const double model_world[16]);
RRVec3 rr_weapon_current_color(unsigned rgb);
typedef struct RRBoltStreak {RRVec3 center;double flown,length,width;} RRBoltStreak;
RRBoltStreak rr_weapon_current_streak(RRVec3 previous,RRVec3 head,RRVec3 velocity,RRVec3 camera,double flown);
#endif

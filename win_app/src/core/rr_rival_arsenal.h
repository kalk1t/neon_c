#ifndef RR_RIVAL_ARSENAL_H
#define RR_RIVAL_ARSENAL_H
#include "rr_rival_lifecycle.h"
enum {RR_RIVAL_CUE_BOLT=1,RR_RIVAL_CUE_SEEKER,RR_RIVAL_CUE_EMP,RR_RIVAL_NOTICE_SEEKER,RR_RIVAL_NOTICE_EMP};
/* Read-only ordered audio/HUD notification; no simulation RNG, reentry or roster
 * mutation. Sound.zap has no RNG in the qualified null-audio-context profile. */
typedef void (*RRRivalArsenalCue)(void *context,unsigned event,RRLocalRacer *owner,RRLocalRacer *target);
/* Explicit v0.2.16 local arsenal. Caller supplies pre-weapon source model matrix
 * and owner paint, roster/storage/shared RNG, dt/phase; no pose resync invented
 * after dodge. Full pack, cold graphics/audio and rewards remain separate.
 * Validate true return AND no local error before continuing a fixture/run. */
bool rr_rival_arsenal_step(RRFireLocal *local,RRLocalRacer *rival,RRRivalLifeResources *state,double dt,double race_time,bool race_active,double road_fire,const RRFireBinding *binding,RRRivalArsenalCue cue,void *cue_context);
/* v0.2.17 profile EMP radius/victim reaction plus source spin/charge limits.
 * Normal production-design rivals remain Roadster; alternative profiles in tests
 * are isolated fixtures, not an unlocked roster or activation. */
bool rr_rival_arsenal_v217_step(RRFireLocal *local,RRLocalRacer *rival,RRRivalLifeResources *state,double dt,double race_time,bool race_active,double road_fire,const RRFireBinding *binding,RRRivalArsenalCue cue,void *cue_context);
#endif

#ifndef RR_RIVAL_H
#define RR_RIVAL_H
#include "rr_ai.h"
#include "rr_race.h"
#define RR_LOCAL_MAX_CARS (RR_AI_RIVALS+1)
#define RR_LOCAL_MAX_CONTACTS 55
typedef struct RRLocalRacer {
    RRCar car;RRAIDriver ai;RRRace race;RRVec3 previous;
    double hp,wreck,lost,stuck,wrong_way,smoke_time,fire_cd,react;
    int takedowns,coins;uint32_t passed;
    double *lap_times;size_t lap_count,lap_capacity;
    bool is_player,visible,error;
} RRLocalRacer;
typedef struct RRCarContactEvent {int a,b;double relative_speed,hit_volume;bool player_hit;} RRCarContactEvent;
typedef struct RRCarContacts {RRCarContactEvent events[RR_LOCAL_MAX_CONTACTS];size_t count;double shake;} RRCarContacts;
enum {RR_RIVAL_RESPAWN=8,RR_RIVAL_SMOKE=16,RR_RIVAL_ERROR=32};
/* Zero-initialize before first use. These functions simulate local rivals only:
 * no roam player spawn, networking, damage/rewards, storage or model calls. */
typedef bool (*RRRivalRandom)(void *context,double values[2]);
/* Callback is consumed only when a respawn actually occurs. */
unsigned rr_rival_step_random(RRLocalRacer *r,const RRTrack *track,const struct RRWorld *world,
    RRInput input,double dt,double race_time,double floor_height,RRRivalRandom random,void *context);
void rr_rival_dispose(RRLocalRacer *r);
bool rr_rival_reset(RRLocalRacer *r,const RRTrack *track,RRVec3 position,double yaw,
    RRRaceMode mode,double random_fire,double random_reaction);
bool rr_rival_respawn(RRLocalRacer *r,const RRTrack *track,bool keep_hp,double random_x,double random_y);
unsigned rr_rival_rules(RRLocalRacer *r,const RRTrack *track,double dt,double race_time,
    double floor_height,double random_x,double random_y);
unsigned rr_rival_step(RRLocalRacer *r,const RRTrack *track,const struct RRWorld *world,
    RRInput input,double dt,double race_time,double floor_height,double random_x,double random_y);
/* Same local racer pair order as main.js; finished-but-alive cars still collide.
 * Output events expose numerical sound/shake cues without playing audio. */
bool rr_local_car_contacts(RRLocalRacer *racers,size_t count,double shake,RRCarContacts *out);
/* Working-source per-car mass, same pair order/guards/thresholds/shake/sound.
 * Current pack calls this path. No health/speed damage, RNG or pose resync.
 * Legacy rr_local_car_contacts retains equal-mass frozen reference behavior. */
bool rr_local_car_contacts_current(RRLocalRacer *racers,size_t count,double shake,RRCarContacts *out);
/* Explicit current flight + strict45m rival gate rules; source respawn/smoke
 * order unchanged. No weapon resources reset on respawn. */
unsigned rr_rival_step_current_random(RRLocalRacer *r,const RRTrack *track,const struct RRWorld *world,RRInput input,double dt,double race_time,double floor_height,RRRivalRandom random,void *context);
unsigned rr_rival_rules_current(RRLocalRacer *r,const RRTrack *track,double dt,double race_time,double floor_height,double random_x,double random_y);
#endif

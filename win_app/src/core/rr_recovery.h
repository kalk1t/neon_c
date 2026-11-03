#ifndef RR_RECOVERY_H
#define RR_RECOVERY_H
#include "rr_track.h"
typedef struct RRRecovery { int sample; double wrong_way,lost_time; } RRRecovery;
enum { RR_RECOVERY_WRONG_WAY=1, RR_RECOVERY_SIGNAL_LOST=2, RR_RECOVERY_RESPAWN=4 };
/* Geometry-free browser player warning/recovery conditions. dt is the same
 * clamped simulation delta used by flight/rules. Respawn emits once and clears
 * lost_time; caller places the car and rebases its nearest sample afterward.
 */
unsigned rr_recovery_conditions(RRRecovery *state,double along,double off,
    double height,double floor_height,double dt,double message_time,bool roam);
void rr_recovery_init(RRRecovery *state,const RRTrack *track,RRVec3 pos);
void rr_recovery_rebase(RRRecovery *state,const RRTrack *track,RRVec3 pos);
unsigned rr_recovery_update(RRRecovery *state,const RRTrack *track,RRVec3 pos,
    RRVec3 velocity,double floor_height,double dt,double message_time,bool roam);
#endif

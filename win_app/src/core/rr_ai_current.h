#ifndef RR_AI_CURRENT_H
#define RR_AI_CURRENT_H
#include "rr_ai.h"
/* Caller-owned counterpart of browser WeakMap<Track,Float32Array>. Prepare after
 * every track construction/rebuild, including reuse of the same RRTrack pointer.
 * Immutable during controls; no hidden lazy allocation or stale global cache. */
typedef struct RRAICorners {const RRTrack *track;int count;double length;float speed[RR_TRACK_MAX_SAMPLES];} RRAICorners;
bool rr_ai_current_prepare(RRAICorners *corners,const RRTrack *track);
double rr_ai_current_corner_limit(const RRAICorners *corners,int sample,double speed);
/* Explicit current source controller. Peers exclude self, retain iteration order.
 * Borrow prepared route cache; no car step/race rules/combat/account side effects. */
bool rr_ai_current_controls(RRAIDriver *driver,const RRCar *car,const RRTrack *track,
 const RRAICorners *corners,const RRAIPeer *peers,size_t count,RRVec3 target,
 double time,double dt,const RRVec3 *coin,RRInput *out);
/* Current road speed and ranking catch-up; invalid inputs return NaN. This does
 * not rank racers, mutate progress or unlock a road. Ahead gets no slowdown. */
double rr_ai_current_top(int road_id);
double rr_ai_current_speed_factor(double player_progress,double rival_progress,int samples,int road_id);
#endif

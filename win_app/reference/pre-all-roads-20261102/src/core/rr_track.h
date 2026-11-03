#ifndef RR_TRACK_H
#define RR_TRACK_H

#include <stdbool.h>
#include "rr_math.h"

#define RR_TRACK_MAX_SAMPLES 4096
#define RR_TRACK_ARC_DIVISIONS 200
#define RR_TRACK_GATES 8

typedef struct RRGate { RRVec3 pos, forward; double yaw; } RRGate;
typedef struct RRTrack {
    double arcs[RR_TRACK_ARC_DIVISIONS+1], length;
    int sample_count;
    RRVec3 samples[RR_TRACK_MAX_SAMPLES];
    RRVec3 tangents[RR_TRACK_MAX_SAMPLES];
    RRGate gates[RR_TRACK_GATES];
} RRTrack;

RRVec3 rr_track_point(double t);
RRVec3 rr_track_tangent(double t);
RRVec3 rr_track_point_at(const RRTrack *track, double u);
RRVec3 rr_track_tangent_at(const RRTrack *track, double u);
bool rr_track_init(RRTrack *track);
int rr_track_nearest(const RRTrack *track, RRVec3 pos);
int rr_track_nearest_local(const RRTrack *track, RRVec3 pos, int hint);
bool rr_gate_crossed(const RRGate *gate, RRVec3 previous, RRVec3 current, double radius);

#endif

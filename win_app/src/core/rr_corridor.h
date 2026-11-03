#ifndef RR_CORRIDOR_H
#define RR_CORRIDOR_H
#include "rr_track.h"
enum {
    RR_CORRIDOR_CELL=160,
    RR_CORRIDOR_MAX_POINTS=RR_ROAD_COUNT*RR_TRACK_MAX_SAMPLES,
    RR_CORRIDOR_MAX_COARSE=RR_ROAD_COUNT*((RR_TRACK_MAX_SAMPLES+4)/5),
    RR_CORRIDOR_MAX_GATES=RR_ROAD_COUNT*RR_TRACK_MAX_GATES,
    RR_CORRIDOR_GRID_SIDE=64,
    RR_CORRIDOR_GRID_CELLS=RR_CORRIDOR_GRID_SIDE*RR_CORRIDOR_GRID_SIDE
};
typedef struct RRCorridor {
    RRVec3 points[RR_CORRIDOR_MAX_POINTS],coarse[RR_CORRIDOR_MAX_COARSE];
    RRGate gates[RR_CORRIDOR_MAX_GATES];
    RRVec3 minimum,maximum;
    int point_count,coarse_count,gate_count,occupied_cells;
    int cell_min_x,cell_min_z,cell_width,cell_height;
    int heads[RR_CORRIDOR_GRID_CELLS],next[RR_CORRIDOR_MAX_COARSE];
} RRCorridor;
typedef struct RRNearestXZ {double distance,height;} RRNearestXZ;
bool rr_corridor_init_all(RRCorridor *corridor,RRTrack *scratch);
/* Bounded analytical fixture/geometry adapter; full points equal coarse points. */
bool rr_corridor_init_points(RRCorridor *corridor,const RRVec3 *coarse,int count,const RRGate *gates,int gate_count);
RRNearestXZ rr_corridor_nearest_xz(const RRCorridor *corridor,double x,double z);
bool rr_corridor_clear(const RRCorridor *corridor,double x,double z,double radius,double top,double margin);
#endif

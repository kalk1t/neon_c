#ifndef RR_ROAD_PICKER_H
#define RR_ROAD_PICKER_H
#include "rr_session.h"
typedef struct RRRoadRow { double length,best_lap;int gates; } RRRoadRow;
typedef struct RRRoadPicker {
    bool open;
    int current_road,focus_road,open_count;
    RRRaceMode mode;
    RRRoadRow rows[RR_ROAD_COUNT];
} RRRoadPicker;
/* All progress inputs are supplied by an eventual platform/profile adapter.
 * No storage/identity calls, unlock grants or user-data writes occur here.
 */
int rr_roads_open(double local_open,int profile_roads_won);
bool rr_road_picker_init(RRRoadPicker *picker,RRTrack *scratch);
void rr_road_picker_open(RRRoadPicker *picker,RRRaceMode mode,double local_open,int profile_roads_won);
void rr_road_picker_close(RRRoadPicker *picker);
void rr_road_picker_move(RRRoadPicker *picker,int direction);
bool rr_road_picker_choose(RRRoadPicker *picker,RRTrack *track,int id);
bool rr_road_picker_start(const RRRoadPicker *picker,RRTrack *track,RRSession *session,RRCar *car);
void rr_road_picker_lap(RRRoadPicker *picker,double seconds);
/* Pure browser eligibility only. Never called by the offline-trial app to grant progress. */
int rr_road_unlock_candidate(RRRaceMode mode,int finish_position,int road_id,int roads_open);
#endif

#include "rr_road_picker.h"
int rr_roads_open(double local_open,int won)
{
    if(isnan(local_open)||local_open==0)local_open=1;
    return (int)fmin(RR_ROAD_COUNT,fmax(1,fmax(local_open,(double)won+1)));
}
bool rr_road_picker_init(RRRoadPicker *p,RRTrack *scratch)
{
    *p=(RRRoadPicker){0};p->current_road=1;p->focus_road=1;p->open_count=1;
    for(int id=1;id<=RR_ROAD_COUNT;id++){
        if(!rr_track_init_road(scratch,id))return false;
        p->rows[id-1].length=scratch->length;p->rows[id-1].gates=scratch->gate_count;
    }
    return rr_track_init_road(scratch,1);
}
void rr_road_picker_open(RRRoadPicker *p,RRRaceMode mode,double local_open,int won)
{
    p->mode=mode;p->open_count=rr_roads_open(local_open,won);p->open=true;
    p->focus_road=p->current_road<=p->open_count?p->current_road:1;
}
void rr_road_picker_close(RRRoadPicker *p){p->open=false;}
void rr_road_picker_move(RRRoadPicker *p,int direction)
{
    if(!p->open || !direction)return;
    p->focus_road=(p->focus_road-1+(direction>0?1:RR_ROAD_COUNT-1))%RR_ROAD_COUNT+1;
}
bool rr_road_picker_choose(RRRoadPicker *p,RRTrack *track,int id)
{
    if(!p->open||id<1||id>RR_ROAD_COUNT||id>p->open_count)return false;
    if(!rr_track_init_road(track,id))return false;
    p->current_road=id;p->focus_road=id;p->open=false;return true;
}
bool rr_road_picker_start(const RRRoadPicker *p,RRTrack *track,RRSession *session,RRCar *car)
{
    if(p->current_road<1||p->current_road>RR_ROAD_COUNT)return false;
    if(!rr_track_init_road(track,p->current_road))return false;
    rr_session_start(session,car,p->mode);return true;
}
void rr_road_picker_lap(RRRoadPicker *p,double seconds)
{
    if(!isfinite(seconds)||seconds<=0)return;
    double *best=&p->rows[p->current_road-1].best_lap;
    if(*best<=0 || seconds<*best)*best=seconds;
}
int rr_road_unlock_candidate(RRRaceMode mode,int position,int id,int open)
{
    if(mode!=RR_RACE_TWO_LAPS||position!=1||id>=RR_ROAD_COUNT||id<1||id+1<=open)return 0;
    return id+1;
}

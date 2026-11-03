#include <stdio.h>
#include "rr_road_picker.h"
#include "rr_minimap.h"
typedef struct PickerFixture {double local;int won,current,mode,pick,open,focus,accepted,selected,closed,position,candidate;} PickerFixture;
typedef struct PickerMapFixture {int road,index;double x,y;} PickerMapFixture;
#include "generated_road_picker_reference.h"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);failures++;}}while(0)
int main(void)
{
    static RRTrack track;RRRoadPicker baseline;
    CHECK(rr_road_picker_init(&baseline,&track));CHECK(track.road->id==1);
    for(size_t i=0;i<sizeof(picker_fixtures)/sizeof(picker_fixtures[0]);i++){
        const PickerFixture *f=&picker_fixtures[i];RRRoadPicker p=baseline;p.current_road=f->current;
        CHECK(rr_track_init_road(&track,p.current_road));
        rr_road_picker_open(&p,(RRRaceMode)f->mode,f->local,f->won);
        CHECK(p.open_count==f->open && p.focus_road==f->focus);
        bool accepted=rr_road_picker_choose(&p,&track,f->pick);
        CHECK(accepted==(f->accepted!=0));CHECK(p.current_road==f->selected && !p.open==(f->closed!=0));
        CHECK(rr_road_unlock_candidate(p.mode,f->position,p.current_road,p.open_count)==f->candidate);
        CHECK(p.open_count==f->open); // eligibility does not grant or persist progress
        if(accepted)CHECK(track.road->id==p.current_road);
        else CHECK(track.road->id==f->current);
    }
    RRSession session;RRCar car;rr_session_init(&session);
    for(int id=1;id<=RR_ROAD_COUNT;id++){
        RRRoadPicker p=baseline;rr_road_picker_open(&p,RR_RACE_TIME_ATTACK,1,8);
        CHECK(rr_road_picker_choose(&p,&track,id));CHECK(track.road->id==id);
        CHECK(track.gate_count==p.rows[id-1].gates && fabs(track.length-p.rows[id-1].length)<1e-8);
        RRMinimap map;rr_minimap_init(&map,&track);
        for(size_t i=0;i<sizeof(picker_map_fixtures)/sizeof(picker_map_fixtures[0]);i++){
            const PickerMapFixture *f=&picker_map_fixtures[i];if(f->road!=id)continue;
            RRVec3 pos=track.samples[f->index];double x,y;rr_minimap_point(&map,pos.x,pos.z,&x,&y);
            CHECK(fabs(x-f->x)<1e-8 && fabs(y-f->y)<1e-8);
        }
        CHECK(rr_road_picker_start(&p,&track,&session,&car));
        CHECK(session.phase==RR_SESSION_COUNTDOWN && session.race.mode==RR_RACE_TIME_ATTACK);
        session.race_time=17;session.race.lap_time=10;car.boost=.2;
        rr_road_picker_lap(&p,120);rr_road_picker_lap(&p,110);rr_road_picker_lap(&p,140);
        rr_road_picker_lap(&p,-1);rr_road_picker_lap(&p,NAN);
        CHECK(p.rows[id-1].best_lap==110);
        CHECK(rr_road_picker_start(&p,&track,&session,&car));
        CHECK(track.road->id==id && session.race_time==0 && session.race.lap_time==0 && car.boost==1);
        CHECK(p.current_road==id && p.mode==RR_RACE_TIME_ATTACK && p.open_count==9);
        CHECK(p.rows[id-1].best_lap==110); // session-local best survives restart, no user file written
    }
    RRRoadPicker p=baseline;rr_road_picker_open(&p,RR_RACE_TWO_LAPS,1,0);
    rr_road_picker_move(&p,-1);CHECK(p.focus_road==9);
    CHECK(!rr_road_picker_choose(&p,&track,9) && p.open && p.current_road==1 && p.open_count==1);
    CHECK(!rr_road_picker_choose(&p,&track,0));rr_road_picker_move(&p,1);CHECK(p.focus_road==1);
    rr_road_picker_close(&p);CHECK(!p.open);CHECK(!rr_road_picker_choose(&p,&track,1));
    rr_session_dispose(&session);
    printf("20 actual browser picker cases, 36 minimap cases; initialization/restart on 9 roads; no progression writes; %d failures.\n",failures);
    return failures?1:0;
}

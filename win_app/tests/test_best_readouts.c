#include "rr_best_readout.h"
#include <string.h>
typedef struct BestCase {int road,open;double value;const char *home,*row;} BestCase;
#include "best_readout_fixture.h"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"Best readout check line%d\n",__LINE__);failures++;}}while(0)
int main(void){
for(size_t i=0;i<sizeof(best_cases)/sizeof(best_cases[0]);i++){
const BestCase *c=&best_cases[i];RRRoadPicker picker={0};picker.current_road=c->road;picker.open_count=c->open;picker.rows[c->road-1].best_lap=c->value;RRRoadPicker before=picker;char home[32],row[64];
rr_ui_best_clock(home,&picker,picker.current_road);rr_ui_best_row(row,&picker,c->road);
CHECK(!strcmp(home,c->home)&&!strcmp(row,c->row));CHECK(picker.current_road==before.current_road&&picker.open_count==before.open_count&&picker.rows[c->road-1].best_lap==before.rows[c->road-1].best_lap);
}
static RRTrack track;RRRoadPicker picker;CHECK(rr_road_picker_init(&picker,&track));RRSession session;RRCar car;rr_session_init(&session);
rr_road_picker_lap(&picker,42.18);char clock[32];rr_ui_best_clock(clock,&picker,1);CHECK(!strcmp(clock,"00:42.18"));
picker.mode=RR_RACE_TIME_ATTACK;CHECK(rr_road_picker_start(&picker,&track,&session,&car));rr_road_picker_lap(&picker,44);rr_ui_best_clock(clock,&picker,1);CHECK(!strcmp(clock,"00:42.18"));
picker.mode=RR_RACE_TWO_LAPS;CHECK(rr_road_picker_start(&picker,&track,&session,&car));rr_road_picker_lap(&picker,41);rr_ui_best_clock(clock,&picker,1);CHECK(!strcmp(clock,"00:41.00"));
CHECK(!rr_road_picker_choose(&picker,&track,2)&&picker.current_road==1&&picker.open_count==1);char locked[64];rr_ui_best_row(locked,&picker,2);CHECK(!strcmp(locked,"WIN ROAD 1 IN 1ST"));rr_session_dispose(&session);
for(int i=0;i<4;i++){float w=(float[]){960,1280,1440,1920}[i],h=(float[]){720,720,960,1080}[i];RRUIMenuLayout l=rr_ui_menu_layout(w,h);CHECK(l.best_y>=l.actions[4].y+l.button_h+6&&l.footer_y>=l.best_y+18&&l.footer_y+146<h);}
printf("Best record UI: %zu actual browser home/row strings; read-only, modes/restart/improvement/locked-road and footer containment checks, %d failures\n",sizeof(best_cases)/sizeof(best_cases[0]),failures);return failures?1:0;}

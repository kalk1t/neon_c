#include "rr_race_ui.h"
#include <stdio.h>
typedef struct HistoryCase {size_t count,best;double scale,height,panel_h;size_t visible;double best_y;int best_visible;} HistoryCase;
typedef struct RowCase {size_t state,row;double y,height,font;} RowCase;
#include "history_style_fixture.h"
static int failures;
#define CHECK(v) do{if(!(v)){if(failures<5)fprintf(stderr,"line%d: %s\n",__LINE__,#v);failures++;}}while(0)
static RRUIHistory layout(const HistoryCase *c){return rr_ui_history_layout((RRUIRect){22*(float)c->scale,140*(float)c->scale,210*(float)c->scale,0},(float)c->scale,(float)c->height,c->count,c->best);}
int main(void){
for(size_t i=0;i<sizeof(history_cases)/sizeof(history_cases[0]);i++){
 const HistoryCase *c=&history_cases[i];RRUIHistory h=layout(c);
 CHECK(fabs(h.panel.h-c->panel_h)<.001);CHECK(h.visible_rows==c->visible);CHECK(h.best_visible==(bool)c->best_visible);if(c->best_visible)CHECK(fabs(h.best_y-c->best_y)<.001);
}
for(size_t i=0;i<sizeof(row_cases)/sizeof(row_cases[0]);i++){
 const RowCase *c=&row_cases[i];RRUIHistory h=layout(&history_cases[c->state]);
 CHECK(fabs(rr_ui_history_row_y(h,c->row)-c->y)<.002);CHECK(fabs(rr_ui_history_row_height(h,c->row)-c->height)<.001);CHECK(fabs(rr_ui_history_row_font(h,c->row)-c->font)<.001);
}
const size_t extremes[]={1000000,SIZE_MAX};for(int k=0;k<2;k++)for(int best=0;best<2;best++){
 RRUIHistory h=rr_ui_history_layout((RRUIRect){22,140,210,0},1,1080,extremes[k],best?extremes[k]-1:0);CHECK(h.visible_rows<=48);CHECK(h.panel.h==940);CHECK(!h.best_visible);CHECK(rr_ui_history_row_y(h,h.visible_rows-1)<1080);CHECK(rr_ui_history_row_y(h,h.visible_rows)>=1080);
}
RRUIHistory invalid=rr_ui_history_layout((RRUIRect){22,140,210,0},1,1000,4,SIZE_MAX);CHECK(invalid.best_index==0);CHECK(rr_ui_history_row_y(invalid,1)==191);
for(int k=0;k<2;k++){RRUIHudLayout h=rr_ui_hud_layout((float)(k?1280:1600),(float)(k?720:1000));CHECK(fabs(h.history.y-140*h.scale)<.001);}
printf("History CSS: %zu source-class layout cases, %zu selected prefix/best/boundary rows; first-best order, 20/16 desktop model and clipped extreme counts: %d failures; no graphics\n",sizeof(history_cases)/sizeof(history_cases[0]),sizeof(row_cases)/sizeof(row_cases[0]),failures);return failures?1:0;
}

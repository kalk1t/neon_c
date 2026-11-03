#include "rr_race_ui.h"
#include "rr_minimap.h"
#include "ui-time-reference.h"
#include "minimap_marker_fixture.h"
#include <stdlib.h>
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d %s\n",__LINE__,#x);failures++;}}while(0)
static bool inside(RRUIRect a,RRUIRect b){return b.x>=a.x&&b.y>=a.y&&b.x+b.w<=a.x+a.w&&b.y+b.h<=a.y+a.h;}
int main(void)
{
    char out[32];for(size_t i=0;i<sizeof(time_cases)/sizeof(time_cases[0]);i++){rr_ui_time(out,time_cases[i].t);CHECK(strcmp(out,time_cases[i].expected)==0);}
    rr_ui_time(out,NAN);CHECK(strcmp(out,"--:--.--")==0);rr_ui_time(out,-1);CHECK(strcmp(out,"--:--.--")==0);
    CHECK(rr_ui_speed(-12.5)==45);CHECK(rr_ui_speed(1.25)==5);CHECK(rr_ui_spin_fill(6)==0);CHECK(rr_ui_spin_fill(3)==.5f);CHECK(rr_ui_spin_fill(0)==1);
    double ties[]={10,10,9,9,11};size_t best=0;
    for(size_t n=1;n<=5;n++){best=rr_ui_best_lap_on_append(ties,n,best);CHECK(best==(n<3?0:2));}
    CHECK(rr_ui_best_lap_on_append(NULL,0,0)==0);
    const float sizes[][2]={{960,720},{1180,900},{1181,721},{1280,720},{1920,1080},{1440,960}};
    for(int k=0;k<6;k++){
        float w=sizes[k][0],h=sizes[k][1];RRUIRect viewport={0,0,w,h};RRUIHudLayout l=rr_ui_hud_layout(w,h);
        CHECK(inside(viewport,l.timer)&&inside(viewport,l.map)&&inside(viewport,l.speed)&&inside(viewport,l.spin)&&inside(viewport,l.history));
        CHECK(l.timer.x+l.timer.w<l.map.x&&l.history.x+l.history.w<l.timer.x);
        CHECK(l.speed.x+l.speed.w<l.spin.x);CHECK(l.scale==((w<=1180||h<=720)?.8f:1.f));
        RRUIOverlayLayout p=rr_ui_pause_layout(w,h),r=rr_ui_results_layout(w,h);CHECK(inside(viewport,p.panel)&&inside(viewport,r.panel));
        for(int i=0;i<3;i++)CHECK(inside(p.panel,p.actions[i]));for(int i=0;i<2;i++)CHECK(inside(r.panel,r.actions[i]));
        CHECK(r.left.x+r.left.w<r.right.x&&r.left.y+r.left.h<r.actions[0].y);
    }
    double ax=-2,ay=0,bx=2,by=0;CHECK(rr_ui_circle_segment(&ax,&ay,&bx,&by,0,0,1));CHECK(fabs(ax+1)<1e-12&&fabs(bx-1)<1e-12);
    ax=-2;ay=2;bx=2;by=2;CHECK(!rr_ui_circle_segment(&ax,&ay,&bx,&by,0,0,1));
    static RRTrack track;
    for(int road=1;road<=9;road++){
        CHECK(rr_track_init_road(&track,road));RRMinimap map;rr_minimap_init(&map,&track);
        for(int i=0;i<track.sample_count;i++){double x,y;rr_minimap_point(&map,track.samples[i].x,track.samples[i].z,&x,&y);CHECK(hypot(x-.5,y-.5)<.49);}
    }
    /* The supplied v3 browser capture has two18px OL rows, best y77 and height103.
     * A fourth lap must retain row0 instead of shifting to a last-three window. */
    RRUIRect anchor={22,108,210,146};RRUIHistory two=rr_ui_history_layout(anchor,1,1000,2);
    CHECK(two.panel.h==103&&two.best_y==185&&two.visible_rows==2&&two.best_visible);
    RRUIHistory four=rr_ui_history_layout(anchor,1,1000,4);
    CHECK(four.visible_rows==4&&four.panel.h==139&&four.best_y==221);
    RRUIHistory empty=rr_ui_history_layout(anchor,1,1000,0);CHECK(empty.visible_rows==0&&empty.panel.h==67&&empty.best_y==149);
    const size_t counts[]={0,1,2,3,4,10,1000,SIZE_MAX};
    for(int k=0;k<6;k++)for(size_t n=0;n<sizeof(counts)/sizeof(counts[0]);n++){
        RRUIHudLayout l=rr_ui_hud_layout(sizes[k][0],sizes[k][1]);RRUIHistory p=rr_ui_history_layout(l.history,l.scale,sizes[k][1],counts[n]);
        CHECK(p.panel.h>=0&&p.panel.y+p.panel.h<=sizes[k][1]+.001f&&p.visible_rows<=counts[n]&&p.visible_rows<=64);
        if(p.visible_rows)CHECK(l.history.y+(35+(float)(p.visible_rows-1)*18)*l.scale<sizes[k][1]);
        if(p.best_visible)CHECK(p.best_y<sizes[k][1]);
    }
    for(size_t i=0;i<sizeof(marker_cases)/sizeof(marker_cases[0]);i++){
        const MarkerCase *c=&marker_cases[i];RRMinimap m={200,100,732};double p[4][2];
        rr_minimap_player_points(&m,c->x,c->z,c->yaw,c->vx,c->vy,c->w,c->h,p);
        for(int j=0;j<4;j++)for(int k=0;k<2;k++)CHECK(fabs(p[j][k]-c->points[j][k])<1e-10);
        /* The notch-to-tip diagonal partitions the source arrow without overlap. */
        for(int side=0;side<2;side++){int b=side?3:2,d=side?2:1;
            double area=(p[b][0]-p[0][0])*(p[d][1]-p[0][1])-(p[b][1]-p[0][1])*(p[d][0]-p[0][0]);CHECK(area<0);}
    }
    RRSession s;RRCar c;rr_session_init(&s);
    for(int phase=RR_SESSION_ACTIVE;phase<=RR_SESSION_RESULTS;phase++)if(phase!=RR_SESSION_PAUSED){
        rr_session_start(&s,&c,RR_RACE_TWO_LAPS);s.phase=(RRSessionPhase)phase;CHECK(rr_session_pause(&s));CHECK(rr_session_resume(&s));CHECK(s.phase==(RRSessionPhase)phase);
        CHECK(rr_session_pause(&s));rr_session_start(&s,&c,RR_RACE_TWO_LAPS);CHECK(s.phase==RR_SESSION_COUNTDOWN&&s.lap_count==0&&s.race.next_checkpoint==0);
        CHECK(rr_session_pause(&s));rr_session_menu(&s);CHECK(s.phase==RR_SESSION_MENU&&s.finish_wait==0);
    }
    rr_session_dispose(&s);printf("Race UI: source Canvas player marker and chronological history/capture bounds, 142 browser time cases, 6 viewports, 9 minimaps and pause/restart/menu lifecycle: %d failures\n",failures);return failures?1:0;
}

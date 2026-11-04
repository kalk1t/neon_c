#include "rr_results_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,draws;static bool draw_ok=true;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static RRRunCurrent run;static RRBattleCurrent graph;static RRLocalBattle battle;
static RRLocalPack pack;static RRLocalRacer actors[11];static RRTrack track;
static double laps[2]={30,40};static RRResultsCurrent painted;static size_t revision;
static bool draw(void *ctx,const RRResultsCurrent *s,size_t rev){CHECK(ctx==&draws);painted=*s;revision=rev;draws++;return draw_ok;}
static void setup(void){
 run.current=&graph;run.host.battle=&battle;graph.battle=&battle;graph.pack.pack=&pack;
 battle.pack=&pack;pack.player=battle.fire.player=actors;pack.track=&track;
 track.sample_count=4096;pack.finish_position=1;run.host.phase=RR_SESSION_FINISHED;run.host.race_time=70;
 for(int i=0;i<11;i++){battle.roster[i]=actors+i;actors[i].race.mode=RR_RACE_TWO_LAPS;actors[i].race.lap=1;actors[i].race.next_checkpoint=1;actors[i].ai.sample=i*173;}
 actors[0].race.finished=true;actors[0].race.finish_time=70;actors[0].lap_times=laps;actors[0].lap_count=actors[0].lap_capacity=2;
}
int main(void){
 setup();RRResultsTimer t={0};RRResultsPump ev;RRRunEvents finish={0};finish.player=RR_RACE_FINISH;
 CHECK(rr_results_ui_frame(&t,&run,&finish,10,false,draw,&draws,&ev));CHECK(t.pending==1&&!draws);
 CHECK(rr_results_ui_frame(&t,&run,NULL,11.999999,false,draw,&draws,&ev));CHECK(!draws);
 CHECK(rr_results_ui_frame(&t,&run,NULL,12,false,draw,&draws,&ev));CHECK(ev.callbacks==1&&ev.published==1&&draws==1&&revision==1&&painted.total==70);
 RRResultsCurrent frozen=painted;laps[0]=999;actors[1].coins=99;
 CHECK(rr_results_ui_frame(&t,&run,NULL,13,false,draw,&draws,&ev));CHECK(draws==2&&!memcmp(&painted,&frozen,sizeof(frozen))&&!ev.callbacks);
 /* Draw errors preserve the publication and a later frame can repaint it. */
 draw_ok=false;CHECK(!rr_results_ui_frame(&t,&run,NULL,14,false,draw,&draws,&ev));CHECK(!t.error&&t.visible&&t.revision==1);
 draw_ok=true;CHECK(rr_results_ui_frame(&t,&run,NULL,15,false,draw,&draws,&ev));CHECK(draws==4&&revision==1);
 CHECK(!rr_results_ui_frame(&t,&run,NULL,16,false,NULL,NULL,&ev));CHECK(t.visible&&!t.error);
 /* Hide does not cancel; menu UI-only pump consumes due callback without draw. */
 CHECK(rr_results_ui_frame(&t,&run,&finish,17,true,draw,&draws,&ev));CHECK(!t.visible&&t.pending==1);
 run.host.phase=RR_SESSION_MENU;unsigned before=draws;
 CHECK(rr_results_ui_frame(&t,&run,NULL,20,true,draw,&draws,&ev));CHECK(ev.callbacks==1&&!ev.published&&!t.pending&&draws==before);
 /* An old callback reads a newly finished race, even after a same-frame hide. */
 run.host.phase=RR_SESSION_FINISHED;laps[0]=20;
 CHECK(rr_results_ui_frame(&t,&run,&finish,21,true,draw,&draws,&ev));
 run.host.phase=RR_SESSION_COUNTDOWN;CHECK(rr_results_ui_frame(&t,&run,NULL,22,true,draw,&draws,&ev));
 run.host.phase=RR_SESSION_FINISHED;laps[1]=50;
 CHECK(rr_results_ui_frame(&t,&run,&finish,22.5,true,draw,&draws,&ev));
 CHECK(rr_results_ui_frame(&t,&run,NULL,23,true,draw,&draws,&ev));CHECK(ev.callbacks==1&&ev.published==1&&t.visible&&revision==2&&painted.total==70&&t.pending==1);
 CHECK(rr_results_ui_frame(&t,&run,NULL,24.5,false,draw,&draws,&ev));CHECK(ev.published==1&&revision==3&&!t.pending);
 RRResultsTimer copy=t;RRResultsPump zero={9,9};run.host.error=true;
 CHECK(!rr_results_ui_frame(&t,&run,NULL,25,true,draw,&draws,&zero));CHECK(!memcmp(&t,&copy,sizeof(t))&&!zero.callbacks&&!zero.published);run.host.error=false;
 CHECK(!rr_results_ui_frame(&t,&run,NULL,24,true,draw,&draws,&ev));CHECK(t.error&&t.visible);
 RRResultsTimer bad={0};CHECK(!rr_results_ui_frame(&bad,&run,NULL,INFINITY,false,draw,&draws,&ev));CHECK(bad.error);
 bad=(RRResultsTimer){0};CHECK(!rr_results_ui_frame(&bad,&run,NULL,-1,false,draw,&draws,&ev));CHECK(bad.error);
 bad=(RRResultsTimer){0};CHECK(!rr_results_ui_frame(&bad,&run,NULL,NAN,false,draw,&draws,&ev));CHECK(bad.error);
 printf("RESULTS_UI_SUMMARY draws=%u revision=%zu failures=%u no_graphics=true\n",draws,revision,failures);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

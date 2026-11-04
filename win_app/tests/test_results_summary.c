#include "rr_results_summary.h"
#include "rr_results_bind.h"
#include "results_summary_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,draws,lookups;
static RRLocalRacer actors[11],saved_actors[11];static const RRLocalRacer *roster[11];
static RRResultsCurrent snapshot,saved_snapshot;static RRResultsSummary text,saved_text;
static RRRunCurrent run;static RRBattleCurrent graph;static RRLocalBattle battle;static RRLocalPack pack;static RRTrack track;static double laps[2]={10,20};
static RRResultsTimer timer;static RRResultsBinding binding;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){fprintf(stderr,"summary line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static const char *name(void *ctx,size_t i,const RRLocalRacer *r){(void)ctx;CHECK(i<11&&r==actors+i);lookups++;return "Pilot";}
static bool draw(void *ctx,const RRResultsBoundView *v){CHECK(ctx==&binding);draws++;return rr_results_summary_build(v->results,&text);}
static void actors_ready(size_t n){memset(actors,0,sizeof(actors));for(size_t i=0;i<n;i++){roster[i]=actors+i;actors[i].race.finished=true;actors[i].race.finish_time=20+(double)i;actors[i].race.mode=RR_RACE_TWO_LAPS;actors[i].coins=(int)i;}}
int main(void){
 for(size_t i=0;i<COUNT(summary_cases);i++){
  const SummaryCase *c=summary_cases+i;actors_ready((size_t)c->count);actors[0].takedowns=c->takedowns;actors[0].coins=100;
  for(int j=1;j<c->count;j++)actors[j].coins=j<c->coin?200:0;
  memcpy(saved_actors,actors,sizeof(actors));
  CHECK(rr_results_current_build(roster,(size_t)c->count,0,4096,100,c->finish,RR_SESSION_FINISHED,&snapshot));
  CHECK(!memcmp(actors,saved_actors,sizeof(actors))&&snapshot.player_takedowns==c->takedowns&&snapshot.player_coin_rank==c->coin&&snapshot.finish_position==c->finish);
  actors[0].takedowns=7; /* formatting must read only the copied publication */
  CHECK(rr_results_summary_build(&snapshot,&text));CHECK(!strcmp(text.place,c->place)&&!strcmp(text.coin_ordinal,c->coin_ordinal));
 }
 for(size_t i=0;i<COUNT(ordinal_cases);i++){char out[RR_RESULTS_ORDINAL_BYTES];CHECK(rr_results_ordinal(ordinal_cases[i].n,out)&&!strcmp(out,ordinal_cases[i].text));}
 memset(&text,0x5a,sizeof(text));saved_text=text;saved_snapshot=snapshot;
 snapshot.player_takedowns=-1;CHECK(!rr_results_summary_build(&snapshot,&text)&&!memcmp(&text,&saved_text,sizeof(text)));snapshot=saved_snapshot;
 snapshot.coin_count=0;CHECK(!rr_results_summary_build(&snapshot,&text));snapshot=saved_snapshot;
 snapshot.coin_count=12;CHECK(!rr_results_summary_build(&snapshot,&text));snapshot=saved_snapshot;
 snapshot.finish_position=0;CHECK(!rr_results_summary_build(&snapshot,&text));snapshot=saved_snapshot;
 snapshot.player_coin_rank=12;CHECK(!rr_results_summary_build(&snapshot,&text));snapshot=saved_snapshot;
 CHECK(!memcmp(&text,&saved_text,sizeof(text))&&!rr_results_summary_build(NULL,&text)&&!rr_results_summary_build(&snapshot,NULL));
 char ordinal[16],old_ordinal[16];memset(ordinal,0x5a,sizeof(ordinal));memcpy(old_ordinal,ordinal,sizeof(ordinal));CHECK(!rr_results_ordinal(0,ordinal)&&!rr_results_ordinal(-1,ordinal)&&!rr_results_ordinal(1,NULL)&&!memcmp(ordinal,old_ordinal,sizeof(ordinal)));
 actors_ready(11);actors[0].takedowns=-1;snapshot=saved_snapshot;CHECK(!rr_results_current_build(roster,11,0,4096,100,1,RR_SESSION_FINISHED,&snapshot)&&!memcmp(&snapshot,&saved_snapshot,sizeof(snapshot)));
 /* Actual timer/binding graph boundary, no simulation/graphics or account hook. */
 actors_ready(11);actors[0].takedowns=1;actors[0].lap_times=laps;actors[0].lap_count=actors[0].lap_capacity=2;
 run.current=&graph;run.host.battle=&battle;run.host.phase=RR_SESSION_FINISHED;run.host.race_time=100;graph.battle=&battle;graph.pack.pack=&pack;battle.pack=&pack;pack.player=battle.fire.player=actors;pack.track=&track;track.sample_count=4096;pack.finish_position=3;
 for(size_t i=0;i<11;i++)battle.roster[i]=actors+i;
 RRResultsPump events;CHECK(rr_results_timer_schedule(&timer,0));actors[0].takedowns=2;
 CHECK(rr_results_bind_frame(&binding,&timer,&run,NULL,1.999,false,name,NULL,draw,&binding,&events)&&!events.published&&!draws);
 CHECK(rr_results_bind_frame(&binding,&timer,&run,NULL,2,false,name,NULL,draw,&binding,&events)&&events.published==1&&draws==1&&lookups==11&&timer.snapshot.player_takedowns==2);
 CHECK(!strcmp(text.place,"3rd OF 11 \302\267 2 TAKEDOWNS")&&!strcmp(text.coin_ordinal,"11th"));saved_text=text;
 actors[0].takedowns=17;actors[0].coins=999;pack.finish_position=8;
 CHECK(rr_results_bind_frame(&binding,&timer,&run,NULL,2.001,false,name,NULL,draw,&binding,&events)&&!events.published&&draws==2&&lookups==11&&!memcmp(&text,&saved_text,sizeof(text)));
 CHECK(rr_results_timer_schedule(&timer,2001));run.host.phase=RR_SESSION_COUNTDOWN;
 CHECK(rr_results_bind_frame(&binding,&timer,&run,NULL,4.001,true,name,NULL,draw,&binding,&events)&&events.callbacks==1&&!events.published&&draws==2&&timer.snapshot.player_takedowns==2&&!timer.visible);
 CHECK(rr_results_timer_schedule(&timer,4002));run.host.phase=RR_SESSION_FINISHED;actors[0].takedowns=1;
 CHECK(rr_results_bind_frame(&binding,&timer,&run,NULL,6.002,false,name,NULL,draw,&binding,&events)&&events.published==1&&draws==3&&lookups==22&&timer.snapshot.player_takedowns==1);
 CHECK(!strcmp(text.place,"8th OF 11 \302\267 1 TAKEDOWN")&&!strcmp(text.coin_ordinal,"1st"));
 printf("RESULTS_SUMMARY_DETAIL source_cases=%zu ordinal_cases=%zu bound_publications=%zu bound_draws=%u metadata_lookups=%u snapshot_bytes=%zu summary_bytes=%zu failures=%u no_rewards_or_render=true\n",COUNT(summary_cases),COUNT(ordinal_cases),timer.revision,draws,lookups,sizeof(RRResultsCurrent),sizeof(RRResultsSummary),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

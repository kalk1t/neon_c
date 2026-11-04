#include "rr_results_bind.h"
#include <stdio.h>
#include <string.h>
static RRResultsBinding replay_bound;static unsigned bound_failures,bound_draws,bound_lookups;static size_t bound_frame,bound_pub_frame,bound_revision;static char bound_name[64];
static const char *replay_name(void *ctx,size_t index,const RRLocalRacer *racer){const RRRunCurrent *r=ctx;if(index>=11||racer!=r->host.battle->roster[index])bound_failures++;bound_lookups++;snprintf(bound_name,sizeof(bound_name),"F%zu D%zu",bound_frame,index);return bound_name;}
static bool replay_draw(void *ctx,const RRResultsBoundView *view){(void)ctx;if(!view||view->count!=11||!view->revision){bound_failures++;return false;}if(view->revision!=bound_revision){bound_revision=view->revision;bound_pub_frame=bound_frame;}for(size_t i=0;i<11;i++){char expected[64];snprintf(expected,sizeof(expected),"F%zu D%zu",bound_pub_frame,i);if(strcmp(expected,view->names[i]))bound_failures++;}bound_draws++;return true;}
static bool replay_tick(RRResultsTimer *timer,const RRRunCurrent *run,const RRRunEvents *tick,double now,RRResultsPump *events){
 /* Each source trace is an independent application/timer lifetime, not a race
  * restart. Ordinary source start() only hides timer and retains binding. */
 if(!timer->clock_ready){memset(&replay_bound,0,sizeof(replay_bound));bound_revision=0;}
 bool ok=rr_results_bind_frame(&replay_bound,timer,run,tick,now/1000,false,replay_name,(void *)run,replay_draw,NULL,events);bound_frame++;return ok;
}
#define rr_run_results_after_tick replay_tick
#define main results_binding_reference_main
#include "test_results_timer.c"
#undef main
#undef rr_run_results_after_tick
int main(void){int result=results_binding_reference_main();if(bound_draws!=timer_visible||bound_lookups!=timer_published*11)bound_failures++;printf("RESULTS_BIND_REPLAY frames=%zu draws=%u names=%u source_publications=%u failures=%u production_inactive=true\n",bound_frame,bound_draws,bound_lookups,timer_published,bound_failures);return result||bound_failures;}

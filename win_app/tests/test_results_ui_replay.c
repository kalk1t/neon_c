/* Reuse the unchanged actual-source timer/input/pack oracle and assertions,
 * routing its completed ticks through the UI-frame seconds/drawing boundary. */
#include "rr_results_ui.h"
#include <stdio.h>
static unsigned ui_replay_draws;
static bool ui_replay_draw(void *ctx,const RRResultsCurrent *snapshot,size_t revision)
{(void)ctx;if(!snapshot||!revision)return false;ui_replay_draws++;return true;}
static bool ui_replay_tick(RRResultsTimer *timer,const RRRunCurrent *run,
 const RRRunEvents *tick,double now_ms,RRResultsPump *events)
{return rr_results_ui_frame(timer,run,tick,now_ms/1000,false,ui_replay_draw,NULL,events);}
#define rr_run_results_after_tick ui_replay_tick
#define main results_oracle_main
#include "test_results_timer.c"
#undef main
#undef rr_run_results_after_tick
int main(void){int result=results_oracle_main();
 printf("RESULTS_UI_REPLAY draws=%u expected_visible=%u\n",ui_replay_draws,timer_visible);
 return result||ui_replay_draws!=timer_visible;}

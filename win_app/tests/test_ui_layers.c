#include "rr_ui_layers.h"
#include "rr_finish.h"
#include <stdio.h>
typedef struct LayerCase {int phase,count,layers[3];} LayerCase;
#include "ui_layers_fixture.h"
static int failures;
static RRUIPaintLayer layer_at(RRSessionPhase phase,int index){RRUIPaintPlan plan=rr_ui_paint_plan(phase);return plan.layers[index];}
static void check(RRUIPaintPlan p){int hud=0,nitro=0,overlay=0;if(p.count<1||p.count>3)failures++;for(int i=0;i<p.count;i++){if(p.layers[i]==RR_UI_LAYER_HUD)hud++;else if(p.layers[i]==RR_UI_LAYER_NITRO)nitro++;else{overlay++;if(i!=p.count-1)failures++;}}if(hud>1||nitro!=1||overlay>1)failures++;}
int main(void){for(size_t i=0;i<sizeof(layer_cases)/sizeof(layer_cases[0]);i++){
 const LayerCase *c=&layer_cases[i];RRUIPaintPlan p=rr_ui_paint_plan((RRSessionPhase)c->phase);check(p);if(p.count!=c->count)failures++;for(int j=0;j<p.count&&j<c->count;j++)if((int)p.layers[j]!=c->layers[j])failures++;
}
if(rr_ui_results_backdrop()!=source_backdrop)failures++;
/* Native error and focus/overload pause are safe extensions beyond source. */
for(int phase=RR_SESSION_MENU;phase<=RR_SESSION_ERROR;phase++)check(rr_ui_paint_plan((RRSessionPhase)phase));
RRUIPaintPlan error=rr_ui_paint_plan(RR_SESSION_ERROR);if(error.count!=2||error.layers[0]!=RR_UI_LAYER_NITRO||error.layers[1]!=RR_UI_LAYER_ERROR)failures++;
/* Session time and pending results still freeze/cancel correctly while the
 * painter hides HUD. Drawing never calls a simulation step or advances state. */
RRSession s;RRCar car={0};rr_session_init(&s);rr_session_start(&s,&car,RR_RACE_TWO_LAPS);RRUIPaintPlan countdown=rr_ui_paint_plan(s.phase);
if(!rr_session_pause(&s)||layer_at(s.phase,1)!=RR_UI_LAYER_PAUSE)failures++;
double clock=s.countdown;rr_session_step(&s,&car,NULL,0,(RRInput){0},1);if(s.countdown!=clock)failures++;if(!rr_session_resume(&s)||rr_ui_paint_plan(s.phase).count!=countdown.count)failures++;
s.phase=RR_SESSION_FINISHED;s.race.finished=true;s.finish_wait=1.99;rr_session_step(&s,&car,NULL,0,(RRInput){0},.02);RRUIPaintPlan results=rr_ui_paint_plan(s.phase);if(s.phase!=RR_SESSION_RESULTS||results.count!=3||results.layers[0]!=RR_UI_LAYER_HUD||results.layers[2]!=RR_UI_LAYER_RESULTS)failures++;
if(!rr_session_pause(&s)||rr_ui_paint_plan(s.phase).count!=2||!rr_session_resume(&s)||rr_ui_paint_plan(s.phase).count!=3)failures++;
rr_session_menu(&s);rr_session_step(&s,&car,NULL,0,(RRInput){0},5);if(layer_at(s.phase,1)!=RR_UI_LAYER_MENU)failures++;
rr_session_start(&s,&car,RR_RACE_TIME_ATTACK);if(layer_at(s.phase,0)!=RR_UI_LAYER_HUD||s.lap_count!=0)failures++;
rr_session_dispose(&s);
printf("Actual browser UI layers: %zu transition states, CSS backdrop/order, external NITRO, native guards and lifecycle; max3 layers, %d failures\n",sizeof(layer_cases)/sizeof(layer_cases[0]),failures);return failures?1:0;}

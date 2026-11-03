#include "rr_hud_message.h"
#include "rr_recovery.h"
#include <string.h>
#include <math.h>
typedef struct MessageCase {int op,lap,total;double dt;const char *input;double duration;int input_warn;const char *text;double timer;int shown,warn;} MessageCase;
#include "hud_message_fixture.h"
int main(void){RRHUDMessage m={0};int failures=0;
for(size_t i=0;i<sizeof(message_cases)/sizeof(message_cases[0]);i++){
const MessageCase *c=&message_cases[i];
if(c->op==0)rr_hud_message_set(&m,c->input,c->duration,c->input_warn!=0);
else if(c->op==1)rr_hud_message_lap(&m,c->lap,c->total);
else if(c->op==2)rr_hud_message_finish(&m);
else if(c->op==3)rr_hud_message_tick(&m,c->dt);
/* op4: menu/restart do not mutate the browser's existing HUD message. */
if(strcmp(m.text,c->text)||fabs(m.timer-c->timer)>1e-12||m.shown!=(c->shown!=0)||m.warn!=(c->warn!=0)){
fprintf(stderr,"Message mismatch row %zu\n",i);failures++;}
}
RRRecovery recovery={0};rr_hud_message_lap(&m,2,2);
if(rr_recovery_conditions(&recovery,-30,0,115,34,1.3,m.timer,false)&RR_RECOVERY_WRONG_WAY)failures++;
rr_hud_message_tick(&m,1.6);
if(!(rr_recovery_conditions(&recovery,-30,0,115,34,0,m.timer,false)&RR_RECOVERY_WRONG_WAY))failures++;
printf("Actual browser HUD announcement channel: %zu states plus recovery timer coupling, %d failures\n",sizeof(message_cases)/sizeof(message_cases[0]),failures);return failures?1:0;}

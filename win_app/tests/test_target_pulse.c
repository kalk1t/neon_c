#include "rr_target_pulse.h"
#include "rr_target_guidance.h"
#include <stdio.h>
typedef struct PulseCase {int sequence;double now,nx,ny,nz;int hud;double w,h,zoom;int rendered;double pulse,x,y;} PulseCase;
typedef struct MarkerCase {double x,y,cx,cy,radius,text_top;} MarkerCase;
#include "target_pulse_fixture.h"
static int failures;static double worst;
#define CHECK(v) do{if(!(v)){if(failures<5)fprintf(stderr,"line%d: %s\n",__LINE__,#v);failures++;}}while(0)
int main(void){RRTargetPulse clock={0};int sequence=-1;
for(size_t i=0;i<sizeof(pulse_cases)/sizeof(pulse_cases[0]);i++){
 const PulseCase *c=&pulse_cases[i];if(c->sequence!=sequence){clock=(RRTargetPulse){0};sequence=c->sequence;}
 RRTargetGuidance target=rr_target_guidance(c->nx,c->ny,c->nz,c->w,c->h,c->zoom,240);bool rendered=target.on&&c->hud;CHECK(rendered==(c->rendered!=0));double pulse=rr_target_pulse_observe(&clock,rendered,c->now),error=fabs(pulse-c->pulse);if(error>worst)worst=error;
 if(error>1e-9){if(failures<5)fprintf(stderr,"Pulse row%zu got%.17g expected%.17g\n",i,pulse,c->pulse);failures++;}CHECK(pulse>=1&&pulse<=1.25);
 if(rendered){RRTargetMarker m=rr_target_marker(target.x,target.y,c->zoom,pulse);CHECK(fabs(m.cx-c->x)<1e-8);CHECK(fabs(m.cy-(c->y-12*c->zoom))<1e-8);CHECK(fabs(m.text_top-(c->y+7*c->zoom))<1e-8);CHECK(fabs(m.radius/sqrt(2.0)-11*c->zoom*pulse)<1e-10);CHECK(fabs((m.radius-m.thickness*sqrt(2.0))/sqrt(2.0)-8*c->zoom*pulse)<1e-10);}
}
RRTargetMarker measured=rr_target_marker(marker_case.x,marker_case.y,1,1);CHECK(fabs(measured.cx-marker_case.cx)<.02);CHECK(fabs(measured.cy-marker_case.cy)<.001);CHECK(fabs(measured.radius-marker_case.radius)<.001);CHECK(measured.text_top==marker_case.text_top);CHECK(measured.font==13);CHECK(fabs(measured.text_baseline-(507+(16-13*1.254)/2+13*1.011))<1e-10);
clock=(RRTargetPulse){0};CHECK(rr_target_pulse_observe(&clock,true,10)==1);CHECK(rr_target_pulse_observe(&clock,true,10.8)==1.25);CHECK(fabs(rr_target_pulse_observe(&clock,true,11.6)-1)<1e-12);rr_target_pulse_observe(&clock,false,12);CHECK(rr_target_pulse_observe(&clock,true,13)==1);CHECK(rr_target_pulse_observe(&clock,true,13.8)==1.25);
printf("Target pulse: %zu source DOM/spec-model states across16 sequences, one reused group/child box; layout/border scaling and display restarts; worst scale error%.12g, %d failures; no graphics\n",sizeof(pulse_cases)/sizeof(pulse_cases[0]),worst,failures);return failures?1:0;}

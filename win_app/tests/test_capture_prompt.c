#include "rr_capture_prompt.h"
#include "rr_mouse_capture.h"
#include <stdio.h>
#include <string.h>
typedef struct HintCase {int phase,captured,touch,shown;} HintCase;
typedef struct GestureCase {int sequence,phase,captured,focused,inside,pressed,released,request;} GestureCase;
#include "capture_prompt_fixture.h"
static int failures;
#define CHECK(v) do{if(!(v)){if(failures<5)fprintf(stderr,"line%d: %s\n",__LINE__,#v);failures++;}}while(0)
int main(void){CHECK(strcmp(RR_CAPTURE_HINT_TEXT,source_hint_text)==0);
for(size_t i=0;i<sizeof(hint_cases)/sizeof(hint_cases[0]);i++){const HintCase *c=&hint_cases[i];CHECK(rr_capture_hint_visible((RRSessionPhase)c->phase,c->captured!=0,c->touch!=0)==(c->shown!=0));}
RRMouseClickCapture click={0};int sequence=-1;
for(size_t i=0;i<sizeof(gesture_cases)/sizeof(gesture_cases[0]);i++){const GestureCase *c=&gesture_cases[i];if(sequence!=c->sequence){click=(RRMouseClickCapture){0};sequence=c->sequence;}bool request=rr_capture_click_step(&click,(RRSessionPhase)c->phase,c->focused!=0,c->captured!=0,c->pressed!=0,c->released!=0,c->inside!=0);if(request!=(c->request!=0)){if(failures<5)fprintf(stderr,"Gesture row%zu\n",i);failures++;}}
/* Successful capture hides the hint immediately, while the unchanged aim
 * guard still rejects the click's residual frame motion for150ms. */
RRMouseCapture capture={0};click=(RRMouseClickCapture){0};CHECK(rr_capture_hint_visible(RR_SESSION_ACTIVE,false,false));CHECK(!rr_capture_click_step(&click,RR_SESSION_ACTIVE,true,false,true,false,true));CHECK(rr_capture_click_step(&click,RR_SESSION_ACTIVE,true,false,false,true,true));rr_mouse_capture_change(&capture,true,1000);CHECK(!rr_capture_hint_visible(RR_SESSION_ACTIVE,capture.captured,false));CHECK(!rr_mouse_capture_allows(&capture,1000));CHECK(!rr_mouse_capture_allows(&capture,1149.999));CHECK(rr_mouse_capture_allows(&capture,1150));CHECK(!click.armed);CHECK(!rr_capture_click_step(&click,RR_SESSION_ACTIVE,true,true,false,true,true));
const double viewports[][3]={{1600,1000,1},{1920,1080,1},{1280,720,.8},{960,720,.8}};
for(int i=0;i<4;i++){const double *v=viewports[i];RRCaptureHintLayout l=rr_capture_hint_layout(v[0],v[1],v[2]);CHECK(l.cx==v[0]/2&&l.top==v[1]*.62&&l.font==14*v[2]&&l.spacing==3*v[2]);double line=floor(14*v[2]*1.254+.5),baseline=l.top+(line-14*v[2]*1.254)/2+14*v[2]*1.011;CHECK(l.height==line&&fabs(l.baseline-baseline)<1e-10);}
printf("Capture prompt: %zu actual-source hint cases, %zu callback/gesture states across98 sequences; native ownership and unchanged150ms guard; CSS/nominal layout, %d failures; no graphics or OS capture\n",sizeof(hint_cases)/sizeof(hint_cases[0]),sizeof(gesture_cases)/sizeof(gesture_cases[0]),failures);return failures?1:0;}

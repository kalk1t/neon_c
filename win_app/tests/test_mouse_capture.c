#include "rr_mouse_capture.h"
#include "rr_mouse_aim.h"
#include <stdio.h>
typedef struct CaptureCase {int change,captured;double changed,now;int allowed;} CaptureCase;
#include "mouse_capture_fixture.h"
int main(void){int failures=0;RRMouseCapture capture={0};
for(size_t i=0;i<sizeof(capture_cases)/sizeof(capture_cases[0]);i++){
const CaptureCase *c=&capture_cases[i];if(c->change)rr_mouse_capture_change(&capture,c->captured!=0,c->changed);
if(rr_mouse_capture_allows(&capture,c->now)!=(c->allowed!=0)){fprintf(stderr,"Capture row%zu\n",i);failures++;}
}
rr_mouse_capture_change(&capture,true,1000);if(rr_mouse_capture_allows(&capture,NAN))failures++;
RRMouseAim aim={0};RRInput keyboard={0};keyboard.steer=1;keyboard.climb=-1;
bool accepted=rr_mouse_capture_allows(&capture,1149.999);RRInput result=rr_mouse_aim_step(&aim,(RRMouseSettings){1,false},.5,-.2,keyboard,accepted?500:0,accepted?500:0,false);
if(result.steer!=1||result.climb!=-1||aim.yaw!=.5||aim.pitch!=-.2)failures++;
rr_mouse_capture_change(&capture,false,2000);if(rr_mouse_capture_allows(&capture,5000))failures++;
rr_mouse_capture_change(&capture,true,5000);if(rr_mouse_capture_allows(&capture,5149.999)||!rr_mouse_capture_allows(&capture,5150))failures++;
printf("Actual browser capture guard: %zu source listener eligibility states plus keyboard/capture-loss/reacquire checks, %d failures; event-vs-frame delivery unqualified\n",sizeof(capture_cases)/sizeof(capture_cases[0]),failures);return failures?1:0;}

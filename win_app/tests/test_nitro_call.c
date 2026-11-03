#include "rr_nitro_call.h"
#include "rr_camera_fx.h"
#include <stdio.h>
typedef struct NitroBrowserCase {double seconds,opacity,scale,spacing,translation_y;} NitroBrowserCase;
#include "nitro_call_fixture.h"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"Nitro call check line%d\n",__LINE__);failures++;}}while(0)
int main(void){RRNitroCall call={0};CHECK(!rr_nitro_call_sample(&call,1).shown);rr_nitro_call_start(&call,10);
for(size_t i=0;i<sizeof(nitro_browser_cases)/sizeof(nitro_browser_cases[0]);i++){
const NitroBrowserCase *c=&nitro_browser_cases[i];RRNitroStyle s=rr_nitro_call_sample(&call,10+c->seconds);
CHECK(s.shown&&fabs(s.opacity-c->opacity)<1e-6&&fabs(s.scale-c->scale)<1e-6&&fabs(s.spacing-c->spacing)<1e-6&&fabs(s.translate_y-c->translation_y)<.0002);
}
CHECK(!rr_nitro_call_sample(&call,9).shown&&!rr_nitro_call_sample(&call,NAN).shown);
CHECK(rr_nitro_call_sample(&call,10+.749999).shown);CHECK(!rr_nitro_call_sample(&call,10.75).shown);
rr_nitro_call_start(&call,11);RRNitroStyle begin=rr_nitro_call_sample(&call,11);CHECK(begin.shown&&begin.opacity==0&&begin.scale==1.6&&begin.spacing==30&&begin.translate_y==-50);
/* Pause/minimize/low-FPS do not slow the CSS wall clock. New ignition restarts. */
CHECK(!rr_nitro_call_sample(&call,20).shown);rr_nitro_call_start(&call,20);CHECK(rr_nitro_call_sample(&call,20.135).opacity>.99999);
RRCameraFX fx={0};fx.last_nitro=-9;int starts=0;
if(rr_camera_fx_ignite(&fx,false,true,2)){starts++;rr_nitro_call_start(&call,30);}
if(rr_camera_fx_ignite(&fx,false,true,3)){starts++;rr_nitro_call_start(&call,31);}
CHECK(starts==1&&call.started==30);
if(rr_camera_fx_ignite(&fx,false,true,3.000001)){starts++;rr_nitro_call_start(&call,31.000001);}
CHECK(starts==2&&call.started==31.000001);
printf("Nitro call: %zu manifest-verified Chromium samples, wall-clock/retrigger/expiry checks, %d failures; no GPU\n",sizeof(nitro_browser_cases)/sizeof(nitro_browser_cases[0]),failures);return failures?1:0;}

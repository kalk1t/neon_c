#include "rr_boost_meter.h"
#include <stdio.h>
typedef struct BoostCase {double boost;int on,low,animation;double opacity,brightness,bounds[4];} BoostCase;
#include "boost_meter_fixture.h"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"Boost meter check line%d\n",__LINE__);failures++;}}while(0)
int main(void){
for(size_t i=0;i<sizeof(boost_cases)/sizeof(boost_cases[0]);i++){
 const BoostCase *c=&boost_cases[i];RRBoostMeter m={0};rr_boost_meter_observe(&m,c->boost,c->on!=0,10);RRBoostStyle style=rr_boost_meter_style(&m,10);double bounds[4];rr_boost_fill_bounds(m.fill,bounds);
 CHECK(m.on==(c->on!=0)&&m.low==(c->low!=0)&&m.animation==c->animation);
 CHECK(fabs(style.opacity-c->opacity)<1e-8&&fabs(style.brightness-c->brightness)<1e-8);
 for(int j=0;j<4;j++)CHECK(fabs(bounds[j]-c->bounds[j])<.002);
}
RRBoostMeter m={0};rr_boost_meter_observe(&m,.2,true,10);CHECK(!m.low&&m.animation==RR_BOOST_BURNING);
RRBoostStyle style=rr_boost_meter_style(&m,10.22);CHECK(fabs(style.brightness-1.8)<1e-8&&style.opacity==1);
style=rr_boost_meter_style(&m,10.44);CHECK(fabs(style.brightness-1)<1e-8);
rr_boost_meter_observe(&m,.199999999,true,11);CHECK(m.low&&m.animation==RR_BOOST_LOW&&m.started==11);
style=rr_boost_meter_style(&m,11.4);CHECK(fabs(style.opacity-.25)<1e-8&&style.brightness==1);
rr_boost_meter_observe(&m,.1,false,11.4);CHECK(m.started==11); // same low animation despite gradient change
style=rr_boost_meter_style(&m,11.8);CHECK(fabs(style.opacity-1)<1e-8);
rr_boost_meter_observe(&m,.1,true,12);CHECK(m.started==11); // still low, no brightness pulse
rr_boost_meter_observe(&m,.3,true,12.1);CHECK(m.started==12.1&&m.animation==RR_BOOST_BURNING);
rr_boost_meter_observe(&m,.3,false,12.2);style=rr_boost_meter_style(&m,20);CHECK(m.animation==RR_BOOST_STATIC&&style.opacity==1&&style.brightness==1);
printf("Boost meter: %zu manifest-verified Chromium initial states/geometry, strict20%% threshold, animation precedence/restart/alternate endpoints, %d failures; no GPU\n",sizeof(boost_cases)/sizeof(boost_cases[0]),failures);return failures?1:0;}

#include "rr_boost_ui.h"
#include "rr_boost_meter.h"
#include <stdio.h>
typedef struct BoostBoxCase {double zoom,x,y,w,h,border,label[4];} BoostBoxCase;
typedef struct BoostRenderCase {int sequence;double now,fill;int on,rendered,low,animation;double opacity,brightness;} BoostRenderCase;
#include "boost_layout_fixture.h"
static int failures;static double worst;
#define CHECK(v) do{if(!(v)){if(failures<5)fprintf(stderr,"line%d: %s\n",__LINE__,#v);failures++;}}while(0)
int main(void){const double shear=tan(25*RR_PI/180);
for(size_t i=0;i<sizeof(box_cases)/sizeof(box_cases[0]);i++){
 const BoostBoxCase *c=&box_cases[i];RRBoostUI b=rr_boost_ui((RRUIRect){0},(float)c->zoom);double transformed[]={b.track.x-shear*b.track.h/2,b.track.y,b.track.w+shear*b.track.h,b.track.h};const double expected[]={c->x,c->y,c->w,c->h};for(int k=0;k<4;k++)CHECK(fabs(transformed[k]-expected[k])<.002);
 CHECK(b.border==c->border);double label[]={b.label.x,b.label.y,b.label.w,b.label.h};for(int k=0;k<4;k++)CHECK(fabs(label[k]-c->label[k])<.001);CHECK(b.track.w-2*b.border>0&&b.track.h-2*b.border>0);
}
CHECK(rr_boost_label_color()==UINT32_C(0x7fb9d0ff));RRBoostMeter meter={0};int sequence=-1;
for(size_t i=0;i<sizeof(render_cases)/sizeof(render_cases[0]);i++){
 const BoostRenderCase *c=&render_cases[i];if(sequence!=c->sequence){meter=(RRBoostMeter){0};sequence=c->sequence;}rr_boost_meter_observe(&meter,c->fill,c->on!=0,c->now);rr_boost_meter_render(&meter,c->rendered!=0,c->now);RRBoostStyle s=rr_boost_meter_present_style(&meter,c->now);
 CHECK(meter.on==(c->on!=0)&&meter.low==(c->low!=0)&&meter.animation==c->animation);double error=fmax(fabs(s.opacity-c->opacity),fabs(s.brightness-c->brightness));if(error>worst)worst=error;if(error>2e-9){if(failures<5)fprintf(stderr,"Render row%zu error%.12g\n",i,error);failures++;}
}
/* Same-class reappearance restarts, but gradient-only on changes during low
 * do not restart its animation while visible. */
meter=(RRBoostMeter){0};rr_boost_meter_observe(&meter,.1,true,10);rr_boost_meter_render(&meter,true,10);rr_boost_meter_render(&meter,false,10.2);rr_boost_meter_render(&meter,true,11);CHECK(meter.started==11);CHECK(rr_boost_meter_present_style(&meter,11).opacity==1);rr_boost_meter_observe(&meter,.1,false,11.1);CHECK(meter.started==11);CHECK(rr_boost_meter_present_style(&meter,11.1).brightness==1);rr_boost_meter_observe(&meter,.5,true,11.2);CHECK(meter.started==11.2&&meter.animation==RR_BOOST_BURNING);
printf("Boost layout: two reused Chrome track/caption boxes and %zu source DOM/spec-model states across16 sequences; visibility restart and class precedence; worst style error%.12g, %d failures; no graphics\n",sizeof(render_cases)/sizeof(render_cases[0]),worst,failures);return failures?1:0;}

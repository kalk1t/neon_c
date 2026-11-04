#include "rr_mouse_widgets.h"
#include <stdio.h>
typedef struct WidgetSource {int kind;float w,h;double value;int checked;double center;} WidgetSource;
typedef struct WidgetColor {int kind;uint32_t rgba;} WidgetColor;
typedef struct CallbackCase {int step;double sensitivity;int invert;} CallbackCase;
#include "mouse_widgets_fixture.h"
#include "settings_callback_fixture.h"
static int failures;static double thumb_error;
#define CHECK(x) do{if(!(x)){if(failures<8)fprintf(stderr,"mouse widgets line%d\n",__LINE__);failures++;}}while(0)
static RRMouseWidgetEvent event(double x,double y,bool pressed,bool released,bool held){return (RRMouseWidgetEvent){x,y,true,true,pressed,released,held};}
int main(void)
{
    RRMouseWidgetBoxes b=rr_mouse_widget_boxes(100,200,80);CHECK(b.slider.w==96&&b.slider.h==16&&b.checkbox.w==13&&b.checkbox.h==13);CHECK(b.checkbox.x-b.slider.x==116&&b.checkbox.y-b.slider.y==2.5f);
    for(size_t i=0;i<sizeof(widget_sources)/sizeof(widget_sources[0]);i++){
        const WidgetSource *s=&widget_sources[i];RRUIRect box=s->kind?b.checkbox:b.slider;CHECK(box.w==s->w&&box.h==s->h);
        if(!s->kind){double e=fabs((rr_mouse_widget_thumb(box,s->value)-box.x)-s->center);if(e>thumb_error)thumb_error=e;CHECK(e<.75);}
    }
    for(size_t i=0;i<sizeof(widget_colors)/sizeof(widget_colors[0]);i++)CHECK(rr_mouse_widget_rgba(widget_colors[i].kind)==widget_colors[i].rgba);
    RRMouseWidgetInput g={0};RRMouseSettings settings={1,false};
    for(size_t i=0;i<sizeof(callback_cases)/sizeof(callback_cases[0]);i++){
        const CallbackCase *c=&callback_cases[i];double x=b.slider.x+8+(b.slider.w-16)*c->step/44;
        /* Each fixture row clicks a track from a far-away value to avoid the
         * deliberately preserved thumb-grab offset, then sends source values. */
        settings.sensitivity=c->step<22?2.5:.3;
        settings.sensitivity=rr_mouse_widget_slider(&g,b.slider,settings.sensitivity,event(x,b.slider.y+8,true,true,false));CHECK(fabs(settings.sensitivity-c->sensitivity)<1e-12&&!g.dragging);
        if(settings.invert!=(c->invert!=0)){
            RRMouseWidgetEvent e=event(b.invert_hit.x+50,b.invert_hit.y+10,true,false,true);CHECK(!rr_mouse_widget_checkbox(&g,b.invert_hit,e));e.pressed=false;e.released=true;e.held=false;if(rr_mouse_widget_checkbox(&g,b.invert_hit,e))settings.invert=!settings.invert;
        }
        CHECK(settings.invert==(c->invert!=0));
    }
    double value=1.4,center=rr_mouse_widget_thumb(b.slider,value);value=rr_mouse_widget_slider(&g,b.slider,value,event(center+2,b.slider.y+8,true,false,true));CHECK(fabs(value-1.4)<1e-12);
    value=rr_mouse_widget_slider(&g,b.slider,value,event(center+2+80.0/44,b.slider.y+8,false,false,true));CHECK(fabs(value-1.45)<1e-12);
    value=rr_mouse_widget_slider(&g,b.slider,value,event(center+2+160.0/44,b.slider.y+8,false,true,false));CHECK(fabs(value-1.5)<1e-12&&!g.dragging);
    value=rr_mouse_widget_slider(&g,b.slider,value,event(-100,b.slider.y+8,true,false,true));CHECK(fabs(value-1.5)<1e-12&&!g.dragging);
    value=rr_mouse_widget_slider(&g,b.slider,value,event(b.slider.x+8,b.slider.y+8,true,false,true));CHECK(fabs(value-.3)<1e-12);RRMouseWidgetEvent lost=event(10000,0,false,false,true);lost.focused=false;value=rr_mouse_widget_slider(&g,b.slider,value,lost);CHECK(fabs(value-.3)<1e-12&&!g.dragging);
    CHECK(!rr_mouse_widget_checkbox(&g,b.invert_hit,event(b.invert_hit.x+20,b.invert_hit.y+10,false,true,false)));
    CHECK(!rr_mouse_widget_checkbox(&g,b.invert_hit,event(b.invert_hit.x+20,b.invert_hit.y+10,true,false,true)));CHECK(!rr_mouse_widget_checkbox(&g,b.invert_hit,event(0,0,false,true,false)));
    CHECK(!rr_mouse_widget_checkbox(&g,b.invert_hit,event(b.invert_hit.x+20,b.invert_hit.y+10,true,false,true)));rr_mouse_widgets_cancel(&g);CHECK(!rr_mouse_widget_checkbox(&g,b.invert_hit,event(b.invert_hit.x+20,b.invert_hit.y+10,false,true,false)));
    printf("Mouse widgets:5 reused Chromium samples/colors,45 actual browser callback states; native grab/release/focus/cancel policies; max raster-center bias %.9g, %d failures; no GPU/DOM\n",thumb_error,failures);return failures?1:0;
}

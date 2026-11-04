#include "rr_race_ui.h"
#include "rr_results_layout.h"
#include "results_layout_fixture.h"
#include <stdio.h>
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"results line%d: %s\n",__LINE__,#x);failures++;}}while(0)
static int near(double a,double b){return fabs(a-b)<.002;}
int main(void)
{
    for(int i=0;i<8;i++){RRUIRect a=rr_results_box(i),b=result_boxes[i];CHECK(near(a.x,b.x)&&near(a.y,b.y)&&near(a.w,b.w)&&near(a.h,b.h));}
    CHECK(rr_results_box(-1).w==0&&rr_results_box(8).w==0);
    const float views[][2]={{1600,1000},{1280,720},{1920,1080},{800,600},{680,500},{1600,400}};
    for(size_t j=0;j<sizeof(views)/sizeof(views[0]);j++){
        float w=views[j][0],h=views[j][1];RRUIOverlayLayout l=rr_ui_results_layout(w,h);
        CHECK(near(l.panel.w,680)&&near(l.panel.h,491.1875));
        CHECK(near(l.panel.x+(l.panel.w/2),w/2));
        CHECK(near(l.panel.y,fmax(16,(h-491.1875)/2)));
        CHECK(near(l.left.x-l.panel.x,32)&&near(l.left.y-l.panel.y,result_boxes[4].y));
        /* Independent retained Chromium computed grid tracks: 1.3fr/1fr, gap22. */
        CHECK(near(l.left.w,335.734)&&near(l.right.w,258.25));
        CHECK(near(l.right.x-l.left.x-l.left.w,22));
        for(int i=0;i<2;i++){RRUIRect r=result_boxes[6+i];CHECK(near(l.actions[i].x-l.panel.x,r.x)&&near(l.actions[i].y-l.panel.y,r.y)&&near(l.actions[i].w,r.w)&&near(l.actions[i].h,r.h));}
        CHECK(l.left.y+l.left.h<l.actions[0].y&&l.actions[1].y+l.actions[1].h<l.panel.y+l.panel.h);
    }
    /* Content-height changes affect row positions by half the changed width,
       preserving a centered pair and the source28px gap. No fixed +/-150. */
    for(int a=30;a<=300;a+=30)for(int b=40;b<=320;b+=40){double x=rr_results_lap_left(800,a,b);CHECK(near((x+x+a+28+b)/2,800));CHECK(near((x+a+28)-(x+a),28));}
    CHECK(rr_results_baseline(0,34,900)>24&&rr_results_baseline(0,34,900)<56.296875);
    CHECK(rr_results_baseline(2,32,900)>107.296875&&rr_results_baseline(2,32,900)<147.296875);
    printf("Results layout:8 reused Chrome boxes,6 viewports,80 variable-width lap pairs, fixed offline placeholders; %d failures, no GPU\n",failures);
    return failures?1:0;
}

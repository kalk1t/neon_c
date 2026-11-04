#include "rr_button_hover.h"
#include <stdio.h>
typedef struct HoverCase {int sequence,frame;double now;int scope;unsigned present,hover;double expected[8];} HoverCase;
#include "button_hover_fixture.h"
static int failures;static double worst,worst_float;
#define CHECK(x) do{if(!(x)){if(failures<8)fprintf(stderr,"hover line%d\n",__LINE__);failures++;}}while(0)
int main(void)
{
    RRButtonHover h={0};int sequence=-1;
    for(size_t i=0;i<sizeof(hover_cases)/sizeof(hover_cases[0]);i++){
        const HoverCase *c=&hover_cases[i];if(sequence!=c->sequence){sequence=c->sequence;h=(RRButtonHover){0};}
        rr_button_hover_begin(&h,c->scope,c->now);
        for(int j=0;j<8;j++)if(c->present&(1u<<j)){
            double value=rr_button_hover_sample(&h,j,(c->hover&(1u<<j))!=0),e=fabs(value-c->expected[j]),f=fabs((float)value-c->expected[j]);if(e>worst)worst=e;if(f>worst_float)worst_float=f;
            CHECK(e<5e-9&&f<1e-6&&value>=0&&value<=1);
        }
        rr_button_hover_end(&h);for(int j=0;j<8;j++)if(!(c->present&(1u<<j)))CHECK(!h.slots[j].rendered);
    }
    /* First style on a mounted owner snaps; existing rendered control eases.
       New screen reusing the same numeric slot must not inherit another owner. */
    h=(RRButtonHover){0};rr_button_hover_begin(&h,0,0);CHECK(rr_button_hover_sample(&h,0,false)==0);rr_button_hover_end(&h);
    rr_button_hover_begin(&h,0,.01);CHECK(rr_button_hover_sample(&h,0,true)==0);rr_button_hover_end(&h);
    rr_button_hover_begin(&h,0,.06);double active=rr_button_hover_sample(&h,0,true);CHECK(active>0&&active<1);rr_button_hover_end(&h);
    rr_button_hover_begin(&h,2,.061);CHECK(rr_button_hover_sample(&h,0,true)==1);rr_button_hover_end(&h);
    rr_button_hover_begin(&h,2,.07);rr_button_hover_end(&h);rr_button_hover_begin(&h,2,.08);CHECK(rr_button_hover_sample(&h,0,false)==0);
    CHECK(rr_button_hover_sample(&h,-1,true)==1&&rr_button_hover_sample(&h,8,false)==0);
    printf("Button hover:6400 independent CSS150ms model frames,51200 slot/lifecycle checks; worst %.12g, float %.12g, controller bytes %zu, %d failures; no engine/GPU\n",worst,worst_float,sizeof(h),failures);return failures?1:0;
}

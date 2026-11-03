#include <limits.h>
#include <stdio.h>
#include "rr_render_policy.h"

static int failures;
static void check(bool ok,const char *message){if(!ok){fprintf(stderr,"FAIL: %s\n",message);failures++;}}
static bool visible(RRVec3 center,double radius,double aspect)
{
    return rr_render_sphere_visible(center,radius,rr_v3(0,0,0),rr_v3(0,0,-1),
        rr_v3(1,0,0),rr_v3(0,1,0),1,aspect,1000);
}
int main(void)
{
    const int sizes[][2]={{960,600},{960,720},{1440,900},{3840,2160},{7680,4320},
        {600,1000},{1,INT_MAX},{INT_MAX,1},{INT_MAX,INT_MAX}};
    for(unsigned int i=0;i<sizeof(sizes)/sizeof(sizes[0]);i++){
        RRRenderSize size=rr_render_size(sizes[i][0],sizes[i][1]);
        check(size.width>0 && size.height>0 && size.width<=960 && size.height<=600,"bounded nonzero target even at extreme sizes");
        double scale=fmin(1,fmin(960.0/sizes[i][0],600.0/sizes[i][1]));
        check(fabs(size.width-sizes[i][0]*scale)<=1 && fabs(size.height-sizes[i][1]*scale)<=1,"aspect preserved to pixel rounding");
    }
    check(rr_render_size(0,900).width==0 && rr_render_size(900,-1).height==0,"invalid/minimized target rejected");
    check(visible(rr_v3(0,0,-100),1,1),"forward object retained");
    check(!visible(rr_v3(0,0,100),1,1),"rear object rejected");
    check(!visible(rr_v3(200,0,-100),1,1),"outside horizontal frustum rejected");
    check(!visible(rr_v3(0,200,-100),1,1),"outside vertical frustum rejected");
    check(visible(rr_v3(101,0,-100),2,1),"object crossing side plane retained");
    check(visible(rr_v3(0,0,0),10,1),"object surrounding camera retained");
    check(!visible(rr_v3(0,0,-1010),1,1),"beyond far plane rejected");
    check(visible(rr_v3(0,0,-1001),2,1),"object crossing far plane retained");
    check(visible(rr_v3(150,0,-100),1,2),"wide aspect expands horizontal view");
    check(rr_render_sphere_visible(rr_v3(100,0,0),1,rr_v3(0,0,0),rr_v3(1,0,0),
        rr_v3(0,0,1),rr_v3(0,1,0),1,1,1000),"rotated camera retains its forward object");
    int slow=0;
    for(int i=0;i<9;i++)check(!rr_render_overloaded(.2,&slow),"short slowdown does not pause");
    check(rr_render_overloaded(.2,&slow),"sustained slowdown pauses");
    check(!rr_render_overloaded(.03,&slow) && slow==0,"normal frame resets slowdown streak");
    puts("Render bounds, camera culling, and overload policy checked without graphics.");
    return failures?1:0;
}

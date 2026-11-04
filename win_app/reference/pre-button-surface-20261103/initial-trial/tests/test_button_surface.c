#include "rr_button_surface.h"
#include <stdio.h>
typedef struct ButtonSource {float w,h;int primary;double opacity;} ButtonSource;
typedef struct ButtonPixel {int source;float x,y;int kind;} ButtonPixel;
typedef struct ButtonColor {int source;float x,y;int r,g,b,a;} ButtonColor;
#include "button_surface_fixture.h"
static int failures,old_mismatches;static double color_max[4];
#define CHECK(x) do{if(!(x)){fprintf(stderr,"button surface line%d\n",__LINE__);failures++;}}while(0)
static double cross(float ax,float ay,float bx,float by){return (double)ax*by-(double)ay*bx;}
static int contains(RRButtonVertex p[3],float x,float y)
{double a=cross(p[1].x-p[0].x,p[1].y-p[0].y,x-p[0].x,y-p[0].y),b=cross(p[2].x-p[1].x,p[2].y-p[1].y,x-p[1].x,y-p[1].y),c=cross(p[0].x-p[2].x,p[0].y-p[2].y,x-p[2].x,y-p[2].y);return (a>=-1e-5&&b>=-1e-5&&c>=-1e-5)||(a<=1e-5&&b<=1e-5&&c<=1e-5);}
static int previous_kind(ButtonSource s,float x,float y)
{
    if((x>=14&&x<=s.w&&y>=-1&&y<=1)||(x>=s.w-1&&x<=s.w+1&&y>=0&&y<=s.h-14)||(x>=0&&x<=s.w-14&&y>=s.h-1&&y<=s.h+1)||(x>=-1&&x<=1&&y>=14&&y<=s.h))return 2;
    if(x<0||x>=s.w)return 0;float sx=floorf(x/4)*4,sw=fminf(4,s.w-sx),top=fmaxf(0,14-sx),bottom=fmaxf(0,14-(s.w-sx-sw));return y>=floorf(top)&&y<floorf(top)+floorf(s.h-top-bottom)?1:0;
}
int main(void)
{
    for(size_t i=0;i<sizeof(button_pixels)/sizeof(button_pixels[0]);i++){
        const ButtonPixel *c=&button_pixels[i];ButtonSource s=button_sources[c->source];int found=0;
        for(int j=0;j<14;j++){RRButtonVertex p[3];int kind=rr_button_surface_triangle((RRUIRect){0,0,s.w,s.h},j,p);if(kind&&contains(p,c->x,c->y)&&kind>found)found=kind;}
        CHECK(found==c->kind);if(previous_kind(s,c->x,c->y)!=c->kind)old_mismatches++;
    }
    for(size_t i=0;i<sizeof(button_colors)/sizeof(button_colors[0]);i++){
        const ButtonColor *c=&button_colors[i];ButtonSource s=button_sources[c->source];RRButtonRGBA v=rr_button_surface_rgba(s.primary!=0,s.opacity==1,1,(c->x-2)/(s.w-4));
        const double model[4]={v.r,v.g,v.b,v.a},actual[4]={c->r*c->a/255.0,c->g*c->a/255.0,c->b*c->a/255.0,c->a};
        for(int j=0;j<4;j++){double e=fabs(model[j]-actual[j]);if(e>color_max[j])color_max[j]=e;CHECK(e<1.5);}
    }
    RRButtonVertex p[3];CHECK(!rr_button_surface_triangle((RRUIRect){0,0,27,61},0,p)&&!rr_button_surface_triangle((RRUIRect){0,0,316,27},0,p));CHECK(!rr_button_surface_triangle((RRUIRect){0,0,316,61},14,p));CHECK(!rr_button_surface_triangle((RRUIRect){NAN,0,316,61},0,p));
    printf("Button surface: %zu reused Chrome mask pixels, %zu premultiplied RGBA samples,14 triangles/button; old model %d mask mismatches; color maxima %.9g/%.9g/%.9g/%.9g, %d failures; no GPU\n",sizeof(button_pixels)/sizeof(button_pixels[0]),sizeof(button_colors)/sizeof(button_colors[0]),old_mismatches,color_max[0],color_max[1],color_max[2],color_max[3],failures);return failures?1:0;
}

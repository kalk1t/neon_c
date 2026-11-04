#include "rr_minimap_background.h"
#include <stdio.h>
typedef struct PixelCase {double x,y;unsigned char rgba[4];} PixelCase;
#include "map_background_fixture.h"
static int failures;
#define CHECK(v) do{if(!(v)){if(failures<5)fprintf(stderr,"line%d: %s\n",__LINE__,#v);failures++;}}while(0)
/* Derive raylib's native36-fan outer barycentric weight from each sector's
 * opposite edge. This models ideal channel interpolation, not GPU blending. */
static double weight(double x,double y){const double pi=3.14159265358979323846,step=pi/18;double angle=atan2(y,x);if(angle<0)angle+=2*pi;double normal=(floor(angle/step)+.5)*step;return (x*cos(normal)+y*sin(normal))/(120*cos(step*.5));}
int main(void){uint32_t start=rr_minimap_background_vertex(false),end=rr_minimap_background_vertex(true);CHECK(start==UINT32_C(0x0a1446cc));CHECK(end==UINT32_C(0x050923d5));double worst[4]={0},old_worst[4]={0};const double old_start[]={10,20,70,205},old_end[]={3,5,22,220};
for(size_t i=0;i<sizeof(pixel_cases)/sizeof(pixel_cases[0]);i++){
 const PixelCase *p=&pixel_cases[i];double t=weight(p->x,p->y);CHECK(t>=0&&t<=1);
 for(int k=0;k<4;k++){
  int shift=24-k*8;double a=(start>>shift)&255,b=(end>>shift)&255,value=a+(b-a)*t,previous=old_start[k]+(old_end[k]-old_start[k])*t;
  double error=fabs(value-p->rgba[k]),old_error=fabs(previous-p->rgba[k]);if(error>worst[k])worst[k]=error;if(old_error>old_worst[k])old_worst[k]=old_error;CHECK(error<=2);
 }
}
CHECK(worst[2]<old_worst[2]/4);CHECK(worst[3]<old_worst[3]);
printf("Minimap background: %zu reused Chrome interior RGBA pixels, unchanged36-fan model; new max %.12g/%.12g/%.12g/%.12g, previous %.12g/%.12g/%.12g/%.12g; %d failures; no graphics\n",sizeof(pixel_cases)/sizeof(pixel_cases[0]),worst[0],worst[1],worst[2],worst[3],old_worst[0],old_worst[1],old_worst[2],old_worst[3],failures);return failures?1:0;}

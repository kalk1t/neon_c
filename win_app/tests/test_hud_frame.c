#include "rr_hud_panel_shape.h"
#include <stdio.h>
typedef struct PanelCase {float w,h;} PanelCase;
typedef struct FramePixel {int panel;double x,y;int kind;} FramePixel;
#include "hud_frame_fixture.h"
static int failures;
#define CHECK(v) do{if(!(v)){if(failures<5)fprintf(stderr,"line%d: %s\n",__LINE__,#v);failures++;}}while(0)
static double area(const RRPanelVertex p[3]){return ((double)p[1].x-p[0].x)*(p[2].y-p[0].y)-((double)p[1].y-p[0].y)*(p[2].x-p[0].x);}
static bool contains(const RRPanelVertex p[3],double x,double y){for(int i=0;i<3;i++){int j=(i+1)%3;double cross=(p[j].x-p[i].x)*(y-p[i].y)-(p[j].y-p[i].y)*(x-p[i].x);if(cross>1e-7)return false;}return true;}
static int old_kind(double w,double h,double x,double y){const double c=18;
 if((x>=c&&x<=w-c&&(fabs(y)<=1||fabs(y-h)<=1))||(y>=c&&y<=h-c&&(fabs(x)<=1||fabs(x-w)<=1)))return 2;
 if(x<0||x>=w)return 0;double sx=4*floor(x/4),sw=fmin(4,w-sx),chop=fmax(0,fmax(c-sx,c-(w-sx-sw)));return y>=floor(chop)&&y<floor(chop)+floor(h-2*chop)?1:0;
}
int main(void){RRPanelVertex triangles[2][16][3];int kinds[2][16];
for(int panel=0;panel<2;panel++)for(int i=0;i<16;i++){kinds[panel][i]=rr_hud_panel_triangle((RRUIRect){0,0,panel_cases[panel].w,panel_cases[panel].h},1,i,triangles[panel][i]);CHECK(kinds[panel][i]==(i<8?1:2));CHECK(area(triangles[panel][i])<0);}
size_t old_mismatches=0;
for(size_t i=0;i<sizeof(frame_pixels)/sizeof(frame_pixels[0]);i++){
 const FramePixel *p=&frame_pixels[i];int kind=0;for(int t=0;t<16;t++)if(contains(triangles[p->panel][t],p->x,p->y))kind=kinds[p->panel][t];
 if(kind!=p->kind){if(failures<5)fprintf(stderr,"Mask row%zu x%.1f y%.1f got%d expected%d\n",i,p->x,p->y,kind,p->kind);failures++;}
 if(old_kind(panel_cases[p->panel].w,panel_cases[p->panel].h,p->x,p->y)!=p->kind)old_mismatches++;
}
CHECK(old_mismatches>1000);
/* Independent clipped-rectangle area bounds across source/runtime sizes.
 * These compact cases are analytic, not captured browser raster geometry. */
const float widths[]={142,230,300,400},heights[]={144,153,201},scales[]={1,.8f};
for(int w=0;w<4;w++)for(int h=0;h<3;h++)for(int s=0;s<2;s++){
 float z=scales[s];RRUIRect r={13.25f,7.5f,widths[w]*z,heights[h]*z};double fill=0,border=0,b=floor(2*z),cut=18*z;
 for(int i=0;i<16;i++){
  RRPanelVertex p[3];int kind=rr_hud_panel_triangle(r,z,i,p);double a=-area(p)/2;CHECK(a>0);if(kind==1)fill+=a;else border+=a;
  for(int j=0;j<3;j++){CHECK(p[j].x>=r.x-.0001&&p[j].x<=r.x+r.w+.0001&&p[j].y>=r.y-.0001&&p[j].y<=r.y+r.h+.0001);double t=fmax(0,fmin(1,(p[j].y-r.y-b)/(r.h-2*b)));CHECK(fabs(p[j].gradient-t)<1e-6);}
 }
 CHECK(fabs(fill-((double)r.w*r.h-2*cut*cut))<.02);CHECK(fabs(border-(2*b*(r.w+r.h)-8*b*cut+4*b*b))<.02);
}
RRPanelVertex p[3];CHECK(rr_hud_panel_triangle((RRUIRect){0,0,300,153},1,-1,p)==0);CHECK(rr_hud_panel_triangle((RRUIRect){0,0,300,153},1,16,p)==0);CHECK(rr_hud_panel_triangle((RRUIRect){0,0,30,30},1,0,p)==0);CHECK(rr_hud_panel_triangle((RRUIRect){0,0,300,153},NAN,0,p)==0);
printf("HUD frame: %zu reused Chrome mask pixels, 24 analytic sizes; 16 triangles/panel; previous model%zu mismatches, new%d failures; no graphics\n",sizeof(frame_pixels)/sizeof(frame_pixels[0]),old_mismatches,failures);return failures?1:0;}

#include "rr_arrow_art.h"
#include "rr_target_guidance.h"
#include <stdio.h>
typedef struct ArrowCase {double nx,ny,nz,w,h,zoom,left,top,angle,corners[8];} ArrowCase;
#include "arrow_layout_fixture.h"
#include "arrow_pixels_fixture.h"
static int failures;static double worst;
#define CHECK(v) do{if(!(v)){if(failures<5)fprintf(stderr,"line%d: %s\n",__LINE__,#v);failures++;}}while(0)
int main(int argc,char **argv){if(argc!=2)return 1;SetTraceLogLevel(LOG_WARNING);int bytes=0;unsigned char *data=LoadFileData(argv[1],&bytes);if(!data)return 1;CHECK(rr_arrow_png_header(data,bytes));CHECK(bytes<=RR_ARROW_MAX_FILE_BYTES);
unsigned char bad[33];memcpy(bad,data,33);bad[19]=171;CHECK(!rr_arrow_png_header(bad,33));bad[19]=170;bad[25]=2;CHECK(!rr_arrow_png_header(bad,33));CHECK(!rr_arrow_png_header(data,32));CHECK(!rr_arrow_png_header(data,RR_ARROW_MAX_FILE_BYTES+1));CHECK(!rr_arrow_png_header(NULL,0));
Image image=LoadImageFromMemory(".png",data,bytes);UnloadFileData(data);if(!image.data)return 1;CHECK(image.width==RR_ARROW_WIDTH&&image.height==RR_ARROW_HEIGHT&&image.format==PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);CHECK(image.width*image.height*4==RR_ARROW_RGBA_BYTES);Color *pixels=LoadImageColors(image);if(!pixels){UnloadImage(image);return 1;}
unsigned long long sums[4]={0};int ink=0,clear=0;for(int i=0;i<image.width*image.height;i++){sums[0]+=pixels[i].r;sums[1]+=pixels[i].g;sums[2]+=pixels[i].b;sums[3]+=pixels[i].a;if(pixels[i].a)ink++;else clear++;}for(int k=0;k<4;k++)CHECK(sums[k]==expected_sums[k]);CHECK(ink==expected_ink&&clear==expected_clear);UnloadImageColors(pixels);UnloadImage(image);
for(size_t i=0;i<sizeof(arrow_cases)/sizeof(arrow_cases[0]);i++){
 const ArrowCase *c=&arrow_cases[i];RRTargetGuidance g=rr_target_guidance(c->nx,c->ny,c->nz,c->w,c->h,c->zoom,240);CHECK(!g.on);CHECK(fabs(g.x-c->left)<1e-8&&fabs(g.y-c->top)<1e-8&&fabs(g.angle-c->angle)<1e-12);
 RRArrowLayout a=rr_arrow_layout(g.x,g.y,c->zoom,g.angle);double rad=a.degrees*3.14159265358979323846/180,cs=cos(rad),sn=sin(rad);
 for(int corner=0;corner<4;corner++){double x=(corner==1||corner==2?a.w:0)-a.origin_x,y=(corner>=2?a.h:0)-a.origin_y;double got[2]={a.cx+x*cs-y*sn,a.cy+x*sn+y*cs};for(int k=0;k<2;k++){double error=fabs(got[k]-c->corners[corner*2+k]);if(error>worst)worst=error;CHECK(error<1e-8);}}
}
RRArrowLayout zero=rr_arrow_layout(800,500,1,0);CHECK(zero.cx-zero.origin_x==736&&zero.cy-zero.origin_y==436);CHECK(zero.origin_x==85&&zero.origin_y==94);
printf("Edge arrow: %zu actual-source offscreen transform cases, verified CPU PNG sums and127840-byte RGBA bound; max corner error%.12g, %d failures; no window/upload/GPU\n",sizeof(arrow_cases)/sizeof(arrow_cases[0]),worst,failures);return failures?1:0;}

#include "rr_posters.h"
#include <math.h>
#include <string.h>
#include "generated_posters.h"
static double random5(uint32_t *state)
{
    uint32_t t=(*state+=0x6d2b79f5u);t=(t^(t>>15))*(1u|t);t^=t+(t^(t>>7))*(61u|t);return (double)(t^(t>>14))/4294967296.0;
}
int rr_poster_count(void){return (int)(sizeof(poster_entries)/sizeof(*poster_entries));}
const RRPoster *rr_poster_get(int index){return index>=0&&index<rr_poster_count()?&poster_entries[index]:NULL;}
bool rr_poster_expand(int index,bool double_sided,float (*vertices)[8],size_t capacity)
{
    const RRPoster *p=rr_poster_get(index);size_t count=double_sided?12u:6u;
    if(!p||!vertices||capacity<count)return false;
    for(int i=0;i<6;i++)for(int j=0;j<8;j++)if(!isfinite(p->rows[i][j]))return false;
    memcpy(vertices,p->rows,sizeof(p->rows));
    if(double_sided)for(int triangle=0;triangle<2;triangle++)for(int i=0;i<3;i++){
        int source=triangle*3+2-i,destination=6+triangle*3+i;memcpy(vertices[destination],p->rows[source],sizeof(p->rows[source]));
        for(int k=3;k<6;k++)vertices[destination][k]=-vertices[destination][k];
    }return true;
}
bool rr_poster_glyph(int quadrant,int glyph,double points[3][2])
{
    if(quadrant<0||quadrant>=4||glyph<0||glyph>=7||!points)return false;uint32_t seed=5;
    for(int q=0;q<=quadrant;q++)for(int k=0;k<7;k++){
        double cx=(q%2)*256+50+random5(&seed)*156,cy=(q/2)*512+60+k*62,x2=cx+30*(random5(&seed)-.5)*2;
        if(q==quadrant&&k==glyph){points[0][0]=cx-30;points[0][1]=cy;points[1][0]=x2;points[1][1]=cy+28;points[2][0]=cx+34;points[2][1]=cy-6;return true;}
    }return false;
}
RRPosterMaterial rr_poster_material(void)
{
    RRPosterMaterial m={512,1024,8,true,true,true,true,false,true,true,true,true,false,true,true,1.5,1};return m;
}
size_t rr_poster_rgba_mip_bytes(int width,int height)
{
    if(width<=0||height<=0||width>4096||height>4096)return 0;size_t bytes=0;
    for(;;){bytes+=(size_t)width*(size_t)height*4;if(width==1&&height==1)break;width=width>1?width/2:1;height=height>1?height/2:1;}return bytes;
}

bool rr_poster_sample(double u,double v,RRPosterSample *out)
{
    if(!out||!isfinite(u)||!isfinite(v))return false;
    u=fmin(1,fmax(0,u));v=1-fmin(1,fmax(0,v));double x=u*512-.5,y=v*1024-.5,ix=floor(x),iy=floor(y);
    *out=(RRPosterSample){u,v,(int)fmax(0,fmin(511,ix)),(int)fmax(0,fmin(511,ix+1)),(int)fmax(0,fmin(1023,iy)),(int)fmax(0,fmin(1023,iy+1)),x-ix,y-iy};return true;
}

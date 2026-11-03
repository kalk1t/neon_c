#include "rr_city_occlusion.h"
/* Chosen analysis guards, not a proof of production GPU rounding equivalence. */
static const double screen_guard=.01;
static double depth_guard(double z){return 1+32*z*z/(.3*8388608.0);}
int rr_occlusion_clip(RRVec3 a,RRVec3 b,RRVec3 c,const double planes[6][4],double margin,RRVec3 polygon[16])
{
    RRVec3 next[16];polygon[0]=a;polygon[1]=b;polygon[2]=c;int count=3;
    for(int plane=0;plane<6;plane++){
        const double *p=planes[plane];int out=0;RRVec3 previous=polygon[count-1];double dp=p[0]*previous.x+p[1]*previous.y+p[2]*previous.z+p[3]+margin;
        for(int i=0;i<count;i++){
            RRVec3 current=polygon[i];double dc=p[0]*current.x+p[1]*current.y+p[2]*current.z+p[3]+margin;
            if((dp>=0)!=(dc>=0)){if(out>=16)return -1;next[out++]=rr_mix(previous,current,dp/(dp-dc));}
            if(dc>=0){if(out>=16)return -1;next[out++]=current;}
            previous=current;dp=dc;
        }
        count=out;if(!count)return 0;for(int i=0;i<count;i++)polygon[i]=next[i];
    }
    return count;
}
void rr_occlusion_clear(RROcclusion *b){b->covered=0;for(int i=0;i<RR_OCCLUSION_W*RR_OCCLUSION_H;i++)b->depth[i]=INFINITY;}
bool rr_occlusion_project(RRVec3 p,const double m[16],RROcclusionPoint *out)
{
    double w=m[3]*p.x+m[7]*p.y+m[11]*p.z+m[15];if(!isfinite(w)||w<1)return false;
    out->x=(m[0]*p.x+m[4]*p.y+m[8]*p.z+m[12])/w;out->y=(m[1]*p.x+m[5]*p.y+m[9]*p.z+m[13])/w;out->depth=w;return isfinite(out->x)&&isfinite(out->y);
}
static double edge(RROcclusionPoint a,RROcclusionPoint b,double x,double y){return (b.x-a.x)*(y-a.y)-(b.y-a.y)*(x-a.x);}
bool rr_occlusion_tile_depth(const RROcclusionPoint t[3],double x0,double y0,double x1,double y1,double *maximum)
{
    double area=edge(t[0],t[1],t[2].x,t[2].y);if(!isfinite(area)||fabs(area)<1e-14)return false;double sign=area>0?1:-1,maxz=0;
    for(int i=0;i<3;i++)if(!isfinite(t[i].depth)||t[i].depth<1)return false;
    for(int corner=0;corner<4;corner++){
        double x=(corner&1)?x1:x0,y=(corner&2)?y1:y0;
        for(int e=0;e<3;e++){RROcclusionPoint a=t[e],b=t[(e+1)%3];double length=hypot(b.x-a.x,b.y-a.y);if(sign*edge(a,b,x,y)<screen_guard*length)return false;}
        double a=edge(t[1],t[2],x,y)/area,b=edge(t[2],t[0],x,y)/area,c=1-a-b;
        double inverse=a/t[0].depth+b/t[1].depth+c/t[2].depth;if(!isfinite(inverse)||inverse<=0)return false;double z=1/inverse;if(z>maxz)maxz=z;
    }
    *maximum=maxz+depth_guard(maxz);return true;
}
static int clampi(int n,int high){return n<0?0:n>=high?high-1:n;}
void rr_occlusion_raster(RROcclusion *b,const RROcclusionPoint t[3])
{
    double minx=fmin(t[0].x,fmin(t[1].x,t[2].x)),maxx=fmax(t[0].x,fmax(t[1].x,t[2].x)),miny=fmin(t[0].y,fmin(t[1].y,t[2].y)),maxy=fmax(t[0].y,fmax(t[1].y,t[2].y));
    int x0=clampi((int)floor((minx+1)*.5*RR_OCCLUSION_W),RR_OCCLUSION_W),x1=clampi((int)floor((maxx+1)*.5*RR_OCCLUSION_W),RR_OCCLUSION_W),y0=clampi((int)floor((miny+1)*.5*RR_OCCLUSION_H),RR_OCCLUSION_H),y1=clampi((int)floor((maxy+1)*.5*RR_OCCLUSION_H),RR_OCCLUSION_H);
    for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++){double z;double left=-1+2.0*x/RR_OCCLUSION_W,bottom=-1+2.0*y/RR_OCCLUSION_H;if(rr_occlusion_tile_depth(t,left,bottom,left+2.0/RR_OCCLUSION_W,bottom+2.0/RR_OCCLUSION_H,&z)){int i=y*RR_OCCLUSION_W+x;if(z<b->depth[i]){if(!isfinite(b->depth[i]))b->covered++;b->depth[i]=z;}}}
}
bool rr_occlusion_hidden(const RROcclusion *b,const RROcclusionPoint *p,int count)
{
    if(count<3)return false;double minx=INFINITY,maxx=-INFINITY,miny=INFINITY,maxy=-INFINITY,near=INFINITY;
    for(int i=0;i<count;i++){if(!isfinite(p[i].x)||!isfinite(p[i].y)||!isfinite(p[i].depth)||p[i].depth<1)return false;minx=fmin(minx,p[i].x);maxx=fmax(maxx,p[i].x);miny=fmin(miny,p[i].y);maxy=fmax(maxy,p[i].y);near=fmin(near,p[i].depth);}
    int x0=clampi((int)floor((minx-screen_guard+1)*.5*RR_OCCLUSION_W),RR_OCCLUSION_W),x1=clampi((int)floor((maxx+screen_guard+1)*.5*RR_OCCLUSION_W),RR_OCCLUSION_W),y0=clampi((int)floor((miny-screen_guard+1)*.5*RR_OCCLUSION_H),RR_OCCLUSION_H),y1=clampi((int)floor((maxy+screen_guard+1)*.5*RR_OCCLUSION_H),RR_OCCLUSION_H);
    double guarded=near-depth_guard(near);for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++)if(!isfinite(b->depth[y*RR_OCCLUSION_W+x])||b->depth[y*RR_OCCLUSION_W+x]>=guarded)return false;return true;
}
void rr_occlusion_raster_quad(RROcclusion *b,const RROcclusionPoint q[4])
{
    double area=0,maxdepth=0,minx=INFINITY,maxx=-INFINITY,miny=INFINITY,maxy=-INFINITY;
    for(int i=0;i<4;i++){if(!isfinite(q[i].x)||!isfinite(q[i].y)||!isfinite(q[i].depth)||q[i].depth<1)return;area+=q[i].x*q[(i+1)%4].y-q[(i+1)%4].x*q[i].y;maxdepth=fmax(maxdepth,q[i].depth);minx=fmin(minx,q[i].x);maxx=fmax(maxx,q[i].x);miny=fmin(miny,q[i].y);maxy=fmax(maxy,q[i].y);}
    if(fabs(area)<1e-14)return;double sign=area>0?1:-1;
    for(int i=0;i<4;i++)if(sign*edge(q[i],q[(i+1)%4],q[(i+2)%4].x,q[(i+2)%4].y)<1e-14)return;
    int x0=clampi((int)floor((minx+1)*.5*RR_OCCLUSION_W),RR_OCCLUSION_W),x1=clampi((int)floor((maxx+1)*.5*RR_OCCLUSION_W),RR_OCCLUSION_W),y0=clampi((int)floor((miny+1)*.5*RR_OCCLUSION_H),RR_OCCLUSION_H),y1=clampi((int)floor((maxy+1)*.5*RR_OCCLUSION_H),RR_OCCLUSION_H);double z=maxdepth+depth_guard(maxdepth);
    for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++){
        bool covered=true;
        for(int c=0;c<4&&covered;c++){double px=-1+2.0*(x+(c&1))/RR_OCCLUSION_W,py=-1+2.0*(y+((c>>1)&1))/RR_OCCLUSION_H;for(int e=0;e<4;e++){double length=hypot(q[(e+1)%4].x-q[e].x,q[(e+1)%4].y-q[e].y);if(sign*edge(q[e],q[(e+1)%4],px,py)<screen_guard*length){covered=false;break;}}}
        int index=y*RR_OCCLUSION_W+x;if(covered&&z<b->depth[index]){if(!isfinite(b->depth[index]))b->covered++;b->depth[index]=z;}
    }
}

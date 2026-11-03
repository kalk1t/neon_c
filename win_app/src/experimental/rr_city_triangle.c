#include "rr_city_triangle.h"
static RRVec3 cross(RRVec3 a,RRVec3 b){return rr_v3(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x);}
bool rr_triangle_front(RRVec3 a,RRVec3 b,RRVec3 c,RRVec3 eye)
{
    RRVec3 normal=cross(rr_sub(b,a),rr_sub(c,a)),delta=rr_sub(eye,a);
    /* Retain ambiguous edge-on faces; no production GPU-rounding claim. */
    return rr_dot(normal,delta)>=-1e-9*(rr_length(normal)*rr_length(delta)+1);
}
static double distance(RRVec3 v,const double p[4],double margin){return p[0]*v.x+p[1]*v.y+p[2]*v.z+p[3]+margin;}
bool rr_triangle_frustum(RRVec3 a,RRVec3 b,RRVec3 c,const double planes[6][4],double margin)
{
    if(!planes||!isfinite(margin)||margin<0)return false;
    RRVec3 polygon[16],next[16];polygon[0]=a;polygon[1]=b;polygon[2]=c;int count=3;
    for(int p=0;p<6;p++){
        for(int k=0;k<4;k++)if(!isfinite(planes[p][k]))return false;
        int out=0;RRVec3 previous=polygon[count-1];double dp=distance(previous,planes[p],margin);
        for(int i=0;i<count;i++){
            RRVec3 current=polygon[i];double dc=distance(current,planes[p],margin);
            if((dp>=0)!=(dc>=0)){if(out>=16)return true;next[out++]=rr_mix(previous,current,dp/(dp-dc));}
            if(dc>=0){if(out>=16)return true;next[out++]=current;}
            previous=current;dp=dc;
        }
        count=out;if(!count)return false;for(int i=0;i<count;i++)polygon[i]=next[i];
    }
    return true;
}

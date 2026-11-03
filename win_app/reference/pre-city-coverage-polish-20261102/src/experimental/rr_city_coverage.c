#include "rr_city_coverage.h"
#include <math.h>
#include <string.h>
#define PIECES 128
#define VERTICES 32
typedef struct V {double x,y,q;} V;
typedef struct Poly {V p[VERTICES];int n;} Poly;
typedef struct Plane {double a,b,c,d;} Plane;
static double value(Plane p,V v){return p.a*v.x+p.b*v.y+p.c+p.d*v.q;}
static V mix(V a,V b,double t){V o={a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t,a.q+(b.q-a.q)*t};return o;}
/* Guarded half-plane clipping. Overflow/invalid arithmetic aborts the proof. */
static bool clip(const Poly *in,Plane p,bool positive,Poly *out)
{
    out->n=0;V previous=in->p[in->n-1];double dp=value(p,previous);if(!positive)dp=-dp;
    for(int i=0;i<in->n;i++){V current=in->p[i];double dc=value(p,current);if(!positive)dc=-dc;if(!isfinite(dp)||!isfinite(dc))return false;
        if((dp>=0)!=(dc>=0)){double t=dp/(dp-dc);if(!isfinite(t)||t<0||t>1||out->n==VERTICES)return false;out->p[out->n++]=mix(previous,current,t);}
        if(dc>=0){if(out->n==VERTICES)return false;out->p[out->n++]=current;}previous=current;dp=dc;
    }return true;
}
bool rr_coverage_union(const RROcclusionPoint *p,int count,const RROcclusionPoint (*blockers)[3],int number,unsigned int *peak)
{
    Poly a[PIECES],b[PIECES];int n=1;if(peak)*peak=1;if(!p||!blockers||count<3||count>16||number<1||number>5)return false;
    a[0].n=count;double area=0;for(int i=0;i<count;i++){if(!isfinite(p[i].x)||!isfinite(p[i].y)||!isfinite(p[i].depth)||p[i].depth<1)return false;a[0].p[i]=(V){p[i].x,p[i].y,1/p[i].depth};area+=p[i].x*p[(i+1)%count].y-p[(i+1)%count].x*p[i].y;}if(!isfinite(area)||fabs(area)<1e-14)return false;
    for(int k=0;k<number;k++){
        const RROcclusionPoint *t=blockers[k];bool valid=true;for(int j=0;j<3;j++)if(!isfinite(t[j].x)||!isfinite(t[j].y)||!isfinite(t[j].depth)||t[j].depth<1)valid=false;if(!valid)continue;
        double det=(t[1].x-t[0].x)*(t[2].y-t[0].y)-(t[1].y-t[0].y)*(t[2].x-t[0].x);if(!isfinite(det)||fabs(det)<1e-14)continue;
        Plane planes[4];for(int e=0;e<3;e++){int i=(e+1)%3,j=(e+2)%3;planes[e]=(Plane){(t[i].y-t[j].y)/det,(t[j].x-t[i].x)/det,(t[i].x*t[j].y-t[i].y*t[j].x)/det-1e-7,0};}
        planes[3]=(Plane){0,0,-1e-6,-1};for(int e=0;e<3;e++){double q=1/t[e].depth;planes[3].a+=planes[e].a*q;planes[3].b+=planes[e].b*q;planes[3].c+=(planes[e].c+1e-7)*q;}
        int next=0;for(int j=0;j<n;j++){
            Poly inside=a[j];for(int e=0;e<4&&inside.n>=3;e++){Poly outside,part;if(!clip(&inside,planes[e],false,&outside)||!clip(&inside,planes[e],true,&part))return false;
                /* Never erase a small positive-area remainder by an area threshold. */
                if(outside.n>=3){if(next==PIECES)return false;b[next++]=outside;}inside=part;
            }
        }
        n=next;if(peak&&(unsigned int)n>*peak)*peak=(unsigned int)n;if(!n)return true;memcpy(a,b,(size_t)n*sizeof(*a));
    }return false;
}

#include "rr_city_witness.h"
#include <stdlib.h>
#include <string.h>
static RRVec3 cross(RRVec3 a,RRVec3 b){return rr_v3(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x);}
static RRVec3 point(const RRWitnessScene *s,unsigned int i){float p[3];memcpy(p,s->geometry+(size_t)i*32,12);return rr_v3(p[0],p[1],p[2]);}
static int axis;
static double component(RRVec3 p,int a){return a==0?p.x:a==1?p.y:p.z;}
static int compare_part(const void *a,const void *b){unsigned int x=*(const unsigned int *)a,y=*(const unsigned int *)b;const RRRepackPart *p=rr_repack_part((int)x),*q=rr_repack_part((int)y);double px=component(rr_add(p->minimum,p->maximum),axis),qx=component(rr_add(q->minimum,q->maximum),axis);return px<qx?-1:px>qx?1:x<y?-1:x>y?1:0;}
static int build(RRWitnessScene *s,int start,int count)
{
    if(s->count==s->capacity)return -1;int index=s->count++;RRWitnessNode *n=&s->nodes[index];n->minimum=rr_v3(INFINITY,INFINITY,INFINITY);n->maximum=rr_v3(-INFINITY,-INFINITY,-INFINITY);n->left=n->right=-1;n->start=start;n->count=count;
    for(int i=start;i<start+count;i++){const RRRepackPart *p=rr_repack_part((int)s->parts[i]);n->minimum.x=fmin(n->minimum.x,p->minimum.x);n->minimum.y=fmin(n->minimum.y,p->minimum.y);n->minimum.z=fmin(n->minimum.z,p->minimum.z);n->maximum.x=fmax(n->maximum.x,p->maximum.x);n->maximum.y=fmax(n->maximum.y,p->maximum.y);n->maximum.z=fmax(n->maximum.z,p->maximum.z);}
    if(count<=4)return index;RRVec3 extent=rr_sub(n->maximum,n->minimum);axis=extent.x>=extent.y&&extent.x>=extent.z?0:extent.y>=extent.z?1:2;qsort(s->parts+start,(size_t)count,sizeof(*s->parts),compare_part);int half=count/2;
    n->count=0;n->left=build(s,start,half);n->right=build(s,start+half,count-half);return n->left<0||n->right<0?-1:index;
}
bool rr_witness_scene_init(RRWitnessScene *s,const unsigned char *geometry)
{
    if(!s||!geometry)return false;memset(s,0,sizeof(*s));s->geometry=geometry;s->capacity=rr_repack_part_count()*2;s->nodes=calloc((size_t)s->capacity,sizeof(*s->nodes));s->parts=malloc((size_t)rr_repack_part_count()*sizeof(*s->parts));if(!s->nodes||!s->parts){rr_witness_scene_free(s);return false;}
    for(int i=0;i<rr_repack_part_count();i++){
        s->parts[i]=(unsigned int)i;const RRRepackPart *p=rr_repack_part(i);
        for(unsigned int j=0;j<p->vertices;j++){RRVec3 v=point(s,p->first+j);if(!isfinite(v.x)||!isfinite(v.y)||!isfinite(v.z)||v.x<p->minimum.x||v.x>p->maximum.x||v.y<p->minimum.y||v.y>p->maximum.y||v.z<p->minimum.z||v.z>p->maximum.z){rr_witness_scene_free(s);return false;}}
    }
    if(build(s,0,rr_repack_part_count())!=0){rr_witness_scene_free(s);return false;}return true;
}
void rr_witness_scene_free(RRWitnessScene *s){if(!s)return;free(s->parts);free(s->nodes);memset(s,0,sizeof(*s));}
bool rr_witness_triangle(RRVec3 o,RRVec3 d,RRVec3 a,RRVec3 b,RRVec3 c,bool two,double *distance,bool *ambiguous)
{
    if(ambiguous)*ambiguous=false;RRVec3 e1=rr_sub(b,a),e2=rr_sub(c,a),h=cross(d,e2);double determinant=rr_dot(e1,h),tolerance=1e-12*(rr_length(e1)*rr_length(e2)+1);
    if(fabs(determinant)<=tolerance){if(ambiguous)*ambiguous=true;return false;}
    if(!two&&determinant<0)return false;double inverse=1/determinant;RRVec3 s=rr_sub(o,a);double u=inverse*rr_dot(s,h);if(u< -1e-10||u>1+1e-10)return false;
    RRVec3 q=cross(s,e1);double v=inverse*rr_dot(d,q);if(v< -1e-10||u+v>1+1e-10)return false;
    double t=inverse*rr_dot(e2,q);if(t<0||!isfinite(t))return false;*distance=t;return true;
}
static bool box(RRVec3 lo,RRVec3 hi,RRVec3 o,RRVec3 d,double maximum)
{
    double near=0,far=maximum;
    for(int a=0;a<3;a++){double l=component(lo,a),h=component(hi,a),origin=component(o,a),direction=component(d,a),pad=1e-6+2e-7*fmax(fabs(l),fabs(h));l-=pad;h+=pad;
        if(fabs(direction)<1e-18){if(origin<l||origin>h)return false;continue;}
        double t0=(l-origin)/direction,t1=(h-origin)/direction;if(t0>t1){double swap=t0;t0=t1;t1=swap;}near=fmax(near,t0);far=fmin(far,t1);if(near>far)return false;
    }return true;
}
static void cast_part(const RRWitnessScene *s,const RRWitnessView *view,int part,RRVec3 o,RRVec3 d,double maximum,unsigned int exclude,RRWitnessHit *out)
{
    const RRRepackPart *p=rr_repack_part(part);bool visible=p->source_batch<32?((view->lo>>(unsigned int)p->source_batch)&1u)!=0:((view->hi>>(unsigned int)(p->source_batch-32))&1u)!=0;if(!visible)return;
    for(unsigned int j=0;j<p->vertices;j+=3){unsigned int first=p->first+j;if(first==exclude)continue;RRVec3 a=point(s,first),b=point(s,first+1),c=point(s,first+2);double distance;bool uncertain=false;
        if(rr_witness_triangle(o,d,a,b,c,p->material==4,&distance,&uncertain)&&distance<=out->distance){RRVec3 hit=rr_add(o,rr_scale(d,distance));double z=view->mvp[3]*hit.x+view->mvp[7]*hit.y+view->mvp[11]*hit.z+view->mvp[15];
            if(z<.29||z>22000.01)continue;if(z<.31){out->ambiguous=true;continue;}
            out->hit=true;out->distance=distance;out->triangle=first;
        }else if(uncertain){RRVec3 lo=rr_v3(fmin(a.x,fmin(b.x,c.x)),fmin(a.y,fmin(b.y,c.y)),fmin(a.z,fmin(b.z,c.z))),hi=rr_v3(fmax(a.x,fmax(b.x,c.x)),fmax(a.y,fmax(b.y,c.y)),fmax(a.z,fmax(b.z,c.z)));if(box(lo,hi,o,d,maximum))out->ambiguous=true;}
    }
}
static void cast_node(const RRWitnessScene *s,const RRWitnessView *v,int index,RRVec3 o,RRVec3 d,double maximum,unsigned int exclude,RRWitnessHit *out)
{
    const RRWitnessNode *n=&s->nodes[index];if(!box(n->minimum,n->maximum,o,d,out->distance))return;
    if(n->count){for(int i=n->start;i<n->start+n->count;i++)cast_part(s,v,(int)s->parts[i],o,d,maximum,exclude,out);}
    else{cast_node(s,v,n->left,o,d,maximum,exclude,out);cast_node(s,v,n->right,o,d,maximum,exclude,out);}
}
RRWitnessHit rr_witness_cast(const RRWitnessScene *s,const RRWitnessView *v,RRVec3 o,RRVec3 d,double maximum,unsigned int exclude,bool brute)
{
    RRWitnessHit result={false,false,maximum,0};if(!s||!v||!isfinite(maximum)||maximum<=0){result.ambiguous=true;return result;}
    if(brute)for(int i=0;i<rr_repack_part_count();i++)cast_part(s,v,i,o,d,maximum,exclude,&result);else cast_node(s,v,0,o,d,maximum,exclude,&result);return result;
}

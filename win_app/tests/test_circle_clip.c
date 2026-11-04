#include "rr_circle_clip.h"
#include <stdio.h>
typedef struct ClipCase {double radius;RRCirclePoint triangle[3];double exact_area,area_bound;int marker;} ClipCase;
#include "circle_clip_fixture.h"
static double cross(RRCirclePoint a,RRCirclePoint b,RRCirclePoint c){return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);}
int main(void){int failures=0,max_vertices=0,max_marker_vertices=0;double worst_marker_loss=0;
for(size_t row=0;row<sizeof(clip_cases)/sizeof(clip_cases[0]);row++){
const ClipCase *c=&clip_cases[row];RRCirclePoint polygon[RR_CIRCLE_CLIP_MAX];int count=rr_circle_clip_triangle(c->triangle,0,0,c->radius,polygon);
if(count<0||count>131){failures++;continue;}if(count>max_vertices)max_vertices=count;if(c->marker&&count>max_marker_vertices)max_marker_vertices=count;
double area=0;for(int i=0;i<count;i++){
 RRCirclePoint p=polygon[i],q=polygon[(i+1)%count];area+=p.x*q.y-p.y*q.x;
 if(hypot(p.x,p.y)>c->radius+1e-7)failures++;
 double sign=cross(c->triangle[0],c->triangle[1],c->triangle[2]);
 for(int j=0;j<3;j++)if(cross(c->triangle[j],c->triangle[(j+1)%3],p)*sign < -1e-5)failures++;
}
area=fabs(area)*.5;double loss=c->exact_area-area;
if(loss < -1e-6||loss>c->area_bound+1e-6){if(failures<8)fprintf(stderr,"Area row%zu loss%.17g bound%.17g\n",row,loss,c->area_bound);failures++;}
if(c->marker&&loss>worst_marker_loss)worst_marker_loss=loss;
}
RRCirclePoint p[3]={{0,0},{1,0},{0,1}},out[RR_CIRCLE_CLIP_MAX];if(rr_circle_clip_triangle(p,0,0,0,out)!=0)failures++;p[0].x=NAN;if(rr_circle_clip_triangle(p,0,0,1,out)!=-1)failures++;
RRCirclePoint edge[3]={{117,-5},{122,5},{110,5}},shifted[3],second[RR_CIRCLE_CLIP_MAX];
for(int j=0;j<3;j++)shifted[j]=(RRCirclePoint){edge[j].x+1000,edge[j].y-300};
int first=rr_circle_clip_triangle(edge,0,0,118,out),other=rr_circle_clip_triangle(shifted,1000,-300,118,second);
if(first<3||first!=other)failures++;else for(int j=0;j<first;j++)if(hypot(out[j].x-(second[j].x-1000),out[j].y-(second[j].y+300))>1e-8)failures++;
printf("Circle clipping: %zu analytic area cases, max vertices%d/marker%d, worst sampled marker area loss%.17g, %d failures;128-gon radial gap at118px %.17g\n",sizeof(clip_cases)/sizeof(clip_cases[0]),max_vertices,max_marker_vertices,worst_marker_loss,failures,118*(1-RR_CIRCLE_APOTHEM));return failures?1:0;}

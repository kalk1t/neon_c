#include "rr_minimap_marks.h"
#include <stdio.h>
typedef struct MarkCase {int next,mask,index;double x,z,vx,vy,size,expected[4];uint32_t rgba;double start[4];} MarkCase;
typedef struct GeometryCase {double clip,x,y,radius,stroke;int outline;double exact,bound;} GeometryCase;
typedef struct RectCase {double clip,x,y,exact,bound;} RectCase;
#include "minimap_marks_fixture.h"
#include "minimap_marks_geometry.h"
static double cross(RRCirclePoint a,RRCirclePoint b,RRCirclePoint c){return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);}
int main(void){int failures=0,max_vertices=0;double worst_error=0;RRMinimap map={200,100,732};
for(size_t n=0;n<sizeof(mark_cases)/sizeof(mark_cases[0]);n++){
 const MarkCase *c=&mark_cases[n];RRMapMark mark=rr_minimap_checkpoint(&map,c->x,c->z,c->vx,c->vy,c->size,c->index==c->next,(c->mask&(1<<c->index))!=0);
 double got[]={mark.x,mark.y,mark.radius,mark.stroke};for(int i=0;i<4;i++)if(fabs(got[i]-c->expected[i])>1e-10)failures++;
 if(mark.rgba!=c->rgba||mark.next!=(c->index==c->next))failures++;
 RRCirclePoint start[3];const double *s=c->start;RRCirclePoint rect[4]={{s[0],s[1]},{s[0],s[1]+s[3]},{s[0]+s[2],s[1]+s[3]},{s[0]+s[2],s[1]}};
 for(int i=0;i<2;i++){if(!rr_minimap_start_triangle(&map,c->vx,c->vy,c->size,i,start)||cross(start[0],start[1],start[2])>=0)failures++;
   RRCirclePoint expected[]={rect[0],rect[i+1],rect[i+2]};for(int j=0;j<3;j++)if(hypot(start[j].x-expected[j].x,start[j].y-expected[j].y)>1e-10)failures++;
 }
}
for(size_t n=0;n<sizeof(geometry_cases)/sizeof(geometry_cases[0]);n++){
 const GeometryCase *c=&geometry_cases[n];RRMapMark mark={c->x,c->y,c->radius,c->stroke,0,true};double sum=0;int triangles=c->outline?128:64;
 for(int i=0;i<triangles;i++){
   RRCirclePoint source[3],clipped[RR_CIRCLE_CLIP_MAX];if(!rr_minimap_mark_triangle(mark,c->outline!=0,i,source)){failures++;continue;}
   if(cross(source[0],source[1],source[2])>=0)failures++;
   int count=rr_circle_clip_triangle(source,0,0,c->clip,clipped);if(count<0){failures++;continue;}if(count>max_vertices)max_vertices=count;
   for(int j=0;j<count;j++){
     if(hypot(clipped[j].x,clipped[j].y)>c->clip+1e-9)failures++;
     double radial=hypot(clipped[j].x-c->x,clipped[j].y-c->y),outer=c->radius+(c->outline?c->stroke*.5:0),inner=c->outline?fmax(0,c->radius-c->stroke*.5):0;
     if(radial>outer+1e-9||radial+1e-9<inner*cos(RR_PI/64))failures++;
   }
   for(int j=1;j+1<count;j++){double area=cross(clipped[0],clipped[j],clipped[j+1])*.5;if(area>1e-9)failures++;sum-=area;}
 }
 double error=fabs(sum-c->exact);if(error>worst_error)worst_error=error;
 if(!isfinite(sum)||error>c->bound+1e-7){if(failures<5)fprintf(stderr,"Geometry%zu area%.17g exact%.17g bound%.17g\n",n,sum,c->exact,c->bound);failures++;}
}
for(size_t n=0;n<sizeof(rect_cases)/sizeof(rect_cases[0]);n++){
 const RectCase *c=&rect_cases[n];double size=2*c->clip;RRMinimap local={-c->x/(.82*size),-c->y/(.82*size),1};double sum=0;
 for(int i=0;i<2;i++){
   RRCirclePoint source[3],out[RR_CIRCLE_CLIP_MAX];if(!rr_minimap_start_triangle(&local,-c->clip,-c->clip,size,i,source)){failures++;continue;}
   int count=rr_circle_clip_triangle(source,0,0,c->clip,out);if(count<0){failures++;continue;}
   for(int j=1;j+1<count;j++)sum-=cross(out[0],out[j],out[j+1])*.5;
 }
 if(fabs(sum-c->exact)>c->bound+1e-7)failures++;
}
/* All nine actual roads stay represented. Mark rendering cannot alter progress. */
static RRTrack track;int max_gates=0,max_source_triangles=0,max_output_triangles=0;for(int road=1;road<=RR_ROAD_COUNT;road++){
 if(!rr_track_init_road(&track,road)){failures++;continue;}rr_minimap_init(&map,&track);if(track.gate_count>max_gates)max_gates=track.gate_count;
 for(int next=0;next<=track.gate_count;next++){
   int input=0,output=0;
   for(int i=0;i<track.gate_count;i++){
     RRMapMark mark=rr_minimap_checkpoint(&map,track.gates[i].pos.x,track.gates[i].pos.z,2,2,236,i==next,i<next);if(!isfinite(mark.x)||!isfinite(mark.y))failures++;
     for(int layer=0;layer<(mark.next?2:1);layer++)for(int j=0;j<(layer?128:64);j++){
       RRCirclePoint triangle[3],clipped[RR_CIRCLE_CLIP_MAX];if(!rr_minimap_mark_triangle(mark,layer!=0,j,triangle)){failures++;continue;}
       int count=rr_circle_clip_triangle(triangle,120,120,118,clipped);if(count<0){failures++;continue;}input++;if(count>=3)output+=count-2;
       for(int k=0;k<count;k++)if(hypot(clipped[k].x-120,clipped[k].y-120)>118+1e-9)failures++;
     }
   }
   for(int i=0;i<2;i++){RRCirclePoint triangle[3],clipped[RR_CIRCLE_CLIP_MAX];rr_minimap_start_triangle(&map,2,2,236,i,triangle);int count=rr_circle_clip_triangle(triangle,120,120,118,clipped);if(count<0){failures++;continue;}input++;if(count>=3)output+=count-2;}
   if(input>max_source_triangles)max_source_triangles=input;if(output>max_output_triangles)max_output_triangles=output;
 }
}
RRCirclePoint invalid[3];RRMapMark mark={0,0,1,.3,0,false};if(rr_minimap_mark_triangle(mark,true,0,invalid)||rr_minimap_mark_triangle(mark,false,64,invalid)||rr_minimap_start_triangle(&map,0,0,236,2,invalid))failures++;
printf("Browser minimap marks: %zu style, %zu analytic circle/annulus, %zu rectangle cases, all nine roads; max clipped vertices%d, max gates%d, worst area error%.17g, %d failures\n",sizeof(mark_cases)/sizeof(mark_cases[0]),sizeof(geometry_cases)/sizeof(geometry_cases[0]),sizeof(rect_cases)/sizeof(rect_cases[0]),max_vertices,max_gates,worst_error,failures);
printf("Actual roads desktop markers: max input triangles%d, max output triangles%d; state%zu bytes, caller clip%zu bytes, no new textures/tables/heap\n",max_source_triangles,max_output_triangles,sizeof(RRMapMark),sizeof(RRCirclePoint)*RR_CIRCLE_CLIP_MAX);return failures?1:0;}

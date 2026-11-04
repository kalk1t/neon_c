#ifndef RR_MINIMAP_MARKS_H
#define RR_MINIMAP_MARKS_H
#include "rr_minimap.h"
#include "rr_circle_clip.h"
#include <stdint.h>
#define RR_MAP_MARK_SEGMENTS 64
typedef struct RRMapMark {double x,y,radius,stroke;uint32_t rgba;bool next;} RRMapMark;
/* HUD Canvas is440 square; CSS scales both radii and the centered3px stroke.
 * Boolean passed is supplied by race state, never written by presentation. */
static inline RRMapMark rr_minimap_checkpoint(const RRMinimap *map,double x,double z,double vx,double vy,double size,bool next,bool passed)
{
    double px,py;rr_minimap_point(map,x,z,&px,&py);
    return (RRMapMark){vx+px*size,vy+py*size,(next?11:7)*size/440,3*size/440,passed?UINT32_C(0x19e6ffff):UINT32_C(0x2bff7aff),next};
}
/* Inscribed64-gon fill; annulus uses matching inner/outer64-gons. Reuse the
 * existing clipper unit normals, so no new table/trigonometry/heap is needed.
 * Output winding matches raylib DrawTriangle's screen-space front face. */
static inline bool rr_minimap_mark_triangle(RRMapMark mark,bool outline,int index,RRCirclePoint out[3])
{
    int count=outline?2*RR_MAP_MARK_SEGMENTS:RR_MAP_MARK_SEGMENTS;
    if(!out||index<0||index>=count||!isfinite(mark.x)||!isfinite(mark.y)||!isfinite(mark.radius)||!isfinite(mark.stroke)||mark.radius<=0||mark.stroke<0|| (outline&&!mark.next))return false;
    int segment=outline?index/2:index;RRCirclePoint a=rr_circle_normals[2*segment],b=rr_circle_normals[2*((segment+1)%RR_MAP_MARK_SEGMENTS)];
    double outer=mark.radius+(outline?mark.stroke*.5:0),inner=outline?fmax(0,mark.radius-mark.stroke*.5):0;
    RRCirclePoint ao={mark.x+a.x*outer,mark.y+a.y*outer},bo={mark.x+b.x*outer,mark.y+b.y*outer};
    RRCirclePoint ai={mark.x+a.x*inner,mark.y+a.y*inner},bi={mark.x+b.x*inner,mark.y+b.y*inner};
    if(!outline){out[0]=(RRCirclePoint){mark.x,mark.y};out[1]=bo;out[2]=ao;}
    else if(index%2==0){out[0]=ao;out[1]=ai;out[2]=bi;}
    else{out[0]=ao;out[1]=bi;out[2]=bo;}
    return true;
}
/* Source fillRect(sx-12,sy-3,24,6), with two triangles before mask clipping. */
static inline bool rr_minimap_start_triangle(const RRMinimap *map,double vx,double vy,double size,int index,RRCirclePoint out[3])
{
    if(!out||index<0||index>1||!isfinite(size)||size<=0)return false;
    double px,py;rr_minimap_point(map,0,0,&px,&py);double x=vx+px*size,y=vy+py*size,dx=12*size/440,dy=3*size/440;
    RRCirclePoint points[4]={{x-dx,y-dy},{x-dx,y+dy},{x+dx,y+dy},{x+dx,y-dy}};
    out[0]=points[0];out[1]=points[index+1];out[2]=points[index+2];return true;
}
#endif

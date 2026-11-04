#ifndef RR_BUTTON_SURFACE_H
#define RR_BUTTON_SURFACE_H
#include "rr_ui_layout.h"
#include <math.h>
typedef struct RRButtonVertex {float x,y,gradient;} RRButtonVertex;
typedef struct RRButtonRGBA {double r,g,b,a;} RRButtonRGBA;
/* CSS clipped border-box,14px opposing cuts and2px inside rectangular border.
 * Fill omits border-covered pixels, preserving disabled group opacity without
 * a new render target. Six fill fan triangles plus eight border triangles. */
static inline int rr_button_surface_triangle(RRUIRect r,int index,RRButtonVertex out[3])
{
    if(!out||index<0||index>=14||!isfinite(r.x)||!isfinite(r.y)||!isfinite(r.w)||!isfinite(r.h)||r.w<28||r.h<28)return 0;
    const float p[6][2]={{12,2},{r.w-2,2},{r.w-2,r.h-12},{r.w-12,r.h-2},{2,r.h-2},{2,12}};float points[3][2];int kind=1;
    if(index<6){points[0][0]=r.w*.5f;points[0][1]=r.h*.5f;int next=(index+1)%6;for(int j=0;j<2;j++){points[1][j]=p[next][j];points[2][j]=p[index][j];}}
    else{
        const float q[4][4][2]={{{14,0},{r.w,0},{r.w,2},{12,2}},{{r.w-2,2},{r.w,2},{r.w,r.h-14},{r.w-2,r.h-12}},{{0,r.h-2},{r.w-12,r.h-2},{r.w-14,r.h},{0,r.h}},{{0,14},{2,12},{2,r.h-2},{0,r.h-2}}};
        const int indices[2][3]={{0,2,1},{0,3,2}};int face=(index-6)/2,second=index%2;kind=2;
        for(int i=0;i<3;i++)for(int j=0;j<2;j++)points[i][j]=q[face][indices[second][i]][j];
    }
    for(int i=0;i<3;i++){float t=fmaxf(0,fminf(1,(points[i][0]-2)/(r.w-4)));out[i]=(RRButtonVertex){r.x+points[i][0],r.y+points[i][1],t};}return kind;
}
/* Premultiplied sRGB vertex channels,0..255. CSS default gradient runs across
 * padding-box width. Kept premultiplied through interpolation and GPU blend. */
static inline RRButtonRGBA rr_button_surface_rgba(bool primary,bool enabled,int kind,double t)
{
    double opacity=enabled?1:.4;t=fmax(0,fmin(1,t));
    if(kind==2)return primary?(RRButtonRGBA){255*opacity,43*opacity,214*opacity,255*opacity}:(RRButtonRGBA){25*opacity,230*opacity,255*opacity,255*opacity};
    if(primary)return (RRButtonRGBA){(255*.35*(1-t)+6*.8*t)*opacity,(43*.35*(1-t)+14*.8*t)*opacity,(214*.35*(1-t)+46*.8*t)*opacity,255*(.35*(1-t)+.8*t)*opacity};
    return (RRButtonRGBA){6*.8*opacity,14*.8*opacity,46*.8*opacity,255*.8*opacity};
}
#endif

#ifndef RR_HUD_PANEL_SHAPE_H
#define RR_HUD_PANEL_SHAPE_H
#include "rr_ui_layout.h"
#include <math.h>
typedef struct RRPanelVertex {float x,y,gradient;} RRPanelVertex;
/* Source .panel: clipped border-box,18px corners,2px inside border. Compact
 * border rounding models supplied DPR1 Chrome zoom. Other DPI is unqualified.
 * Eight fill triangles and four clipped border quads =16 triangles total. */
static inline int rr_hud_panel_triangle(RRUIRect r,float scale,int index,RRPanelVertex out[3])
{
    if(!out||index<0||index>=16||!isfinite(scale)||scale<=0||!isfinite(r.x)||!isfinite(r.y)||!isfinite(r.w)||!isfinite(r.h)||r.w<36*scale||r.h<36*scale)return 0;
    float c=18*scale,b=floorf(2*scale);if(b<1)b=1;
    const float p[8][2]={{c,0},{r.w-c,0},{r.w,c},{r.w,r.h-c},{r.w-c,r.h},{c,r.h},{0,r.h-c},{0,c}};
    float points[3][2];int kind=1;
    if(index<8){points[0][0]=r.w*.5f;points[0][1]=r.h*.5f;int a=(index+1)%8;for(int j=0;j<2;j++){points[1][j]=p[a][j];points[2][j]=p[index][j];}}
    else{
        kind=2;const float q[4][4][2]={
            {{c,0},{r.w-c,0},{r.w-c+b,b},{c-b,b}},
            {{r.w,c},{r.w,r.h-c},{r.w-b,r.h-c+b},{r.w-b,c-b}},
            {{r.w-c,r.h},{c,r.h},{c-b,r.h-b},{r.w-c+b,r.h-b}},
            {{0,r.h-c},{0,c},{b,c-b},{b,r.h-c+b}}};
        int face=(index-8)/2,second=index%2;const int indices[2][3]={{0,2,1},{0,3,2}};
        for(int i=0;i<3;i++)for(int j=0;j<2;j++)points[i][j]=q[face][indices[second][i]][j];
    }
    for(int i=0;i<3;i++){float t=fmaxf(0,fminf(1,(points[i][1]-b)/(r.h-2*b)));out[i]=(RRPanelVertex){r.x+points[i][0],r.y+points[i][1],t};}return kind;
}
#endif

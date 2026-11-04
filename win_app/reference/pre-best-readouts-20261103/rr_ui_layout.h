#ifndef RR_UI_LAYOUT_H
#define RR_UI_LAYOUT_H
#include <stdbool.h>
#include <math.h>
typedef struct RRUIRect { float x,y,w,h; } RRUIRect;
typedef struct RRUIMenuLayout { float x,y,width,brand,button_h,gap; RRUIRect actions[5]; } RRUIMenuLayout;
typedef struct RRUIRoadLayout { RRUIRect panel,list,back; float row_h,stride,content_h,max_scroll; } RRUIRoadLayout;
static inline RRUIMenuLayout rr_ui_menu_layout(float w,float h)
{
    RRUIMenuLayout l={0}; bool compact=h<=820;
    l.x=w*.07f;l.width=fminf(470,w-l.x-24);l.brand=compact?46.f:64.f;
    l.button_h=compact?50.f:60.f;l.gap=compact?9.f:14.f;
    float total=l.brand*1.9f+31+5*(l.button_h+l.gap)+105;
    l.y=fmaxf(16,(h-total)*.5f);
    float top=l.y+l.brand*1.9f+31;
    for(int i=0;i<5;i++)l.actions[i]=(RRUIRect){l.x,top+i*(l.button_h+l.gap),l.width,l.button_h};
    return l;
}
static inline RRUIRoadLayout rr_ui_road_layout(float w,float h)
{
    RRUIRoadLayout l={0}; float ph=fminf(736,h*.96f),pw=fminf(640,w-32);
    l.panel=(RRUIRect){(w-pw)*.5f,(h-ph)*.5f,pw,ph};
    l.list=(RRUIRect){l.panel.x+30,l.panel.y+138,pw-60,ph-232};
    l.back=(RRUIRect){l.list.x,l.panel.y+ph-74,l.list.w,52};
    l.row_h=50;l.stride=56;l.content_h=9*l.stride-6;
    l.max_scroll=fmaxf(0,l.content_h-l.list.h);return l;
}
static inline float rr_ui_scroll_focus(RRUIRoadLayout l,float scroll,int id)
{
    float top=(id-1)*l.stride;
    if(top<scroll)scroll=top;
    if(top+l.row_h>scroll+l.list.h)scroll=top+l.row_h-l.list.h;
    return fmaxf(0,fminf(l.max_scroll,scroll));
}
/* Same clipped top-left/bottom-right corners as browser button clip-path. */
static inline bool rr_ui_hit(RRUIRect r,float px,float py,float cut)
{
    float x=px-r.x,y=py-r.y;
    return x>=0&&y>=0&&x<=r.w&&y<=r.h&&x+y>=cut&&(r.w-x)+(r.h-y)>=cut;
}
#endif

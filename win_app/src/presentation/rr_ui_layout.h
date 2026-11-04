#ifndef RR_UI_LAYOUT_H
#define RR_UI_LAYOUT_H
#include <stdbool.h>
#include <math.h>
#include "rr_menu_footer.h"
typedef struct RRUIRect { float x,y,w,h; } RRUIRect;
typedef struct RRUIMenuLayout { float x,y,width,brand,button_h,gap,best_y,footer_y,brand_line_h,track_y,track_h,content_h; RRUIRect actions[5]; } RRUIMenuLayout;
typedef struct RRUIRoadLayout { RRUIRect panel,list,back; float row_h,stride,content_h,max_scroll; } RRUIRoadLayout;
static inline RRUIMenuLayout rr_ui_menu_layout(float w,float h)
{
    RRUIMenuLayout l={0}; bool compact=h<=820;
    l.x=w*.07f;l.width=fminf(470,w-l.x-24);l.brand=compact?46.f:64.f;
    l.brand_line_h=compact?43.6875f:60.796875f;l.track_h=18;
    l.button_h=compact?50.f:64.f;l.gap=compact?9.f:14.f;
    float track=2*l.brand_line_h+l.gap,top=track+l.track_h+(compact?4.f:14.f)+l.gap;
    const float compact_heights[5]={50,50,50,48,48},desktop_heights[5]={64,64,64,63,61};
    for(int i=0;i<5;i++){float bh=compact?compact_heights[i]:desktop_heights[i];l.actions[i]=(RRUIRect){l.x,top,l.width,bh};top+=bh+(i<4?l.gap:0);}
    l.best_y=top+14;l.footer_y=l.best_y+24;
    /* Native footer uses the source keycap/control style. Its contents are shorter
     * than the browser's full paint/profile/settings/controls card. */
    l.content_h=l.footer_y+rr_menu_footer_height(h);l.y=fmaxf(16,(h-l.content_h)*.5f);
    l.track_y=l.y+track;l.best_y+=l.y;l.footer_y+=l.y;
    for(int i=0;i<5;i++)l.actions[i].y+=l.y;return l;
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

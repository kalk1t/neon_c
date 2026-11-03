#include "rr_ui_layout.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
int main(void)
{
    float sizes[][2]={{960,720},{1280,720},{1920,1080},{1440,960}};
    for(int k=0;k<4;k++){
        float w=sizes[k][0],h=sizes[k][1];RRUIMenuLayout m=rr_ui_menu_layout(w,h);RRUIRoadLayout r=rr_ui_road_layout(w,h);
        assert(fabsf(m.x-w*.07f)<.01f&&m.width==470);
        for(int i=0;i<5;i++){RRUIRect a=m.actions[i];assert(a.x>=0&&a.y>=0&&a.x+a.w<=w&&a.y+a.h<=h-80);if(i)assert(a.y>=m.actions[i-1].y+m.button_h);}
        assert(r.panel.w==640&&r.panel.y>=0&&r.panel.y+r.panel.h<=h);
        assert(r.list.y+r.list.h<r.back.y&&r.back.y+r.back.h<r.panel.y+r.panel.h);
        float scroll=0;
        for(int id=1;id<=9;id++){scroll=rr_ui_scroll_focus(r,scroll,id);float top=r.list.y+(id-1)*r.stride-scroll;assert(top>=r.list.y-.01f&&top+r.row_h<=r.list.y+r.list.h+.01f);}
        assert(rr_ui_scroll_focus(r,scroll,1)==0);
        assert(rr_ui_scroll_focus(r,9999,9)<=r.max_scroll);
    }
    RRUIRect b={100,200,470,50};assert(!rr_ui_hit(b,101,201,14));assert(rr_ui_hit(b,120,220,14));assert(!rr_ui_hit(b,569,249,14));assert(!rr_ui_hit(b,99,220,14));
    puts("UI layout: viewport containment, focus scroll, clipped hit regions passed (4 viewports).");return 0;
}

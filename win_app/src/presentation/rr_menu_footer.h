#ifndef RR_MENU_FOOTER_H
#define RR_MENU_FOOTER_H
#include "rr_css_font_metrics.h"
#include <stddef.h>
typedef struct RRHelpKey {const char *text;float width;} RRHelpKey;
typedef struct RRHelpRun {int key;const char *text;} RRHelpRun;
typedef struct RRHelpBox {float x,y,w,h;} RRHelpBox;
static inline const RRHelpKey *rr_menu_help_key(int index)
{
    static const RRHelpKey keys[]={{"MOUSE",64.125f},{"RMB",45.03125f},{"W",28.15625f},{"S",23.921875f},{"SHIFT",54.609375f},{"F",22.6875f},{"C",23.875f},{"R",23.90625f},{"ESC",42.96875f}};
    return index>=0&&index<9?&keys[index]:NULL;
}
static inline int rr_menu_help_count(int row){return row==0?4:row==1?8:row==2?6:0;}
static inline const RRHelpRun *rr_menu_help_run(int row,int index)
{
    static const RRHelpRun rows[3][8]={
      {{0,NULL},{-1," steer + climb/dive "},{1,NULL},{-1," hold to look around"}},
      {{2,NULL},{-1," thrust "},{3,NULL},{-1," brake "},{4,NULL},{-1," boost "},{5,NULL},{-1," spin dodge"}},
      {{6,NULL},{-1," camera "},{7,NULL},{-1," reset "},{8,NULL},{-1," pause"}}};
    return index>=0&&index<rr_menu_help_count(row)?&rows[row][index]:NULL;
}
static inline float rr_menu_help_line_height(float height){return height<=820?24.f:30.f;}
static inline float rr_menu_help_pending(float height){return 61+3*rr_menu_help_line_height(height)+8;}
static inline float rr_menu_help_error(float height){return rr_menu_help_pending(height)+22;}
static inline float rr_menu_footer_height(float height){return rr_menu_help_error(height)+18;}
static inline double rr_menu_help_baseline(double top,double line_height)
{return rr_css_line_baseline(top,line_height,15,0);}
static inline RRHelpBox rr_menu_help_key_box(int index,float pen,float baseline,float screen_height)
{
    const RRHelpKey *k=rr_menu_help_key(index);if(!k)return (RRHelpBox){0};
    bool compact=screen_height<=820;double from_top=(compact?1:3)+rr_css_line_baseline(0,15,12,700);
    return (RRHelpBox){pen+2,baseline-(float)from_top,k->width,compact?17.f:21.f};
}
static inline double rr_menu_help_advance(const RRHelpRun *run,double text_width)
{const RRHelpKey *k=run?rr_menu_help_key(run->key):NULL;return k?k->width+4:text_width;}
#endif

#ifndef RR_MENU_HEADER_H
#define RR_MENU_HEADER_H
#include "rr_ui_layout.h"
#include "rr_css_font_metrics.h"
static inline double rr_menu_brand_baseline(RRUIMenuLayout l,int line)
{return rr_css_line_baseline(l.y+(line?l.brand_line_h:0),l.brand_line_h,l.brand,900);}
static inline double rr_menu_track_baseline(RRUIMenuLayout l)
{return rr_css_line_baseline(l.track_y,l.track_h,13,700);}
static inline double rr_menu_best_baseline(RRUIMenuLayout l)
{return rr_css_line_baseline(l.best_y,17,14,700);}
#endif

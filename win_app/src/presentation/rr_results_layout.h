#ifndef RR_RESULTS_LAYOUT_H
#define RR_RESULTS_LAYOUT_H
#include "rr_ui_layout.h"
#include "rr_css_font_metrics.h"
/* Prepared source no-next card. Offline placeholders retain this fixed space;
 * this is not variable rival/reward/account row layout or touch-screen parity. */
static inline RRUIRect rr_results_box(int index)
{
    static const RRUIRect boxes[]={
      {32,24,616,32.296875f},{32,65.296875f,616,33},
      {32,107.296875f,616,40},{32,156.296875f,616,20},
      {32,185.296875f,616,113.890625f},{32,308.1875f,616,19},
      {32,336.1875f,616,61},{32,406.1875f,616,61}};
    return index>=0&&index<8?boxes[index]:(RRUIRect){0};
}
static inline double rr_results_baseline(int index,double size,int weight)
{RRUIRect r=rr_results_box(index);return rr_css_line_baseline(r.y,r.h,size,weight);}
static inline double rr_results_lap_left(double center,double width1,double width2)
{return center-(width1+28+width2)*.5;}
#endif

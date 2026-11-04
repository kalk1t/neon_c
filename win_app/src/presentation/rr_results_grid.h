#ifndef RR_RESULTS_GRID_H
#define RR_RESULTS_GRID_H
#include "rr_ui_layout.h"
#include <float.h>
/* QUALIFIED intrinsic tracks/natural column heights. Browser auto-minimum grid
 * sizing and text/float layout must supply these; do not infer them from fixed
 * fractions, line counts or the presence of a relocated float. No font shaping. */
typedef struct RRResultsGridMetrics {double left_width,right_width,left_natural,right_natural;} RRResultsGridMetrics;
typedef struct RRResultsGridLayout {RRUIRect box,left,right;double overflow_x;} RRResultsGridLayout;
/* Source22px gap, max natural height and stretch of both columns. Preserve
 * widths extending beyond the fixed grid box rather than shrinking/dropping.
 * Invalid inputs/derived overflow leave out unchanged. No heap/draw/culling. */
static inline bool rr_results_grid_layout(RRUIRect anchor,const RRResultsGridMetrics *m,RRResultsGridLayout *out)
{
 if(!m||!out||!isfinite(anchor.x)||!isfinite(anchor.y)||!isfinite(anchor.w)||anchor.w<=0||
    !isfinite(m->left_width)||!isfinite(m->right_width)||!isfinite(m->left_natural)||!isfinite(m->right_natural)||
    m->left_width<=0||m->right_width<=0||m->left_natural<0||m->right_natural<0)return false;
 double h=fmax(m->left_natural,m->right_natural),right=(double)anchor.x+m->left_width+22;
 double end=right+m->right_width,overflow=fmax(0,m->left_width+22+m->right_width-anchor.w);
 if(h>FLT_MAX||m->left_width>FLT_MAX||m->right_width>FLT_MAX||!isfinite(end)||fabs(right)>FLT_MAX||fabs(end)>FLT_MAX||!isfinite(overflow)||overflow>FLT_MAX)return false;
 RRResultsGridLayout g={0};g.box=(RRUIRect){anchor.x,anchor.y,anchor.w,(float)h};g.left=(RRUIRect){anchor.x,anchor.y,(float)m->left_width,(float)h};g.right=(RRUIRect){(float)right,anchor.y,(float)m->right_width,(float)h};g.overflow_x=overflow;*out=g;return true;
}
#endif

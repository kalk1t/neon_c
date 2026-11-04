#ifndef RR_BUTTON_TEXT_H
#define RR_BUTTON_TEXT_H
#include "rr_ui_layout.h"
#include "rr_css_font_metrics.h"
#include <stdint.h>
typedef struct RRButtonText {double x,baseline,size,tracking;uint32_t rgba;} RRButtonText;
static inline double rr_menu_button_css_size(double height){return height<=820?18:20;}
/* Font size is CSS em, inset includes the2px border. Hover timing remains
 * the host's existing approximation; these endpoints match prepared styles. */
static inline RRButtonText rr_button_text(RRUIRect box,double css_size,double inset,double hover,bool enabled)
{
    double t=isfinite(hover)?fmax(0,fmin(1,hover)):0;uint32_t red=(uint32_t)lround(221+34*t),alpha=enabled?255u:102u;
    return (RRButtonText){box.x+inset+10*t,rr_css_line_baseline(box.y,box.h,css_size,700),css_size,3,(red<<24)|UINT32_C(0x00ffff00)|alpha};
}
#endif

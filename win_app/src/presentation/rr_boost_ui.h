#ifndef RR_BOOST_UI_H
#define RR_BOOST_UI_H
#include "rr_ui_layout.h"
#include "rr_css_font_metrics.h"
#include <stdint.h>
typedef struct RRBoostUI {RRUIRect track,label;float border;} RRBoostUI;
/* Verified v3 desktop and compact DPR1 CSS-zoom boxes. Caption600 uses the
 * existing closest700 face; exact font raster/weight matching remains open. */
static inline RRBoostUI rr_boost_ui(RRUIRect panel,float zoom)
{
    if(zoom<1)return (RRBoostUI){{panel.x+18.59375f,panel.y+104.375f,146.8125f,6.390625f},{panel.x+18.59375f,panel.y+114.765625f,146.8125f,10},1};
    return (RRBoostUI){{panel.x+24,panel.y+131,182,8},{panel.x+24,panel.y+144,182,12},1};
}
static inline uint32_t rr_boost_label_color(void){return UINT32_C(0x7fb9d0ff);}
#endif

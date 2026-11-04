#ifndef RR_SPIN_READOUT_H
#define RR_SPIN_READOUT_H
#include "rr_css_font_metrics.h"
#define RR_SPIN_KEY_TEXT "DODGE \xc2\xb7 F"
typedef struct RRSpinReadout {double right_inset,center_y,label_baseline,key_top,key_baseline,key_font,key_spacing;} RRSpinReadout;
static inline RRSpinReadout rr_spin_readout(double zoom)
{
    /* Prepared full attack-panel SPIN group: desktop ring top25.5/height74,
     * key top105.5/height13; compact top19.796875/height59.1875, key83.78125/10.
     * Horizontal placement stays in the native partial/offline wrapper. */
    RRSpinReadout s={0};s.right_inset=zoom<1?48.1875:61;s.center_y=zoom<1?49.390625:62.5;s.key_top=zoom<1?83.78125:105.5;
    double label_line=zoom<1?14:18,key_line=zoom<1?10:13;
    s.label_baseline=rr_css_line_baseline(s.center_y-label_line*.5,label_line,14*zoom,900);
    s.key_font=9*zoom;s.key_spacing=1.5*zoom;s.key_baseline=rr_css_line_baseline(s.key_top,key_line,s.key_font,700);return s;
}
#endif

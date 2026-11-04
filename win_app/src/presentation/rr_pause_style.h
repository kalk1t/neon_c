#ifndef RR_PAUSE_STYLE_H
#define RR_PAUSE_STYLE_H
#include <math.h>
#include <stdint.h>
/* Current source desktop gradient: .82 at0%, .3 at32%, transparent at50%.
 * Keep RGB constant when alpha fades: CSS transparent is premultiplied. */
static inline double rr_pause_backdrop_alpha(double unit)
{
    if(!isfinite(unit))return 0;
    if(unit<=0)return .82;if(unit<.32)return .82+(.3-.82)*unit/.32;
    if(unit<.5)return .3*(.5-unit)/.18;return 0;
}
static inline uint32_t rr_pause_backdrop_rgba(double unit)
{return UINT32_C(0x03041400)|(uint32_t)floor(rr_pause_backdrop_alpha(unit)*255+.5);}
static inline uint32_t rr_pause_panel_rgba(void){return UINT32_C(0x04081eb8);}
#endif

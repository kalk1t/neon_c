#ifndef RR_SHOWCASE_INPUT_H
#define RR_SHOWCASE_INPUT_H
#include "rr_showcase.h"
typedef struct RRShowcaseInputFrame {double x,y,wheel;unsigned pressed,released,held;bool active,focused,inside,control;} RRShowcaseInputFrame;
/* Native frame adapter for source pause pointer callbacks. Any sampled press
 * starts a background drag; any pointer release ends it, even with another
 * button held. Release wins when down/up are coalesced. No event-order/UA claim. */
static inline void rr_showcase_input_step(RRShowcase *s,RRShowcaseInputFrame f)
{
    if(!f.active||!f.focused||!isfinite(f.x)||!isfinite(f.y)){rr_showcase_up(s);return;}
    if(f.pressed&&f.inside)rr_showcase_down(s,f.x,f.y,f.control);
    if(f.released||!f.held)rr_showcase_up(s);else rr_showcase_move(s,f.x,f.y);
    if(f.inside&&isfinite(f.wheel)&&f.wheel!=0)rr_showcase_wheel(s,f.wheel);
}
#endif

#ifndef RR_BUTTON_HOVER_H
#define RR_BUTTON_HOVER_H
#include "rr_announcement.h"
#include <stdint.h>
enum {RR_BUTTON_SLOTS=8};
typedef struct RRButtonHover {RRNoticeFade slots[RR_BUTTON_SLOTS];int scope;uint32_t used;double now,epoch;bool initialized;} RRButtonHover;
/* Reuse the qualified binary CSS transition kernel without changing notices.
 * Time rescaling maps its250ms to source button150ms. Per-owner epoch avoids
 * unnecessarily large scaled absolute timestamps. CSS display:none cancels. */
static inline void rr_button_hover_begin(RRButtonHover *h,int scope,double now)
{
    if(!h->initialized||h->scope!=scope){*h=(RRButtonHover){0};h->scope=scope;h->epoch=now;h->initialized=true;}
    h->used=0;h->now=(now-h->epoch)*(.25/.15);
}
static inline double rr_button_hover_sample(RRButtonHover *h,int slot,bool target)
{
    if(slot<0||slot>=RR_BUTTON_SLOTS)return target?1:0;
    h->used|=UINT32_C(1)<<slot;return rr_notice_observe(&h->slots[slot],target,true,h->now);
}
static inline void rr_button_hover_end(RRButtonHover *h)
{for(int i=0;i<RR_BUTTON_SLOTS;i++)if(!(h->used&(UINT32_C(1)<<i)))rr_notice_observe(&h->slots[i],false,false,h->now);}
#endif

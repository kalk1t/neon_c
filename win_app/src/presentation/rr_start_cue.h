#ifndef RR_START_CUE_H
#define RR_START_CUE_H
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
typedef struct RRStartCue {int count;double timer;bool shown;char text[8];} RRStartCue;
static inline void rr_start_cue_reset(RRStartCue *cue)
{memset(cue,0,sizeof(*cue));cue->count=4;}
/* Browser start resets lastCount while retaining the HUD message already on screen. */
static inline void rr_start_cue_restart(RRStartCue *cue){cue->count=4;}
static inline void rr_start_cue_observe(RRStartCue *cue,int count)
{
    if(count==cue->count)return;
    cue->count=count;cue->timer=.9;cue->shown=true;
    if(count>0)snprintf(cue->text,sizeof(cue->text),"%d",count);
    else snprintf(cue->text,sizeof(cue->text),"GO!");
}
/* Source HUD.update decrements before the frame is painted, including pause. */
static inline void rr_start_cue_tick(RRStartCue *cue,double dt)
{cue->timer-=dt;if(cue->timer<=0)cue->shown=false;}
#endif

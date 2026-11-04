#include "rr_results_ui.h"
bool rr_results_ui_frame(RRResultsTimer *timer,const RRRunCurrent *run,
 const RRRunEvents *tick,double seconds,bool hide,RRResultsDraw draw,
 void *context,RRResultsPump *events)
{
 if(!events)return false;
 *events=(RRResultsPump){0};
 if(!timer||!run||run->host.error||!run->current||run->current->error)return false;
 double now=seconds*1000;
 if(tick&&(tick->player&RR_RACE_FINISH)&&!rr_results_timer_schedule(timer,now))return false;
 /* Validate the clock before applying an explicit UI hide. No clock error may
  * hide an otherwise inspectable published card. pump validates again below. */
 if(!isfinite(now)||now<0||(timer->clock_ready&&now<timer->last_ms)){
  timer->error=true;return false;
 }
 if(timer->error)return false;
 if(hide)rr_results_timer_hide(timer);
 if(!rr_results_timer_pump(timer,run,now,events))return false;
 if(timer->visible){
  if(!timer->has_snapshot||!draw)return false;
  return draw(context,&timer->snapshot,timer->revision);
 }
 return true;
}

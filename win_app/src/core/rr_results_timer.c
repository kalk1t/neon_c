#include "rr_results_timer.h"
#include <stdint.h>
static bool clock_at(RRResultsTimer *p,double now){if(!p||p->error)return false;if(!isfinite(now)||now<0||(p->clock_ready&&now<p->last_ms)){p->error=true;return false;}p->clock_ready=true;p->last_ms=now;return true;}
void rr_results_timer_hide(RRResultsTimer *p){if(p)p->visible=false;}
bool rr_results_timer_schedule(RRResultsTimer *p,double now){
 if(!clock_at(p,now))return false;double due=now+2000;if(p->pending>=RR_RESULTS_TIMERS||!isfinite(due)||due<=now){p->error=true;return false;}p->due[p->pending++]=due;return true;
}
bool rr_results_timer_pump(RRResultsTimer *p,const RRRunCurrent *run,double now,RRResultsPump *events){
 if(!events)return false;*events=(RRResultsPump){0};if(!run||!clock_at(p,now))return false;
 while(p->pending&&p->due[0]<=now){
  for(size_t i=1;i<p->pending;i++)p->due[i-1]=p->due[i];p->pending--;events->callbacks++;
  if(run->host.phase!=RR_SESSION_FINISHED)continue;
  RRResultsCurrent next;if(p->revision==SIZE_MAX||!rr_run_results_build(run,&next)){p->error=true;return false;}
  p->snapshot=next;p->has_snapshot=p->visible=true;p->revision++;events->published++;
 }
 return true;
}
bool rr_run_results_after_tick(RRResultsTimer *p,const RRRunCurrent *run,const RRRunEvents *tick,double now,RRResultsPump *events){
 if(!events)return false;*events=(RRResultsPump){0};if(!p||!run||!tick||run->host.error||!run->current||run->current->error)return false;
 if((tick->player&RR_RACE_FINISH)&&!rr_results_timer_schedule(p,now))return false;
 return rr_results_timer_pump(p,run,now,events);
}

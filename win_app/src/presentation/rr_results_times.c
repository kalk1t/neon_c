#include "rr_results_times.h"
#include "rr_race_ui.h"
#include "rr_results_layout.h"
#include <float.h>
bool rr_results_times_build(const double *laps,size_t count,RRResultsTimes *out)
{
 if(!out||count>2||(count&&!laps))return false;
 RRResultsTimes value={0};value.count=count;value.best=SIZE_MAX;double total=0;
 for(size_t i=0;i<count;i++){
  if(!isfinite(laps[i])||laps[i]<0||laps[i]>1e12)return false;
  total+=laps[i];if(!isfinite(total)||total>1e12)return false;
  if(value.best==SIZE_MAX||laps[i]<laps[value.best])value.best=i;
  char clock[32];rr_ui_time(clock,laps[i]);
  snprintf(value.laps[i],sizeof(value.laps[i]),"LAP %u \xE2\x80\x94 %s",(unsigned)i+1,clock);
 }
 rr_ui_time(value.total,total);*out=value;return true;
}
bool rr_results_times_current(const RRResultsCurrent *snapshot,RRResultsTimes *out)
{return snapshot&&snapshot->lap_count<=2&&rr_results_times_build(snapshot->laps,snapshot->lap_count,out);}
bool rr_results_times_layout(const RRResultsTimes *text,float center,float y,const float widths[2],RRResultsTimesLayout *out)
{
 if(!text||!out||text->count>2||!isfinite(center)||!isfinite(y)||(text->count&&!widths))return false;
 double group=0;for(size_t i=0;i<text->count;i++){if(!isfinite(widths[i])||widths[i]<0)return false;group+=widths[i];}
 if(text->count==2)group+=28;
 double left=(double)center-group*.5,total=(double)y+rr_results_baseline(2,32,900),lap=(double)y+rr_results_baseline(3,16,700);
 if(fabs(left)>FLT_MAX||fabs(total)>FLT_MAX||fabs(lap)>FLT_MAX)return false;
 RRResultsTimesLayout value={0};value.total_baseline=(float)total;value.lap_baseline=(float)lap;
 for(size_t i=0;i<text->count;i++){if(fabs(left)>FLT_MAX)return false;value.left[i]=(float)left;left+=widths[i]+28;}
 *out=value;return true;
}

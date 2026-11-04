#include "rr_results_label.h"
#include "rr_race_ui.h"
#include <string.h>
bool rr_results_label_build(const RRResultsCurrent *s,RRResultsLabelKind kind,size_t row,const char *const *names,size_t count,size_t player,RRResultsLabel *out)
{
 if(!s||!out||!names||!count||count>RR_LOCAL_MAX_CARS||player>=count)return false;
 RRResultsLabel value={0};char clock[32];int racer;bool estimate=false,crown=false;
 if(kind==RR_LABEL_STANDINGS){if(s->time_count>RR_LOCAL_MAX_CARS||row>=s->time_count||!isfinite(s->times[row].seconds)||s->times[row].seconds<0||s->times[row].seconds>1e12)return false;racer=s->times[row].racer;estimate=s->times[row].estimated;rr_ui_time(clock,s->times[row].seconds);int n=snprintf(value.value,sizeof(value.value),"%s%s",estimate?"\xe2\x89\x88 ":"",clock);if(n<0||(size_t)n>=sizeof(value.value))return false;}
 else if(kind==RR_LABEL_COINS){if(s->coin_count>RR_LOCAL_MAX_CARS||row>=s->coin_count||s->coins[row].coins<0)return false;racer=s->coins[row].racer;crown=row==0;int n=snprintf(value.value,sizeof(value.value),"%d",s->coins[row].coins);if(n<0||(size_t)n>=sizeof(value.value))return false;}
 else return false;
 if(racer<0||(size_t)racer>=count||!names[racer])return false;
 size_t length=0;while(length<512&&names[racer][length])length++;if(length>507)return false;
 int n=snprintf(value.name,sizeof(value.name),"%s%s",crown?"\xe2\x99\x9b ":"",names[racer]);if(n<0||(size_t)n>=sizeof(value.name))return false;
 value.racer=(size_t)racer;value.player=(size_t)racer==player;*out=value;return true;
}

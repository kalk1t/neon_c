#include "rr_results_slots.h"
#include "rr_results_summary.h"
#include "rr_results_times.h"
#include <math.h>
static double normal(double size){return floor(size*1011/1000+.5)+floor(size*243/1000+.5);}
RRResultsNameStatus rr_results_empty_slots(const RRResultsCurrent *s,RRResultsCardContent *out)
{
 if(!s||!out)return RR_NAME_INVALID;
 RRResultsSummary summary;if(!rr_results_summary_build(s,&summary))return RR_NAME_INVALID;
 RRResultsTimes times;if(!rr_results_times_current(s,&times))return RR_NAME_INVALID;
 if(s->lap_count!=2||s->player_takedowns>9||s->laps[0]>3600||s->laps[1]>3600)return RR_NAME_NEEDS_FLOW;
 RRResultsCardContent value={{0},RR_CARD_REQUIRED};
 value.heights[RR_CARD_BRAND]=floor(34*.95*64)/64;
 value.heights[RR_CARD_PLACE]=s->coin_count>1?fmax(floor(24*1011.0/1000+.5),floor(24*2254.0/2048+.5))+fmax(floor(24*243.0/1000+.5),floor(24*634.0/2048+.5)):0;
 value.heights[RR_CARD_TOTAL]=normal(32);value.heights[RR_CARD_LAPS]=normal(16);
 value.heights[RR_CARD_AGAIN]=normal(20)+32+4;value.heights[RR_CARD_MENU]=value.heights[RR_CARD_AGAIN];
 *out=value;return RR_NAME_READY;
}

#include "rr_results_caption.h"
#include "rr_results_summary.h"
#include <math.h>
#include <float.h>
#include <stdio.h>
RRResultsNameStatus rr_results_caption_build(const RRResultsCurrent *s,double width,RRResultsCaption *out)
{
 if(!s||!out||!isfinite(width)||width<=0||width>FLT_MAX||!s->coin_count||s->coin_count>11||s->player_coin_rank<1||(size_t)s->player_coin_rank>s->coin_count)return RR_NAME_INVALID;
 if(width<258.25)return RR_NAME_NEEDS_FLOW;
 RRResultsCaption value={0};char ordinal[RR_RESULTS_ORDINAL_BYTES];if(!rr_results_ordinal(s->player_coin_rank,ordinal))return RR_NAME_INVALID;
 int bonus=s->coin_count>1?(s->player_coin_rank==1?15:s->player_coin_rank==2?8:s->player_coin_rank==3?4:0):0;
 int n=bonus?snprintf(value.text,sizeof(value.text),"COIN RACE %s \302\267 BONUS +%d",ordinal,bonus):snprintf(value.text,sizeof(value.text),"COIN RACE %s",ordinal);if(n<0||(size_t)n>=sizeof(value.text))return RR_NAME_INVALID;
 double ascent=floor(12.0*1011/1000+.5),descent=floor(12.0*243/1000+.5);
 if(bonus){ascent=fmax(ascent,floor(12.0*2254/2048+.5));descent=fmax(descent,floor(12.0*634/2048+.5));}
 value.height=ascent+descent;*out=value;return RR_NAME_READY;
}

#include "rr_results_summary.h"
#include <stdio.h>
#include <string.h>
bool rr_results_ordinal(int value,char out[RR_RESULTS_ORDINAL_BYTES])
{
 if(!out||value<1)return false;
 int last=value%10,teens=(value/10)%10;
 const char *suffix=teens==1?"th":last==1?"st":last==2?"nd":last==3?"rd":"th";
 char text[RR_RESULTS_ORDINAL_BYTES];int n=snprintf(text,sizeof(text),"%d%s",value,suffix);
 if(n<0||(size_t)n>=sizeof(text))return false;
 memcpy(out,text,(size_t)n+1);return true;
}
bool rr_results_summary_build(const RRResultsCurrent *s,RRResultsSummary *out)
{
 if(!s||!out||!s->coin_count||s->coin_count>RR_LOCAL_MAX_CARS||s->finish_position<1||(size_t)s->finish_position>s->coin_count||s->player_coin_rank<1||(size_t)s->player_coin_rank>s->coin_count||s->player_takedowns<0)return false;
 RRResultsSummary value={0};char ordinal[RR_RESULTS_ORDINAL_BYTES];
 if(!rr_results_ordinal(s->player_coin_rank,value.coin_ordinal))return false;
 if(s->coin_count>1){
  if(!rr_results_ordinal(s->finish_position,ordinal))return false;
  int n=snprintf(value.place,sizeof(value.place),"%s OF %u \302\267 %d TAKEDOWN%s",ordinal,(unsigned)s->coin_count,s->player_takedowns,s->player_takedowns==1?"":"S");
  if(n<0||(size_t)n>=sizeof(value.place))return false;
 }
 *out=value;return true;
}

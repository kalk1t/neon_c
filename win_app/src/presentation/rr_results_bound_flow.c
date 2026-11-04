#include "rr_results_bound_flow.h"
bool rr_results_flow_prepare(const RRResultsBoundView *v,RRResultsFlowBinding *binding)
{
 if(!v||!binding||!v->results||!v->revision||!v->count||v->count>RR_LOCAL_MAX_CARS||v->player>=v->count)return false;
 const RRResultsCurrent *s=v->results;if(s->coin_count!=v->count||s->time_count!=(v->count>1?v->count:0))return false;
 const size_t counts[2]={s->time_count,s->coin_count};RRResultsLabel check;
 for(int pass=0;pass<2;pass++)for(int k=0;k<2;k++){
  unsigned seen=0;
  for(size_t j=0;j<counts[k];j++){
   if(!rr_results_label_build(s,k?RR_LABEL_COINS:RR_LABEL_STANDINGS,j,v->names,v->count,v->player,&check))return false;
   unsigned bit=1u<<(unsigned)check.racer;if(seen&bit)return false;seen|=bit;
   if(pass){binding->labels[k][j]=check;binding->rows[k][j]=(RRResultsFlowText){binding->labels[k][j].name,binding->labels[k][j].value};binding->player[k][j]=check.player;}
  }
 }
 binding->counts[0]=counts[0];binding->counts[1]=counts[1];binding->revision=v->revision;return true;
}
RRResultsNameStatus rr_ui_results_bound_flow_draw(RRUI *ui,RRResultsArt *art,const RRResultsBoundView *view,const RRResultsBodyInput *profile,int scale,RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row)
{
 if(!profile||!rr_results_flow_prepare(view,binding))return RR_NAME_INVALID;
 RRResultsBodyInput input=*profile;input.standings=binding->rows[0];input.coins=binding->rows[1];input.standings_count=binding->counts[0];input.coin_count=binding->counts[1];
 return rr_ui_results_flow_draw(ui,art,&input,binding->player[0],binding->player[1],scale,work,row);
}

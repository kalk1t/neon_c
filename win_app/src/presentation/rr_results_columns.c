#include "rr_results_columns.h"
#include "rr_css_font_metrics.h"
#include <float.h>
static void no_paint(void *context,const RRResultsPaint *paint){(void)context;(void)paint;}
static bool rect_ready(RRUIRect r){return isfinite(r.x)&&isfinite(r.y)&&isfinite(r.w)&&isfinite(r.h)&&r.w>=26&&r.h>=0;}
RRResultsColumnsResult rr_results_columns_emit(Font font,const RRResultsBoundView *v,const RRResultsCardLayout *card,const double top[2],int scale,RRResultsRow *scratch,RRResultsPaintSink sink,void *ctx)
{
 RRResultsColumnsResult result={RR_RESULTS_COLUMNS_INVALID,RR_RESULTS_STANDINGS,0,0};
 if(!v||!v->results||!card||!top||!scratch||!sink||!v->revision||!v->count||v->count>RR_LOCAL_MAX_CARS||v->player>=v->count||scale<1||scale>3)return result;
 const RRResultsCurrent *s=v->results;
 if(s->coin_count!=v->count||s->time_count!=(v->count>1?v->count:0))return result;
 const size_t counts[2]={s->time_count,s->coin_count};const RRUIRect columns[2]={card->left,card->right};
 const double line=15*1.42;
 for(int k=0;k<2;k++){
  result.kind=(RRResultsRowKind)k;
  if(!rect_ready(columns[k])||!isfinite(top[k])||top[k]<columns[k].y)return result;
  double end=top[k]+line*(double)counts[k];
  if(!isfinite(end)||fabs(end)>FLT_MAX)return result;
  if(end>(double)columns[k].y+columns[k].h){result.status=RR_RESULTS_COLUMNS_NEEDS_HEIGHT;return result;}
 }
 /* Second pass rebuilds only after deterministic complete-board preflight.
  * No provider lookups, core ticks, publication or callback is done by pass0. */
 for(int pass=0;pass<2;pass++)for(int k=0;k<2;k++)for(size_t row=0;row<counts[k];row++){
  result.kind=(RRResultsRowKind)k;result.row=row;
  double left=(double)columns[k].x+26,right=(double)columns[k].x+columns[k].w;
  double baseline=rr_css_line_baseline(top[k]+line*(double)row,line,15,700);
  if(fabs(left)>FLT_MAX||fabs(right)>FLT_MAX||fabs(baseline)>FLT_MAX)return result;
  RRResultsRowStatus status=rr_results_row_build(font,s,(RRResultsRowKind)k,row,v->names,v->count,v->player,scale,(float)left,(float)right,(float)baseline,scratch);
  if(status!=RR_RESULTS_ROW_READY){result.status=status==RR_RESULTS_ROW_NEEDS_WRAP?RR_RESULTS_COLUMNS_NEEDS_WRAP:RR_RESULTS_COLUMNS_INVALID;return result;}
  /* Preflight emitter coordinate validation too, using no externally visible sink. */
  if(!rr_results_row_emit(scratch,pass?sink:no_paint,pass?ctx:NULL))return result;
  if(pass)result.rows++;
 }
 result.status=RR_RESULTS_COLUMNS_READY;result.kind=RR_RESULTS_STANDINGS;result.row=0;return result;
}

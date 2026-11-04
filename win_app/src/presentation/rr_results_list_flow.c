#include "rr_results_list_flow.h"
#include <math.h>
#include <float.h>
static const double line_height=21.296875;
RRResultsNameStatus rr_results_value_float(const char *value,double width,RRResultsNameTail tail,const RRResultsRightFloat *f,size_t n,RRResultsCharacters *scratch,RRResultsRightFloat *out)
{
 if(!out||!scratch||!isfinite(width)||width<=0||width>FLT_MAX||!isfinite(tail.y)||!isfinite(tail.advance)||tail.y<0||tail.advance<0||n>11||(n&&!f))return RR_NAME_INVALID;
 if(tail.y/line_height!=floor(tail.y/line_height)||tail.advance>width)return RR_NAME_NEEDS_FLOW;
 for(size_t i=0;i<n;i++){
  if(!isfinite(f[i].x)||!isfinite(f[i].y)||!isfinite(f[i].width)||!isfinite(f[i].height)||f[i].width<=0||f[i].height<=0||!isfinite(f[i].x+f[i].width)||!isfinite(f[i].y+f[i].height))return RR_NAME_INVALID;
  if(f[i].x<0||f[i].x+f[i].width>width||f[i].y>0||f[i].height!=line_height||f[i].y/line_height!=floor(f[i].y/line_height))return RR_NAME_NEEDS_FLOW;
 }
 RRResultsAdvanceStatus status=rr_results_characters(value,scratch);if(status==RR_ADVANCE_INVALID)return RR_NAME_INVALID;
 if(status!=RR_ADVANCE_READY||!scratch->count||scratch->range_width>width)return RR_NAME_NEEDS_FLOW;
 double y=tail.y,cursor=floor(tail.advance*64)/64;
 for(size_t attempt=0;attempt<13;attempt++){
  double right=width;for(size_t i=0;i<n;i++)if(f[i].y<y+line_height&&f[i].y+f[i].height>y)right=fmin(right,f[i].x);
  double used=y==tail.y?cursor:0;
  if(scratch->range_width+used<=right){*out=(RRResultsRightFloat){right-scratch->range_width,y,scratch->range_width,line_height};return RR_NAME_READY;}
  double next=y+line_height;if(!isfinite(next)||next<=y)return RR_NAME_NEEDS_FLOW;y=next;
 }
 return RR_NAME_NEEDS_FLOW;
}
static double pen(const RRResultsCharacters *s,size_t index){return index==s->count?s->advance:s->ranges[index].pen;}
static RRResultsNameStatus layout(RRResultsListKind kind,const RRResultsFlowText *rows,size_t count,double width,RRResultsListWork *w,RRResultsListSink sink,void *ctx,RRResultsListSummary *summary)
{
 double y=0,bottom=0;
 for(size_t row=0;row<count;row++){
  for(size_t i=0;i<row;i++){w->relative[i]=w->absolute[i];w->relative[i].y-=y;}
  RRResultsNameStatus status=rr_results_name_flow(rows[row].name,width,w->relative,row,&w->name_characters,&w->name);if(status!=RR_NAME_READY)return status;
  const RRResultsNameLine *last=w->name.lines+w->name.count-1;
  RRResultsNameTail tail={last->y-1,pen(&w->name_characters,last->first+last->count)-pen(&w->name_characters,last->first)};
  RRResultsRightFloat placed;status=rr_results_value_float(rows[row].value,width,tail,w->relative,row,&w->value_characters,&placed);if(status!=RR_NAME_READY)return status;
  placed.y+=y;w->absolute[row]=placed;bottom=fmax(bottom,placed.y+placed.height);
  if(sink){RRResultsListRowView view={row,y,w->name.height,&w->name,&w->name_characters,&w->value_characters,placed};sink(ctx,&view);}
  y+=w->name.height;
 }
 *summary=(RRResultsListSummary){count,y,bottom,kind==RR_FLOW_STANDINGS?fmax(y,bottom):y};return RR_NAME_READY;
}
RRResultsNameStatus rr_results_list_flow(RRResultsListKind kind,const RRResultsFlowText *rows,size_t count,double width,RRResultsListWork *work,RRResultsListSink sink,void *ctx,RRResultsListSummary *out)
{
 if(!work||!out||(unsigned)kind>RR_FLOW_COINS||count>11||(count&&!rows)||!isfinite(width)||width<=0||width>FLT_MAX)return RR_NAME_INVALID;
 RRResultsListSummary summary;RRResultsNameStatus status=layout(kind,rows,count,width,work,NULL,NULL,&summary);if(status!=RR_NAME_READY)return status;
 if(sink){status=layout(kind,rows,count,width,work,sink,ctx,&summary);if(status!=RR_NAME_READY)return status;}
 *out=summary;return RR_NAME_READY;
}

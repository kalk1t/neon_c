#include "rr_results_name_flow.h"
#include <math.h>
#include <float.h>
#include <string.h>
static const double line_height=21.296875;
static bool latin(uint32_t c){return (c>=65&&c<=90)||(c>=97&&c<=122);}
static double pen(const RRResultsCharacters *s,size_t i){return i==s->count?s->advance:s->ranges[i].pen;}
static RRResultsNameStatus pass(const RRResultsCharacters *s,double width,const RRResultsRightFloat *f,size_t nf,RRResultsNameFlow *out)
{
 size_t start=0,count=0,steps=0;double y=0;
 while(start<s->count){
  if(steps++>=RR_RESULTS_NAME_LINES+RR_RESULTS_NAME_FLOATS+1)return RR_NAME_NEEDS_FLOW;
  double right=width,next_y=DBL_MAX;
  for(size_t i=0;i<nf;i++)if(f[i].y<y+line_height&&f[i].y+f[i].height>y){right=fmin(right,f[i].x);next_y=fmin(next_y,f[i].y+f[i].height);}
  size_t end=s->count,next=end;double origin=pen(s,start);
  if(pen(s,end)-origin>right){
   end=start;next=start;
   for(size_t i=start;i<s->count;i++){
    uint32_t cp=s->ranges[i].codepoint;size_t candidate,following;
    if(cp==32){candidate=i;following=i+1;}
    else if(cp==45){
     if(!i||i+1==s->count||!latin(s->ranges[i-1].codepoint)||!latin(s->ranges[i+1].codepoint))return RR_NAME_NEEDS_FLOW;
     candidate=i+1;following=i+1;
    }else continue;
    if(candidate>start&&pen(s,candidate)-origin<=right){end=candidate;next=following;}
   }
   if(end==start){if(next_y==DBL_MAX||next_y<=y)return RR_NAME_NEEDS_FLOW;y=next_y;continue;}
  }
  if(count==RR_RESULTS_NAME_LINES)return RR_NAME_NEEDS_FLOW;
  if(out)out->lines[count]=(RRResultsNameLine){start,end-start,0,y+1,ceil((pen(s,end)-origin)*64)/64,19};
  count++;start=next;y+=line_height;
 }
 if(out){out->count=count;out->height=y;}return RR_NAME_READY;
}
RRResultsNameStatus rr_results_name_flow(const char *text,double width,const RRResultsRightFloat *f,size_t nf,RRResultsCharacters *scratch,RRResultsNameFlow *out)
{
 if(!scratch||!out||!isfinite(width)||width<=0||width>FLT_MAX||nf>RR_RESULTS_NAME_FLOATS||(nf&&!f))return RR_NAME_INVALID;
 for(size_t i=0;i<nf;i++){
  if(!isfinite(f[i].x)||!isfinite(f[i].y)||!isfinite(f[i].width)||!isfinite(f[i].height)||f[i].width<=0||f[i].height<=0||!isfinite(f[i].x+f[i].width)||!isfinite(f[i].y+f[i].height))return RR_NAME_INVALID;
  if(f[i].x<0||f[i].x+f[i].width>width||f[i].height!=line_height||f[i].y/line_height!=floor(f[i].y/line_height))return RR_NAME_NEEDS_FLOW;
 }
 RRResultsAdvanceStatus status=rr_results_characters(text,scratch);
 if(status==RR_ADVANCE_INVALID)return RR_NAME_INVALID;if(status!=RR_ADVANCE_READY||!scratch->count)return RR_NAME_NEEDS_FLOW;
 RRResultsNameStatus ready=pass(scratch,width,f,nf,NULL);if(ready!=RR_NAME_READY)return ready;
 memset(out,0,sizeof(*out));return pass(scratch,width,f,nf,out);
}

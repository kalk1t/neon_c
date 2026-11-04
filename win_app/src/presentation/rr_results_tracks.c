#include "rr_results_tracks.h"
#include <math.h>
static bool latin(uint32_t c){return(c>=65&&c<=90)||(c>=97&&c<=122);}
static double pen(const RRResultsCharacters *s,size_t n){return n==s->count?s->advance:s->ranges[n].pen;}
static RRResultsNameStatus minimum(const char *text,bool value,RRResultsCharacters *s,double *out)
{
 RRResultsAdvanceStatus status=rr_results_characters(text,s);if(status==RR_ADVANCE_INVALID)return RR_NAME_INVALID;if(status!=RR_ADVANCE_READY||!s->count)return RR_NAME_NEEDS_FLOW;
 size_t start=0;double widest=0;
 for(size_t j=0;j<s->count;j++){
  uint32_t cp=s->ranges[j].codepoint;bool digit=cp>=48&&cp<=57;
  if(value){if(!(digit||cp==32||cp==58||cp==46||cp==8776))return RR_NAME_NEEDS_FLOW;if(cp==8776&&(j||s->count<3||s->ranges[1].codepoint!=32))return RR_NAME_NEEDS_FLOW;}
  else{if(!(digit||latin(cp)||cp==32||cp==45||cp==9819))return RR_NAME_NEEDS_FLOW;if(cp==9819&&(j||s->count<3||s->ranges[1].codepoint!=32))return RR_NAME_NEEDS_FLOW;}
  if(cp==45&&(!j||j+1==s->count||!latin(s->ranges[j-1].codepoint)||!latin(s->ranges[j+1].codepoint)))return RR_NAME_NEEDS_FLOW;
  if(cp==32||cp==45){size_t end=cp==45?j+1:j;widest=fmax(widest,pen(s,end)-pen(s,start));start=j+1;}
 }
 widest=fmax(widest,s->advance-pen(s,start));*out=ceil(widest*64)/64;return RR_NAME_READY;
}
RRResultsNameStatus rr_results_tracks(const RRResultsFlowText *standings,size_t ns,const RRResultsFlowText *coins,size_t nc,const double other[2],RRResultsCharacters *scratch,RRResultsTracks *out)
{
 if(!scratch||!out||!other||ns>11||nc>11||(ns&&!standings)||(nc&&!coins)||!isfinite(other[0])||!isfinite(other[1])||other[0]<0||other[1]<0)return RR_NAME_INVALID;
 if(!nc)return RR_NAME_NEEDS_FLOW;
 RRResultsTracks result={0};const RRResultsFlowText *lists[2]={standings,coins};const size_t counts[2]={ns,nc};
 for(int k=0;k<2;k++)for(size_t j=0;j<counts[k];j++){
  double name,value;RRResultsNameStatus status=minimum(lists[k][j].name,false,scratch,&name);if(status!=RR_NAME_READY)return status;
  status=minimum(lists[k][j].value,true,scratch,&value);if(status!=RR_NAME_READY)return status;
  result.minimum[k]=fmax(result.minimum[k],26+fmax(name,value));
 }
 double left=594*1.3/2.3,right=594/2.3;
 if(result.minimum[0]+result.minimum[1]>594){left=result.minimum[0];right=result.minimum[1];}
 else if(result.minimum[0]>left){left=result.minimum[0];right=594-left;}
 else if(result.minimum[1]>right){right=result.minimum[1];left=594-right;}
 result.width[0]=floor(left*64)/64;result.width[1]=floor(right*64)/64;
 if(other[0]>result.width[0]||other[1]>result.width[1])return RR_NAME_NEEDS_FLOW;
 result.overflow=fmax(0,result.width[0]+22+result.width[1]-616);*out=result;return RR_NAME_READY;
}

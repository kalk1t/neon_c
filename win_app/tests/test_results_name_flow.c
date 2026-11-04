#include "rr_results_name_flow.h"
#include "results_name_flow_reference.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,lines,leading,multiline;static RRResultsCharacters scratch;static RRResultsNameFlow actual,guard,saved;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"name flow line %d: %s\n",__LINE__,#x);failures++;}}while(0)
int main(void){
 for(size_t i=0;i<COUNT(name_flow_cases);i++){
  const NameFlowCase *c=name_flow_cases+i;CHECK(rr_results_name_flow(c->text,c->width,c->floats,c->float_count,&scratch,&actual)==RR_NAME_READY);CHECK(actual.count==c->count&&actual.height==c->height);
  for(size_t j=0;j<actual.count&&j<c->count;j++){const RRResultsNameLine *a=actual.lines+j,*b=c->lines+j;CHECK(a->first==b->first&&a->count==b->count&&a->x==b->x&&a->y==b->y&&a->width==b->width&&a->height==b->height);lines++;}
  leading+=actual.lines[0].y>1;multiline+=actual.count>1;
 }
 memset(&guard,0x5a,sizeof(guard));saved=guard;
 CHECK(rr_results_name_flow("AWX",300,NULL,0,&scratch,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("WWWW",1,NULL,0,&scratch,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("W-0",1,NULL,0,&scratch,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("",300,NULL,0,&scratch,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("^",300,NULL,0,&scratch,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("ACE",NAN,NULL,0,&scratch,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("ACE",300,NULL,1,&scratch,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 RRResultsRightFloat f={100,0,200,20};CHECK(rr_results_name_flow("ACE",300,&f,1,&scratch,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 f.height=21.296875;f.y=.5;CHECK(rr_results_name_flow("ACE",300,&f,1,&scratch,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 f.y=NAN;CHECK(rr_results_name_flow("ACE",300,&f,1,&scratch,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow(NULL,300,NULL,0,&scratch,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("ACE",300,NULL,0,NULL,&guard)==RR_NAME_INVALID&&rr_results_name_flow("ACE",300,NULL,0,&scratch,NULL)==RR_NAME_INVALID);
 printf("RESULTS_NAME_FLOW_SUMMARY observed_rows=%zu lines=%u leading_clearance=%u multiline=%u output_bytes=%zu scratch_bytes=%zu failures=%u prior_float_states_are_inputs=true current_value_placement_not_implemented=true\n",COUNT(name_flow_cases),lines,leading,multiline,sizeof(actual),sizeof(scratch),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

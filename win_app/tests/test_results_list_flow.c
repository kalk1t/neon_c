#include "rr_results_list_flow.h"
#include "results_list_flow_reference.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,callbacks,rows_seen,lines_seen,relocated;static const FlowExpectedList *expected;
static RRResultsListWork work;static RRResultsListSummary summary,guard,saved;static RRResultsFlowText rows[11];
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"list flow line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void observe(void *ctx,const RRResultsListRowView *v){
 CHECK(ctx==&work&&v->index==callbacks&&v->index<expected->count);callbacks++;if(v->index>=expected->count)return;const FlowExpectedRow *e=expected->rows+v->index;
 CHECK(v->y==e->y&&v->height==e->height&&v->name->count==e->count&&v->name_characters->count>0&&v->value_characters->count>0);
 CHECK(v->value.x==e->value_box.x&&v->value.y==e->value_box.y&&v->value.width==e->value_box.width&&v->value.height==e->value_box.height);
 for(size_t i=0;i<v->name->count&&i<e->count;i++){const RRResultsNameLine *a=v->name->lines+i,*b=e->lines+i;CHECK(a->first==b->first&&a->count==b->count&&a->x==b->x&&a->y==b->y&&a->width==b->width&&a->height==b->height);lines_seen++;}
 rows_seen++;relocated+=v->value.y>v->y;
}
int main(void){
 for(size_t i=0;i<COUNT(flow_lists);i++){
  expected=flow_lists+i;for(size_t j=0;j<expected->count;j++)rows[j]=(RRResultsFlowText){expected->rows[j].name,expected->rows[j].value};callbacks=0;
  CHECK(rr_results_list_flow(expected->kind,rows,expected->count,expected->width,&work,observe,&work,&summary)==RR_NAME_READY);
  CHECK(callbacks==expected->count&&summary.rows==expected->count&&summary.normal_height==expected->normal_height&&summary.float_bottom==expected->float_bottom&&summary.height==expected->height);
 }
 /* Last-row failure must not expose even the first row's geometry. */
 rows[0]=(RRResultsFlowText){"ACE","0"};rows[1]=(RRResultsFlowText){"YOU","^"};callbacks=0;memset(&guard,0x5a,sizeof(guard));saved=guard;
 CHECK(rr_results_list_flow(RR_FLOW_COINS,rows,2,232.25,&work,observe,&work,&guard)==RR_NAME_INVALID&&!callbacks&&!memcmp(&guard,&saved,sizeof(guard)));
 rows[1].value="00:00.00";rows[1].name="AWX";CHECK(rr_results_list_flow(RR_FLOW_COINS,rows,2,232.25,&work,observe,&work,&guard)==RR_NAME_NEEDS_FLOW&&!callbacks&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_list_flow(RR_FLOW_COINS,NULL,0,232.25,&work,NULL,NULL,&summary)==RR_NAME_READY&&!summary.rows&&!summary.height&&!summary.float_bottom);
 CHECK(rr_results_list_flow((RRResultsListKind)2,rows,1,232.25,&work,NULL,NULL,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_list_flow(RR_FLOW_COINS,rows,12,232.25,&work,NULL,NULL,&guard)==RR_NAME_INVALID);
 CHECK(rr_results_list_flow(RR_FLOW_COINS,rows,1,NAN,&work,NULL,NULL,&guard)==RR_NAME_INVALID);
 RRResultsRightFloat placed,before;memset(&placed,0x5a,sizeof(placed));before=placed;RRResultsRightFloat prior={100,21.296875,100,21.296875};
 CHECK(rr_results_value_float("0",232.25,(RRResultsNameTail){0,35},&prior,1,&work.value_characters,&placed)==RR_NAME_NEEDS_FLOW&&!memcmp(&placed,&before,sizeof(placed)));
 CHECK(rr_results_value_float("0",232.25,(RRResultsNameTail){0,NAN},NULL,0,&work.value_characters,&placed)==RR_NAME_INVALID&&!memcmp(&placed,&before,sizeof(placed)));
 CHECK(rr_results_value_float("0",1,(RRResultsNameTail){0,0},NULL,0,&work.value_characters,&placed)==RR_NAME_NEEDS_FLOW&&!memcmp(&placed,&before,sizeof(placed)));
 CHECK(rr_results_value_float(NULL,232.25,(RRResultsNameTail){0,0},NULL,0,&work.value_characters,&placed)==RR_NAME_INVALID);
 printf("RESULTS_LIST_FLOW_SUMMARY observed_lists=%zu rows=%u name_fragments=%u relocated_values=%u work_bytes=%zu summary_bytes=%zu failures=%u prior_floats_generated=true no_painter_or_main_activation=true\n",COUNT(flow_lists),rows_seen,lines_seen,relocated,sizeof(work),sizeof(summary),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

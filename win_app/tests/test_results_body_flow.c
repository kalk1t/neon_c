#include "rr_results_body_flow.h"
#include "results_body_flow_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,rows,scrolled,overflow;static RRResultsListWork work;static RRResultsBodyLayout out,guard,saved;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"body flow line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void same(RRUIRect a,RRUIRect b){CHECK(a.x==b.x&&a.y==b.y&&a.w==b.w&&a.h==b.h);}
static RRResultsBodyInput input(const BodyCase *c){RRResultsBodyInput i={c->width,c->height,c->scroll,c->left,c->right,c->caption_height,c->rows[0],c->rows[1],c->counts[0],c->counts[1],c->other};return i;}
int main(void){
 for(size_t n=0;n<COUNT(body_cases);n++){
  const BodyCase *c=body_cases+n;RRResultsBodyInput i=input(c);i.other.heights[RR_CARD_COLUMNS]=NAN;
  CHECK(rr_results_body_flow(&i,&work,&out)==RR_NAME_READY);same(out.card.panel,c->panel);same(out.card.boxes[RR_CARD_COLUMNS],c->grid);same(out.card.left,c->columns[0]);same(out.card.right,c->columns[1]);same(out.caption,c->caption);
  for(size_t j=0;j<2;j++){same(out.labels[j],c->labels[j]);same(out.list_boxes[j],c->lists[j]);rows+=(unsigned)c->counts[j];}
  CHECK(out.metrics.left_natural==c->natural[0]&&out.metrics.right_natural==c->natural[1]&&out.card.max_scroll==c->max_scroll&&out.card.scroll==c->scroll&&out.card.content_height==c->panel.h);scrolled+=c->scroll>0;overflow+=c->left+22+c->right>616;
  /* Same frozen content: resize clamps previous scroll, negative request clamps
   * to zero, oversized request reaches integer source max without shrinking. */
  i.scroll=-100;CHECK(rr_results_body_flow(&i,&work,&out)==RR_NAME_READY&&out.card.scroll==0);
  i.scroll=1e9;CHECK(rr_results_body_flow(&i,&work,&out)==RR_NAME_READY&&out.card.scroll==out.card.max_scroll);
  i.height=2000;i.scroll=c->max_scroll;CHECK(rr_results_body_flow(&i,&work,&out)==RR_NAME_READY&&out.card.scroll==0);
 }
 RRResultsBodyInput i=input(body_cases);memset(&guard,0x5a,sizeof(guard));saved=guard;
 const double invalid[]={0,-1,NAN,INFINITY};for(size_t n=0;n<COUNT(invalid);n++){i.caption_height=invalid[n];CHECK(rr_results_body_flow(&i,&work,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));}i=input(body_cases);
 i.width=.5;CHECK(rr_results_body_flow(&i,&work,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));i=input(body_cases);i.coin_count=0;CHECK(rr_results_body_flow(&i,&work,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 i=input(body_cases);RRResultsFlowText bad[]={ {"YOU","^"} };i.coins=bad;i.coin_count=1;CHECK(rr_results_body_flow(&i,&work,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 i=input(body_cases);i.other.visible=0;CHECK(rr_results_body_flow(&i,&work,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_body_flow(NULL,&work,&guard)==RR_NAME_INVALID&&rr_results_body_flow(&i,NULL,&guard)==RR_NAME_INVALID&&rr_results_body_flow(&i,&work,NULL)==RR_NAME_INVALID);
 printf("RESULTS_BODY_FLOW_SUMMARY observed_cards=%zu rows=%u bottom_scroll_observations=%u overflow_tracks=%u work_bytes=%zu output_bytes=%zu failures=%u caption_and_tracks_qualified_inputs=true native_main_unchanged=true\n",COUNT(body_cases),rows,scrolled,overflow,sizeof(work),sizeof(out),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

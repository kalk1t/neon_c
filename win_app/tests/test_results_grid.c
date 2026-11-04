#include "rr_results_card.h"
#include "results_grid_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,expanded,integration;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"grid line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static bool near(double a,double b){return fabs(a-b)<.002;}
static void same(RRUIRect a,RRUIRect b){CHECK(near(a.x,b.x)&&near(a.y,b.y)&&near(a.w,b.w)&&near(a.h,b.h));}
int main(void){
 for(size_t i=0;i<COUNT(grid_cases);i++){
  const GridCase *c=grid_cases+i;RRResultsGridLayout g;CHECK(rr_results_grid_layout(c->anchor,&c->metrics,&g));same(g.box,c->grid);same(g.left,c->left);same(g.right,c->right);
  CHECK(near(g.box.h,fmax(c->metrics.left_natural,c->metrics.right_natural))&&near(g.overflow_x,fmax(0,c->metrics.left_width+22+c->metrics.right_width-c->anchor.w)));expanded+=g.overflow_x>0;
  /* Derived integration uses the existing native content profile with measured
   * grid heights/tracks substituted, not the package's different frozen header. */
  RRResultsCardContent content=rr_results_card_legacy();content.heights[RR_CARD_COLUMNS]=fmax(c->metrics.left_natural,c->metrics.right_natural);
  RRResultsCardLayout card;CHECK(rr_results_card_flow_intrinsic(960,480,&content,&c->metrics,100,&card));
  CHECK(near(card.left.w,c->metrics.left_width)&&near(card.right.w,c->metrics.right_width)&&near(card.left.h,g.box.h)&&near(card.right.h,g.box.h)&&near(card.right.x-card.left.x,c->metrics.left_width+22));integration++;
 }
 RRResultsCardContent content=rr_results_card_legacy();RRResultsCardLayout old,now;RRResultsGridMetrics metrics={335.734375,258.25,content.heights[RR_CARD_COLUMNS],content.heights[RR_CARD_COLUMNS]};
 CHECK(rr_results_card_flow(1600,1000,&content,0,&old)&&rr_results_card_flow_intrinsic(1600,1000,&content,&metrics,0,&now)&&!memcmp(&old,&now,sizeof(old)));
 RRResultsGridLayout guard,saved;memset(&guard,0x5a,sizeof(guard));saved=guard;RRUIRect anchor={32,100,616,0};RRResultsGridMetrics m={300,300,20,40};
 double bad[]={NAN,INFINITY,-1};for(size_t i=0;i<COUNT(bad);i++){m.left_width=bad[i];CHECK(!rr_results_grid_layout(anchor,&m,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));m.left_width=300;m.right_natural=bad[i];CHECK(!rr_results_grid_layout(anchor,&m,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));m.right_natural=40;}
 m.left_width=0;CHECK(!rr_results_grid_layout(anchor,&m,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));m.left_width=300;
 volatile float huge=FLT_MAX;anchor.x=huge;m.left_width=huge;CHECK(!rr_results_grid_layout(anchor,&m,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(!rr_results_grid_layout((RRUIRect){0,0,616,0},NULL,&guard)&&!rr_results_grid_layout((RRUIRect){0,0,616,0},&m,NULL));
 memset(&now,0x5a,sizeof(now));RRResultsCardLayout saved_card=now;metrics.left_natural=999;CHECK(!rr_results_card_flow_intrinsic(1600,1000,&content,&metrics,0,&now)&&!memcmp(&now,&saved_card,sizeof(now)));
 printf("RESULTS_GRID_SUMMARY observed=%zu expanded_observations=%u derived_integrations=%u metrics_bytes=%zu grid_bytes=%zu failures=%u measured_tracks_not_auto_min_solver=true no_float_or_native_pixel_claim=true\n",COUNT(grid_cases),expanded,integration,sizeof(metrics),sizeof(guard),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

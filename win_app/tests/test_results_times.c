#include "rr_results_times.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "results_times_reference.h"
static unsigned failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static bool near(double a,double b){return fabs(a-b)<.001;}
int main(void){
 for(size_t i=0;i<sizeof(times_cases)/sizeof(times_cases[0]);i++){
  const struct TimesCase *c=times_cases+i;RRResultsTimes text;
  CHECK(rr_results_times_build(c->count?c->laps:NULL,c->count,&text));CHECK(text.count==c->count&&!strcmp(text.total,c->total));
  CHECK(text.best==(c->best<0?SIZE_MAX:(size_t)c->best));
  for(size_t j=0;j<c->count;j++)CHECK(!strcmp(text.laps[j],c->labels[j]));
  float widths[2]={(float)(50+i%113),(float)(60+i%137)};RRResultsTimesLayout layout;
  CHECK(rr_results_times_layout(&text,800,100,widths,&layout));
  CHECK(near(layout.total_baseline,100+107.296875+(40-32*1.254)/2+32*1.011));
  CHECK(near(layout.lap_baseline,100+156.296875+(20-16*1.254)/2+16*1.011));
  if(c->count==1)CHECK(near(layout.left[0]+widths[0]/2,800));
  if(c->count==2){CHECK(near(layout.left[1]-layout.left[0]-widths[0],28));CHECK(near((layout.left[0]+layout.left[1]+widths[1])/2,800));}
 }
 RRResultsTimes guard;memset(&guard,0x5a,sizeof(guard));RRResultsTimes before=guard;
 double bads[]={-1,NAN,INFINITY,1e12+1};for(size_t i=0;i<4;i++){CHECK(!rr_results_times_build(bads+i,1,&guard));CHECK(!memcmp(&guard,&before,sizeof(guard)));}
 double over[]={6e11,6e11};CHECK(!rr_results_times_build(over,2,&guard));CHECK(!rr_results_times_build(NULL,1,&guard));CHECK(!rr_results_times_build(over,3,&guard));CHECK(!rr_results_times_build(over,1,NULL));
 CHECK(!memcmp(&guard,&before,sizeof(guard)));
 RRResultsCurrent snapshot={0};snapshot.lap_count=2;snapshot.laps[0]=30;snapshot.laps[1]=40;
 CHECK(rr_results_times_current(&snapshot,&guard));RRResultsTimes frozen=guard;snapshot.laps[0]=999;CHECK(!memcmp(&guard,&frozen,sizeof(guard)));
 CHECK(!rr_results_times_current(NULL,&guard));snapshot.lap_count=3;CHECK(!rr_results_times_current(&snapshot,&guard));CHECK(!memcmp(&guard,&frozen,sizeof(guard)));
 RRResultsTimesLayout layout;memset(&layout,0x5a,sizeof(layout));RRResultsTimesLayout saved=layout;
 float widths[2]={-1,20};CHECK(!rr_results_times_layout(&guard,0,0,widths,&layout));CHECK(!memcmp(&layout,&saved,sizeof(layout)));
 widths[0]=NAN;CHECK(!rr_results_times_layout(&guard,0,0,widths,&layout));widths[0]=20;
 CHECK(!rr_results_times_layout(&guard,NAN,0,widths,&layout));CHECK(!rr_results_times_layout(&guard,0,INFINITY,widths,&layout));CHECK(!rr_results_times_layout(&guard,0,0,NULL,&layout));CHECK(!memcmp(&layout,&saved,sizeof(layout)));
 printf("RESULTS_TIMES_SUMMARY source_cases=%zu failures=%u no_graphics=true\n",sizeof(times_cases)/sizeof(times_cases[0]),failures);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

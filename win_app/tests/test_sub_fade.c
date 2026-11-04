#include "rr_announcement.h"
#include "rr_sub_notice.h"
#include "rr_subtext_alpha.h"
#include <stdio.h>
#include <string.h>
typedef struct SubFadeCase {int sequence,op;double now,dt;const char *text;double duration;int visible,shown,rendered,paint;double opacity;const char *expected_text;double timer;int core,shadow;} SubFadeCase;
#include "sub_fade_fixture.h"
static int failures;static double worst;static size_t paints;
#define CHECK(v) do{if(!(v)){if(failures<5)fprintf(stderr,"line%d: %s\n",__LINE__,#v);failures++;}}while(0)
int main(void){RRNoticeFade fade={0};RRSubNotice notice={0};int sequence=-1;bool visible=false;
for(size_t i=0;i<sizeof(sub_cases)/sizeof(sub_cases[0]);i++){
 const SubFadeCase *c=&sub_cases[i];if(c->sequence!=sequence){fade=(RRNoticeFade){0};notice=(RRSubNotice){0};visible=false;sequence=c->sequence;}
 if(c->op==0)rr_sub_notice_set(&notice,c->text,c->duration);else if(c->op==1)rr_sub_notice_tick(&notice,c->dt);else if(c->op==2)visible=c->visible!=0;
 CHECK(notice.shown==(c->shown!=0));CHECK(visible==(c->rendered!=0));CHECK(strcmp(notice.text,c->expected_text)==0);CHECK(fabs(notice.timer-c->timer)<1e-10);
 if(c->paint){
  paints++;double opacity=rr_notice_observe(&fade,notice.shown,visible,c->now),error=fabs(opacity-c->opacity);if(error>worst)worst=error;
  if(error>2e-9){if(failures<5)fprintf(stderr,"paint%zu opacity%.17g expected%.17g\n",i,opacity,c->opacity);failures++;}
  CHECK(rr_subtext_alpha(opacity,0)==c->core);CHECK(rr_subtext_alpha(opacity,1)==c->shadow);
  /* Ideal tint-input quantization bound, not a GPU framebuffer/raster test. */
  for(int pixel=0;pixel<=255;pixel+=51){double tinted=pixel*(double)rr_subtext_alpha(opacity,0)/255;CHECK(fabs(tinted-pixel*opacity)<=.500000001);}
 }
}
CHECK(rr_subtext_alpha(0,0)==0&&rr_subtext_alpha(0,1)==0);CHECK(rr_subtext_alpha(1,0)==255&&rr_subtext_alpha(1,1)==31);CHECK(rr_subtext_alpha(.5,0)==128&&rr_subtext_alpha(.5,1)==16);CHECK(rr_subtext_alpha(-1,0)==0&&rr_subtext_alpha(2,0)==255&&rr_subtext_alpha(NAN,0)==0);
char long_text[256];memset(long_text,'Q',255);long_text[255]=0;rr_sub_notice_set(&notice,long_text,1.6);CHECK(strlen(notice.text)==127&&notice.text[127]==0);rr_sub_notice_set(&notice,notice.text,1);CHECK(strlen(notice.text)==127);rr_sub_notice_set(&notice,notice.text+1,1);CHECK(strlen(notice.text)==126);
/* New submessage fade state cannot alter the existing main-message channel. */
RRNoticeFade main={0};rr_notice_observe(&main,false,true,0);rr_notice_observe(&main,true,true,.01);double before=rr_notice_value(&main,.1);rr_notice_observe(&fade,false,true,.1);CHECK(rr_notice_observe(&main,true,true,.1)==before);
printf("Submessage fade: %zu actual-source states, %zu coalesced paint/model samples across16 sequences; UTF8/plain text and shared tint bounds; worst opacity error%.12g, %d failures; no graphics\n",sizeof(sub_cases)/sizeof(sub_cases[0]),paints,worst,failures);return failures?1:0;}

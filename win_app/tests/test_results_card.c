#include "rr_results_card.h"
#include "rr_race_ui.h"
#include "results_card_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,observed,extensions,generated;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"card line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static bool near(double a,double b){return fabs(a-b)<.002;}
static bool same(RRUIRect a,RRUIRect b){return near(a.x,b.x)&&near(a.y,b.y)&&near(a.w,b.w)&&near(a.h,b.h);}
static void invariants(const RRResultsCardContent *c,const RRResultsCardLayout *l,double width,double height){
 CHECK(l->visible==c->visible&&near(l->panel.w,680)&&near(l->panel.x+l->panel.w/2,width/2));
 CHECK(near(l->panel.h,l->content_height)&&near(l->max_scroll,fmax(0,l->content_height+32-height))&&l->scroll>=0&&l->scroll<=l->max_scroll);
 CHECK(near(l->panel.y,fmax(16,(height-l->content_height)/2)-l->scroll));
 double y=l->panel.y+24;unsigned count=0;double heights=0;
 for(int i=0;i<RR_CARD_SLOTS;i++){
  if(!(c->visible&(1u<<i))){CHECK(same(l->boxes[i],(RRUIRect){0}));continue;}
  CHECK(near(l->boxes[i].x,l->panel.x+32)&&near(l->boxes[i].w,616)&&near(l->boxes[i].y,y)&&near(l->boxes[i].h,c->heights[i]));
  y+=c->heights[i]+9;heights+=c->heights[i];count++;
 }
 CHECK(near(l->panel.h,48+heights+9*(count-1))&&near(y-9+24,l->panel.y+l->panel.h));
 CHECK(near(l->left.x,l->boxes[RR_CARD_COLUMNS].x)&&near(l->right.x-l->left.x-l->left.w,22)&&near(l->left.w,335.734375)&&near(l->right.w,258.25));
 CHECK(near(l->left.y,l->boxes[RR_CARD_COLUMNS].y)&&near(l->right.y,l->left.y)&&near(l->left.h,c->heights[RR_CARD_COLUMNS])&&near(l->right.h,l->left.h));
}
int main(void){
 for(size_t i=0;i<sizeof(card_fixtures)/sizeof(card_fixtures[0]);i++){
  const struct CardFixture *f=card_fixtures+i;RRResultsCardLayout l;
  CHECK(rr_results_card_flow(f->width,f->height,&f->content,0,&l));CHECK(same(l.panel,f->panel));invariants(&f->content,&l,f->width,f->height);
  for(int slot=0;slot<RR_CARD_SLOTS;slot++){
   /* Package coordinates subtract the parent even for display:none nodes;
    * those negative relative offsets restore the observed absolute zero box. */
   RRUIRect expected=f->boxes[slot];expected.x+=f->panel.x;expected.y+=f->panel.y;
   CHECK(same(l.boxes[slot],expected));
  }
  observed++;
  const double views[][2]={{1280,720},{1920,1080},{800,600},{680,500},{1600,400},{360,640},{700,320},{1280,523.1875}};
  for(size_t j=0;j<sizeof(views)/sizeof(views[0]);j++){
   RRResultsCardLayout top,middle,end;
   CHECK(rr_results_card_flow(views[j][0],views[j][1],&f->content,-100,&top));CHECK(top.scroll==0);invariants(&f->content,&top,views[j][0],views[j][1]);
   CHECK(rr_results_card_flow(views[j][0],views[j][1],&f->content,top.max_scroll/2,&middle));invariants(&f->content,&middle,views[j][0],views[j][1]);
   CHECK(rr_results_card_flow(views[j][0],views[j][1],&f->content,1e9,&end));CHECK(end.scroll==end.max_scroll);invariants(&f->content,&end,views[j][0],views[j][1]);
   if(end.max_scroll>0)CHECK(near(end.panel.y+end.panel.h,views[j][1]-16));
   for(int slot=0;slot<RR_CARD_SLOTS;slot++){
    if(!(f->content.visible&(1u<<slot)))continue;
    CHECK(near(middle.boxes[slot].y,top.boxes[slot].y-middle.scroll)&&near(end.boxes[slot].y,top.boxes[slot].y-end.scroll));CHECK(near(end.boxes[slot].h,top.boxes[slot].h));
   }
   extensions+=3;
  }
 }
 RRResultsCardContent legacy=rr_results_card_legacy();RRResultsCardLayout base;CHECK(rr_results_card_flow(1600,1000,&legacy,0,&base));RRUIOverlayLayout overlay=rr_ui_results_layout(1600,1000);
 CHECK(same(base.panel,overlay.panel)&&same(base.left,overlay.left)&&same(base.right,overlay.right)&&same(base.boxes[RR_CARD_AGAIN],overlay.actions[0])&&same(base.boxes[RR_CARD_MENU],overlay.actions[1])&&same(overlay.actions[2],(RRUIRect){0}));
 /* Visible zero-height PLACE retains both surrounding9px gaps. */
 RRResultsCardContent empty=legacy;empty.heights[RR_CARD_PLACE]=0;RRResultsCardLayout e;CHECK(rr_results_card_flow(1600,1000,&empty,0,&e));CHECK(near(e.panel.h,base.panel.h-33)&&near(e.boxes[RR_CARD_TOTAL].y-e.panel.y,74.296875));
 /* Display:none omits its height/gap, even if no measurement is available. */
 empty=legacy;empty.heights[RR_CARD_UNLOCK]=NAN;CHECK(rr_results_card_flow(1600,1000,&empty,0,&e)&&same(e.panel,base.panel));
 empty=legacy;empty.visible|=1u<<RR_CARD_UNLOCK;empty.heights[RR_CARD_UNLOCK]=0;CHECK(rr_results_card_flow(1600,1000,&empty,0,&e)&&near(e.panel.h,base.panel.h+9));
 /* Derived supplied-height example: eight more unwrapped21.296875px rows,
  * optional heights from the prepared next/unlocked fixture. Not fresh DOM. */
 RRResultsCardContent tall=legacy;tall.heights[RR_CARD_COLUMNS]+=8*21.296875;tall.visible=RR_CARD_ALL;tall.heights[RR_CARD_NEXT]=64;tall.heights[RR_CARD_UNLOCK]=23;
 CHECK(rr_results_card_flow(1280,720,&tall,1e9,&e));CHECK(near(e.content_height,766.5625)&&near(e.max_scroll,78.5625)&&near(e.panel.y+e.panel.h,704));
 RRResultsCardLayout resized;CHECK(rr_results_card_flow(1280,1080,&tall,e.scroll,&resized)&&resized.scroll==0&&resized.max_scroll==0);CHECK(rr_results_card_flow(1280,720,&legacy,e.scroll,&resized)&&resized.scroll==0);
 for(unsigned i=0;i<256;i++){
  RRResultsCardContent c=legacy;c.visible|=(i&1)?1u<<RR_CARD_UNLOCK:0;c.visible|=(i&2)?1u<<RR_CARD_NEXT:0;
  for(int slot=0;slot<RR_CARD_SLOTS;slot++)c.heights[slot]=(double)((i*37u+(unsigned)slot*19u)%400u)/4;
  double w=680+(i%5)*110,h=240+(i%9)*70;RRResultsCardLayout l;
  CHECK(rr_results_card_flow(w,h,&c,(double)i*10,&l));invariants(&c,&l,w,h);generated++;
 }
 RRResultsCardLayout guard;memset(&guard,0x5a,sizeof(guard));RRResultsCardLayout saved=guard;RRResultsCardContent bad=legacy;
 const double values[]={-1,NAN,INFINITY,1e300};for(size_t i=0;i<4;i++){bad=legacy;bad.heights[RR_CARD_COLUMNS]=values[i];CHECK(!rr_results_card_flow(1600,1000,&bad,0,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));}
 bad=legacy;bad.visible|=1u<<RR_CARD_SLOTS;CHECK(!rr_results_card_flow(1600,1000,&bad,0,&guard));bad=legacy;bad.visible&=~(1u<<RR_CARD_BRAND);CHECK(!rr_results_card_flow(1600,1000,&bad,0,&guard));
 CHECK(!rr_results_card_flow(0,1000,&legacy,0,&guard)&&!rr_results_card_flow(1600,0,&legacy,0,&guard)&&!rr_results_card_flow(NAN,1000,&legacy,0,&guard)&&!rr_results_card_flow(1600,INFINITY,&legacy,0,&guard));CHECK(!rr_results_card_flow(1600,1000,&legacy,NAN,&guard)&&!rr_results_card_flow(1600,1000,NULL,0,&guard)&&!rr_results_card_flow(1600,1000,&legacy,0,NULL));CHECK(!memcmp(&guard,&saved,sizeof(guard)));
 printf("RESULTS_CARD_SUMMARY observed=%u css_derived_layouts=%u generated=%u layout_bytes=%zu content_bytes=%zu failures=%u prepared_reference_not_fresh_render=true\n",observed,extensions,generated,sizeof(RRResultsCardLayout),sizeof(RRResultsCardContent),failures);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

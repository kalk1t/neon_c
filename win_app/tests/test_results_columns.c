#include "rr_results_columns.h"
#include "rr_ui_codepoints.h"
#include "rr_css_font_metrics.h"
#include "results_rows_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,paints,boards,emitted_rows,expected_paints;
static RRResultsRow scratch,expected_row;
static RRResultsCurrent snapshot;
static RRResultsBoundView view;
static RRResultsCardLayout card;
static double tops[2];
static float first_x,first_y;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"columns line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static bool near(double a,double b){return fabs(a-b)<.002;}
static void record(void *ctx,const RRResultsPaint *p){CHECK(ctx==&card);CHECK(isfinite(p->origin.x)&&isfinite(p->origin.y));if(!paints){first_x=p->origin.x;first_y=p->origin.y;}paints++;}
static void count_paint(void *ctx,const RRResultsPaint *p){(void)ctx;(void)p;expected_paints++;}
static void setup(size_t n){
 snapshot=(RRResultsCurrent){0};view=(RRResultsBoundView){0};view.results=&snapshot;view.count=n;view.player=0;view.revision=1;snapshot.coin_count=n;snapshot.time_count=n>1?n:0;
 for(size_t i=0;i<n;i++){snapshot.times[i]=(RRResultsTime){(int)i,60+(double)i,false};snapshot.coins[i]=(RRResultsCoin){(int)i,(int)(n-i)};view.names[i]="Pilot";}
 RRResultsCardContent c=rr_results_card_legacy();c.heights[RR_CARD_COLUMNS]=300;
 CHECK(rr_results_card_flow(1600,720,&c,0,&card));tops[0]=card.left.y+30;tops[1]=card.right.y+36;
}
static RRResultsColumnsResult run(Font f,int scale){paints=0;return rr_results_columns_emit(f,&view,&card,tops,scale,&scratch,record,&card);}
int main(int argc,char **argv){CHECK(argc==2);if(argc!=2)return 1;SetTraceLogLevel(LOG_WARNING);
 int bytes=0,count=0,cps[RR_UI_MAX_CODEPOINTS];unsigned char *data=LoadFileData(argv[1],&bytes);CHECK(data);if(!data)return 1;
 int requested=rr_ui_codepoints(cps,700);GlyphInfo *glyphs=LoadFontData(data,bytes,64,cps,requested,FONT_DEFAULT,&count);CHECK(glyphs&&count==96);if(!glyphs){UnloadFileData(data);return 1;}
 Rectangle *rects=NULL;Image atlas=GenImageFontAtlas(glyphs,&rects,count,64,4,0);CHECK(atlas.data&&rects);if(!rects){UnloadFontData(glyphs,count);UnloadFileData(data);return 1;}
 Font font={0};font.texture.id=1;font.baseSize=64;font.glyphCount=count;font.glyphs=glyphs;font.recs=rects;
 for(size_t i=0;i<sizeof(row_boards)/sizeof(row_boards[0]);i++){
  const struct RowBoard *b=row_boards+i;setup(b->count);snapshot.time_count=b->time_count;memcpy(snapshot.times,b->times,sizeof(snapshot.times));memcpy(snapshot.coins,b->coins,sizeof(snapshot.coins));for(size_t j=0;j<b->count;j++)view.names[j]=b->names[j];
  for(int scale=1;scale<=3;scale++){
   RRResultsColumnsResult r=run(font,scale);CHECK(r.status==RR_RESULTS_COLUMNS_READY&&r.rows==snapshot.time_count+snapshot.coin_count);
   unsigned actual=paints;expected_paints=0;const RRUIRect columns[2]={card.left,card.right};const size_t ns[2]={snapshot.time_count,snapshot.coin_count};
   for(int k=0;k<2;k++)for(size_t row=0;row<ns[k];row++){
    CHECK(rr_results_row_build(font,&snapshot,(RRResultsRowKind)k,row,view.names,view.count,view.player,scale,columns[k].x+26,columns[k].x+columns[k].w,(float)rr_css_line_baseline(tops[k]+21.3*(double)row,21.3,15,700),&expected_row)==RR_RESULTS_ROW_READY);
    CHECK(rr_results_row_emit(&expected_row,count_paint,NULL));
   }
   CHECK(actual==expected_paints&&actual>0);boards++;emitted_rows+=(unsigned)r.rows;
  }
 }
 /* First paint geometry: source content inset and baseline, with shadow offset. */
 setup(2);CHECK(run(font,1).status==RR_RESULTS_COLUMNS_READY);CHECK(near(first_x,card.left.x+24));CHECK(near(first_y,rr_css_raylib_origin(rr_css_line_baseline(tops[0],21.3,15,700),15,700,64)));
 /* A final invalid name must prevent even the first row from painting. */
 view.names[1]="^";RRResultsColumnsResult r=run(font,1);CHECK(r.status==RR_RESULTS_COLUMNS_INVALID&&r.kind==RR_RESULTS_STANDINGS&&r.row==1&&!paints&&!r.rows);
 /* Source-valid 16-character unbroken nickname: no truncation or invented wrap. */
 setup(1);view.names[0]="WWWWWWWWWWWWWWWW";r=run(font,1);CHECK(r.status==RR_RESULTS_COLUMNS_NEEDS_WRAP&&r.kind==RR_RESULTS_COINS&&r.row==0&&!paints&&!r.rows);
 CHECK(rr_results_row_build(font,&snapshot,RR_RESULTS_COINS,0,view.names,1,0,1,0,1000,100,&scratch)==RR_RESULTS_ROW_READY);float wide_name=scratch.name.width,wide_value=scratch.value.width;CHECK(wide_name+wide_value>card.right.w-26);
 /* A later coin-column wrap must also suppress all earlier standings paints. */
 setup(2);snapshot.times[0].seconds=snapshot.times[1].seconds=0;view.names[1]="WWWWWWWWWWWW";snapshot.coins[1].coins=2147483647;for(size_t j=0;j<2;j++)CHECK(rr_results_row_build(font,&snapshot,RR_RESULTS_STANDINGS,j,view.names,2,0,1,card.left.x+26,card.left.x+card.left.w,100,&scratch)==RR_RESULTS_ROW_READY);r=run(font,1);CHECK(r.status==RR_RESULTS_COLUMNS_NEEDS_WRAP&&r.kind==RR_RESULTS_COINS&&r.row==1&&!paints&&!r.rows);
 setup(11);card.right.h=36+10*21.3f;r=run(font,1);CHECK(r.status==RR_RESULTS_COLUMNS_NEEDS_HEIGHT&&r.kind==RR_RESULTS_COINS&&!paints);
 setup(2);tops[1]=NAN;CHECK(run(font,1).status==RR_RESULTS_COLUMNS_INVALID&&!paints);
 setup(2);card.left.w=25;CHECK(run(font,1).status==RR_RESULTS_COLUMNS_INVALID&&!paints);
 setup(2);view.revision=0;CHECK(run(font,1).status==RR_RESULTS_COLUMNS_INVALID&&!paints);
 setup(2);snapshot.coin_count=1;CHECK(run(font,1).status==RR_RESULTS_COLUMNS_INVALID&&!paints);
 setup(2);CHECK(run((Font){0},1).status==RR_RESULTS_COLUMNS_INVALID&&!paints);CHECK(run(font,0).status==RR_RESULTS_COLUMNS_INVALID&&!paints);
 CHECK(rr_results_columns_emit(font,NULL,&card,tops,1,&scratch,record,&card).status==RR_RESULTS_COLUMNS_INVALID);
 CHECK(rr_results_columns_emit(font,&view,&card,tops,1,NULL,record,&card).status==RR_RESULTS_COLUMNS_INVALID);
 CHECK(rr_results_columns_emit(font,&view,&card,tops,1,&scratch,NULL,&card).status==RR_RESULTS_COLUMNS_INVALID);
 printf("RESULTS_COLUMNS_SUMMARY boards=%u rows=%u content_widths=309.734375/232.25 wide_coin_name=%.6f wide_coin_value=%.6f scratch_bytes=%zu failures=%u recorded_not_rendered=true ideal_css_line_height_not_pixel_qualification=true\n",boards,emitted_rows,wide_name,wide_value,sizeof(scratch),failures);
 UnloadImage(atlas);MemFree(rects);UnloadFontData(glyphs,count);UnloadFileData(data);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

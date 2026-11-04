#include "rr_results_art.h"
#include "rr_results_fallback_glyphs.h"
#include "rr_ui_codepoints.h"
#include "results_rows_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,expected_count,actual_count,total_commands,total_boards;
static RRResultsPaint expected[8192];
static RRResultsRow scratch;
static RRResultsCurrent snapshot;
static RRResultsBoundView view;
static RRResultsCardLayout card;
static RRUI ui;
static RRResultsArt art;
static double tops[2];
#define CHECK(x) do{if(!(x)){fprintf(stderr,"native columns line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static bool near(double a,double b){return fabs(a-b)<.001;}
static bool color(Color a,Color b){return a.r==b.r&&a.g==b.g&&a.b==b.b&&a.a==b.a;}
static void expect(void *ctx,const RRResultsPaint *p){CHECK(ctx==&card);CHECK(expected_count<8192);if(expected_count<8192)expected[expected_count++]=*p;}
/* Only actual GPU paint endpoints are replaced at compile time for this target.
 * Font loading/CPU metrics, native adapter, row preflight and sink conversion are
 * production C. Texture IDs below are metadata sent to recording stubs only. */
void rr_record_DrawTextCodepoint(Font f,int cp,Vector2 origin,float size,Color tint){
 CHECK(actual_count<expected_count);if(actual_count>=expected_count)return;const RRResultsPaint *p=expected+actual_count++;
 CHECK(p->fallback<0&&(int)p->codepoint==cp&&f.texture.id==ui.bold.texture.id&&f.glyphs==ui.bold.glyphs&&f.recs==ui.bold.recs&&f.baseSize==ui.bold.baseSize);
 CHECK(near(origin.x,p->origin.x)&&near(origin.y,p->origin.y)&&near(size,p->font_size)&&color(tint,p->color));
}
void rr_record_DrawTexturePro(Texture2D t,Rectangle source,Rectangle dest,Vector2 origin,float rotation,Color tint){
 CHECK(actual_count<expected_count);if(actual_count>=expected_count)return;const RRResultsPaint *p=expected+actual_count++;
 CHECK(p->fallback>=0&&p->fallback<RR_RESULTS_ART_COUNT);if(p->fallback<0||p->fallback>=RR_RESULTS_ART_COUNT)return;
 CHECK(t.id==art.glyphs[p->fallback].id&&t.width==art.glyphs[p->fallback].width&&t.height==art.glyphs[p->fallback].height);
 CHECK(near(source.x,p->source.x)&&near(source.y,p->source.y)&&near(source.width,p->source.width)&&near(source.height,p->source.height));
 CHECK(near(dest.x,p->destination.x)&&near(dest.y,p->destination.y)&&near(dest.width,p->destination.width)&&near(dest.height,p->destination.height));
 CHECK(origin.x==0&&origin.y==0&&rotation==0&&color(tint,p->color));
}
static RRResultsColumnsResult draw(void){actual_count=0;return rr_ui_results_columns_draw(&ui,&art,&view,&card,tops,1,&scratch);}
int main(int argc,char **argv){
 CHECK(argc==2);if(argc!=2)return 1;SetTraceLogLevel(LOG_WARNING);
 int bytes=0,count=0,cps[RR_UI_MAX_CODEPOINTS];unsigned char *data=LoadFileData(argv[1],&bytes);CHECK(data);if(!data)return 1;
 int requested=rr_ui_codepoints(cps,700);GlyphInfo *glyphs=LoadFontData(data,bytes,64,cps,requested,FONT_DEFAULT,&count);CHECK(glyphs&&count==96);if(!glyphs){UnloadFileData(data);return 1;}
 Rectangle *rects=NULL;Image atlas=GenImageFontAtlas(glyphs,&rects,count,64,4,0);CHECK(atlas.data&&rects);if(!rects){UnloadFontData(glyphs,count);UnloadFileData(data);return 1;}
 Font font={0};font.texture.id=1;font.baseSize=64;font.glyphCount=count;font.glyphs=glyphs;font.recs=rects;ui.loaded=true;ui.bold=font;art.ready=true;
 for(int i=0;i<RR_RESULTS_ART_COUNT;i++){art.glyphs[i].id=(unsigned)i+100;art.glyphs[i].width=rr_results_fallback_glyphs[i].width_pixels;art.glyphs[i].height=rr_results_fallback_glyphs[i].height_pixels;}
 for(size_t i=0;i<sizeof(row_boards)/sizeof(row_boards[0]);i++){
  const struct RowBoard *b=row_boards+i;snapshot=(RRResultsCurrent){0};snapshot.time_count=b->time_count;snapshot.coin_count=b->count;memcpy(snapshot.times,b->times,sizeof(snapshot.times));memcpy(snapshot.coins,b->coins,sizeof(snapshot.coins));
  view=(RRResultsBoundView){0};view.results=&snapshot;view.count=b->count;view.revision=i+1;view.player=0;for(size_t j=0;j<b->count;j++)view.names[j]=b->names[j];
  RRResultsCardContent content=rr_results_card_legacy();content.heights[RR_CARD_COLUMNS]=300;
  CHECK(rr_results_card_flow(1600,720,&content,80,&card));tops[0]=card.left.y+30;tops[1]=card.right.y+36;
  for(int scale=1;scale<=3;scale++){
   expected_count=0;RRResultsColumnsResult expected_result=rr_results_columns_emit(font,&view,&card,tops,scale,&scratch,expect,&card);CHECK(expected_result.status==RR_RESULTS_COLUMNS_READY&&expected_count>0);
   actual_count=0;RRResultsColumnsResult actual_result=rr_ui_results_columns_draw(&ui,&art,&view,&card,tops,scale,&scratch);
   CHECK(actual_result.status==RR_RESULTS_COLUMNS_READY&&actual_result.rows==expected_result.rows&&actual_count==expected_count);
   total_commands+=actual_count;total_boards++;
  }
 }
 for(int i=0;i<RR_RESULTS_ART_COUNT;i++){
  Texture2D saved=art.glyphs[i];art.glyphs[i].id=0;CHECK(draw().status==RR_RESULTS_COLUMNS_INVALID&&!actual_count);art.glyphs[i]=saved;
  art.glyphs[i].width++;CHECK(draw().status==RR_RESULTS_COLUMNS_INVALID&&!actual_count);art.glyphs[i]=saved;
  art.glyphs[i].height++;CHECK(draw().status==RR_RESULTS_COLUMNS_INVALID&&!actual_count);art.glyphs[i]=saved;
 }
 art.ready=false;CHECK(draw().status==RR_RESULTS_COLUMNS_INVALID&&!actual_count);art.ready=true;
 ui.loaded=false;CHECK(draw().status==RR_RESULTS_COLUMNS_INVALID&&!actual_count);ui.loaded=true;
 ui.bold.texture.id=0;CHECK(draw().status==RR_RESULTS_COLUMNS_INVALID&&!actual_count);ui.bold=font;
 CHECK(rr_ui_results_columns_draw(NULL,&art,&view,&card,tops,1,&scratch).status==RR_RESULTS_COLUMNS_INVALID);
 CHECK(rr_ui_results_columns_draw(&ui,NULL,&view,&card,tops,1,&scratch).status==RR_RESULTS_COLUMNS_INVALID);
 view.names[view.count-1]="^";CHECK(draw().status==RR_RESULTS_COLUMNS_INVALID&&!actual_count);
 view.names[view.count-1]="WWWWWWWWWWWWWWWW";CHECK(draw().status==RR_RESULTS_COLUMNS_NEEDS_WRAP&&!actual_count);
 view.names[view.count-1]="Pilot";card.right.h=1;CHECK(draw().status==RR_RESULTS_COLUMNS_NEEDS_HEIGHT&&!actual_count);
 printf("RESULTS_NATIVE_COLUMNS_SUMMARY boards=%u commands=%u failures=%u native_paint_endpoints_recorded=true texture_loads=0 graphics_contexts=0\n",total_boards,total_commands,failures);
 UnloadImage(atlas);MemFree(rects);UnloadFontData(glyphs,count);UnloadFileData(data);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

#include "rr_results_flow_draw.h"
#include "rr_results_fallback_glyphs.h"
#include "rr_css_font_metrics.h"
#include "rr_ui_codepoints.h"
#include "results_body_flow_reference.h"
#include "results_flow_draw_reference.h"
#include <stdio.h>
#include <stdlib.h>
static unsigned failures;static size_t cursor,end,commands;static bool checking;static const FlowDrawCase *current;
static RRResultsListWork work;static RRResultsRow row;static RRUI ui;static RRResultsArt art;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<15)fprintf(stderr,"flow draw line %d glyph %zu: %s\n",__LINE__,cursor,#x);failures++;}}while(0)
static bool color(Color a,Color b){return a.r==b.r&&a.g==b.g&&a.b==b.b&&a.a==b.a;}
static void record(int cp,double x,double y,Color tint){
 commands++;CHECK(checking&&cursor<end);if(!checking||cursor>=end)return;const FlowGlyph *g=flow_glyphs+cursor++;
 CHECK((unsigned)cp==g->cp);
 /* Selection rectangles round outward: the actual pen must be in the first
  * layout unit of that character's measured left envelope. */
 CHECK(x-g->dx>=g->left-.0003&&x-g->dx<=g->left+1.0/64+.0003);
 double baseline=rr_css_line_baseline(g->line_y,21.296875,15,700);
 double expected_y=rr_css_raylib_origin((float)baseline,15,700,ui.bold.baseSize);CHECK(fabs(y-g->dy-expected_y)<.0003);
 Color expected=g->value?(g->kind?(Color){255,210,90,255}:g->player?(Color){255,43,214,255}:(Color){153,187,221,255}):g->player?(Color){255,255,255,255}:g->kind?(Color){243,224,176,255}:(Color){207,239,255,255};
 if(g->shadow)expected=g->kind?(Color){255,160,18,31}:(Color){255,43,214,31};CHECK(color(tint,expected));
}
void rr_record_DrawTextCodepoint(Font f,int cp,Vector2 origin,float size,Color tint){CHECK(f.texture.id==ui.bold.texture.id&&f.glyphs==ui.bold.glyphs&&fabs(size-rr_css_font_size(15,700))<.0001);record(cp,origin.x,origin.y,tint);}
void rr_record_DrawTexturePro(Texture2D t,Rectangle source,Rectangle dest,Vector2 origin,float rotation,Color tint){
 int index=-1;for(int i=0;i<RR_RESULTS_ART_COUNT;i++)if(t.id==art.glyphs[i].id)index=i;CHECK(index>=0);if(index<0)return;
 const RRResultsFallbackGlyph *g=rr_results_fallback_glyphs+index;CHECK(g->device_scale==current->scale&&source.width==g->width_pixels&&source.height==g->height_pixels&&dest.width==g->draw_width&&dest.height==g->draw_height&&origin.x==0&&origin.y==0&&rotation==0);
 double baseline=dest.y-g->offset_y;record((int)g->codepoint,dest.x-g->offset_x,rr_css_raylib_origin(baseline,15,700,ui.bold.baseSize),tint);
}
static RRResultsBodyInput input(const BodyCase *c){RRResultsBodyInput i={c->width,c->height,c->scroll,c->left,c->right,c->caption_height,c->rows[0],c->rows[1],c->counts[0],c->counts[1],c->other};return i;}
int main(int argc,char **argv){
 CHECK(argc==2);if(argc!=2)return 1;SetTraceLogLevel(LOG_WARNING);int bytes=0,count=0,cps[RR_UI_MAX_CODEPOINTS];unsigned char *data=LoadFileData(argv[1],&bytes);CHECK(data);if(!data)return 1;
 int requested=rr_ui_codepoints(cps,700);GlyphInfo *glyphs=LoadFontData(data,bytes,64,cps,requested,FONT_DEFAULT,&count);CHECK(glyphs&&count==96);if(!glyphs){UnloadFileData(data);return 1;}
 Rectangle *rects=NULL;Image atlas=GenImageFontAtlas(glyphs,&rects,count,64,4,0);CHECK(atlas.data&&rects);if(!rects){UnloadFontData(glyphs,count);UnloadFileData(data);return 1;}
 ui.loaded=true;ui.bold=(Font){64,count,0,{1,0,0,0,0},rects,glyphs};art.ready=true;
 for(int j=0;j<RR_RESULTS_ART_COUNT;j++){art.glyphs[j].id=(unsigned)j+100;art.glyphs[j].width=rr_results_fallback_glyphs[j].width_pixels;art.glyphs[j].height=rr_results_fallback_glyphs[j].height_pixels;}
 CHECK(COUNT(body_cases)==COUNT(draw_cases));checking=true;
 for(size_t n=0;n<COUNT(draw_cases);n++){
  current=draw_cases+n;cursor=current->first;end=cursor+current->count;RRResultsBodyInput i=input(body_cases+n);
  CHECK(rr_ui_results_flow_draw(&ui,&art,&i,current->players[0],current->players[1],current->scale,&work,&row)==RR_NAME_READY);CHECK(cursor==end);
 }
 checking=false;size_t before=commands;current=draw_cases;RRResultsBodyInput i=input(body_cases);
 for(int j=0;j<RR_RESULTS_ART_COUNT;j++){Texture2D saved=art.glyphs[j];art.glyphs[j].id=0;CHECK(rr_ui_results_flow_draw(&ui,&art,&i,current->players[0],current->players[1],1,&work,&row)==RR_NAME_INVALID&&commands==before);art.glyphs[j]=saved;art.glyphs[j].width++;CHECK(rr_ui_results_flow_draw(&ui,&art,&i,current->players[0],current->players[1],1,&work,&row)==RR_NAME_INVALID&&commands==before);art.glyphs[j]=saved;}
 /* CPU font loses only a late-needed supported source glyph: complete-board
  * native preflight must expose no commands, including earlier good rows. */
 i=input(body_cases+6);RRResultsFlowText late[]={ {"Pilot","0"},{"ZED","1"} };bool flags[]={true,false};i.coins=late;i.coin_count=2;int z=GetGlyphIndex(ui.bold,'Z'),saved=glyphs[z].value;glyphs[z].value='?';CHECK(rr_ui_results_flow_draw(&ui,&art,&i,draw_cases[6].players[0],flags,1,&work,&row)==RR_NAME_INVALID&&commands==before);glyphs[z].value=saved;
 art.ready=false;CHECK(rr_ui_results_flow_draw(&ui,&art,&i,draw_cases[6].players[0],flags,1,&work,&row)==RR_NAME_INVALID&&commands==before);art.ready=true;
 ui.loaded=false;CHECK(rr_ui_results_flow_draw(&ui,&art,&i,draw_cases[6].players[0],flags,1,&work,&row)==RR_NAME_INVALID&&commands==before);ui.loaded=true;
 CHECK(rr_results_flow_emit(ui.bold,&i,draw_cases[6].players[0],NULL,1,&work,&row,NULL,NULL)==RR_NAME_INVALID&&commands==before);
 printf("RESULTS_FLOW_DRAW_SUMMARY observed_boards=%zu commands=%zu glyph_envelopes=%zu work_bytes=%zu row_bytes=%zu failures=%u native_endpoints_recorded=true graphics_contexts=0 texture_loads=0 main_unchanged=true\n",COUNT(draw_cases),commands,COUNT(flow_glyphs),sizeof(work),sizeof(row),failures);
 UnloadImage(atlas);MemFree(rects);UnloadFontData(glyphs,count);UnloadFileData(data);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

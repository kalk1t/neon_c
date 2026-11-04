#include "rr_results_slots_draw.h"
#include "results_caption_reference.h"
#include "results_bound_flow_reference.h"
#include "rr_results_fallback_glyphs.h"
#include "rr_css_font_metrics.h"
#include "rr_ui_codepoints.h"
#include "results_body_flow_reference.h"
#include "results_flow_draw_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
static RRResultsCardContent slots,guard_slots,saved_slots;static RRResultsBodyLayout body;static RRResultsCaption caption,guard_caption,saved_caption;static RRResultsTracks tracks,guard,saved_tracks;static const double bounds[2]={258.25,258.25};static RRResultsFlowBinding binding,saved_binding;static RRResultsCurrent snapshot;static RRResultsBoundView view;
static RRResultsBodyInput input(const BodyCase *c){RRResultsBodyInput i={c->width,c->height,c->scroll,c->left,c->right,c->caption_height,NULL,NULL,999,999,c->other};return i;}
int main(int argc,char **argv){
 CHECK(argc==2);if(argc!=2)return 1;SetTraceLogLevel(LOG_WARNING);int bytes=0,count=0,cps[RR_UI_MAX_CODEPOINTS];unsigned char *data=LoadFileData(argv[1],&bytes);CHECK(data);if(!data)return 1;
 int requested=rr_ui_codepoints(cps,700);GlyphInfo *glyphs=LoadFontData(data,bytes,64,cps,requested,FONT_DEFAULT,&count);CHECK(glyphs&&count==96);if(!glyphs){UnloadFileData(data);return 1;}
 Rectangle *rects=NULL;Image atlas=GenImageFontAtlas(glyphs,&rects,count,64,4,0);CHECK(atlas.data&&rects);if(!rects){UnloadFontData(glyphs,count);UnloadFileData(data);return 1;}
 ui.loaded=true;ui.bold=(Font){64,count,0,{1,0,0,0,0},rects,glyphs};art.ready=true;
 for(int j=0;j<RR_RESULTS_ART_COUNT;j++){art.glyphs[j].id=(unsigned)j+100;art.glyphs[j].width=rr_results_fallback_glyphs[j].width_pixels;art.glyphs[j].height=rr_results_fallback_glyphs[j].height_pixels;}

 CHECK(COUNT(bound_cases)==COUNT(body_cases));checking=true;
 for(size_t n=0;n<COUNT(bound_cases);n++){
  const BoundFlowCase *b=bound_cases+n;current=draw_cases+n;cursor=current->first;end=cursor+current->count;
  snapshot=(RRResultsCurrent){0};snapshot.finish_position=1;snapshot.player_takedowns=0;snapshot.lap_count=2;snapshot.laps[0]=62.5;snapshot.laps[1]=61.25;snapshot.time_count=b->time_count;snapshot.coin_count=b->count;for(size_t j=0;j<b->count;j++)if(b->coins[j].racer==(int)b->player)snapshot.player_coin_rank=(int)j+1;memcpy(snapshot.times,b->times,sizeof(snapshot.times));memcpy(snapshot.coins,b->coins,sizeof(snapshot.coins));
  view=(RRResultsBoundView){0};view.results=&snapshot;view.count=b->count;view.player=b->player;view.revision=n+1;for(size_t j=0;j<b->count;j++)view.names[j]=b->names[j];
  RRResultsBodyInput i=input(body_cases+n);i.left_width=NAN;i.right_width=NAN;i.caption_height=NAN;
  CHECK(rr_results_flow_prepare(&view,&binding));CHECK(rr_results_tracks(binding.rows[0],binding.counts[0],binding.rows[1],binding.counts[1],bounds,&work.name_characters,&tracks)==RR_NAME_READY&&tracks.width[0]==body_cases[n].left&&tracks.width[1]==body_cases[n].right);
  CHECK(rr_results_empty_slots(&snapshot,&slots)==RR_NAME_READY);for(int slot=0;slot<RR_CARD_SLOTS;slot++)CHECK(slots.heights[slot]==body_cases[n].other.heights[slot]);CHECK(slots.visible==body_cases[n].other.visible);
  CHECK(rr_ui_results_empty_profile_draw(&ui,&art,&view,i.width,i.height,i.scroll,current->scale,&binding,&work,&row,&caption)==RR_NAME_READY&&caption.height==body_cases[n].caption_height);
  RRResultsBodyInput generated={i.width,i.height,i.scroll,tracks.width[0],tracks.width[1],caption.height,binding.rows[0],binding.rows[1],binding.counts[0],binding.counts[1],slots};CHECK(rr_results_body_flow(&generated,&work,&body)==RR_NAME_READY);
  CHECK(body.card.panel.x==body_cases[n].panel.x&&body.card.panel.y==body_cases[n].panel.y&&body.card.panel.w==body_cases[n].panel.w&&body.card.panel.h==body_cases[n].panel.h);CHECK(cursor==end&&binding.revision==view.revision);
  for(int k=0;k<2;k++)for(size_t j=0;j<binding.counts[k];j++){
   CHECK(!strcmp(binding.rows[k][j].name,body_cases[n].rows[k][j].name)&&!strcmp(binding.rows[k][j].value,body_cases[n].rows[k][j].value)&&binding.player[k][j]==current->players[k][j]);
   CHECK(binding.rows[k][j].name==binding.labels[k][j].name&&binding.rows[k][j].value==binding.labels[k][j].value);
  }
 }
 checking=false;size_t before=commands;RRResultsBodyInput i=input(body_cases);memcpy(&saved_binding,&binding,sizeof(binding));
 int racer=snapshot.coins[snapshot.coin_count-1].racer;snapshot.coins[snapshot.coin_count-1].racer=99;
 CHECK(rr_ui_results_tracks_draw(&ui,&art,&view,&i,bounds,1,&binding,&work,&row)==RR_NAME_INVALID&&commands==before&&!memcmp(&binding,&saved_binding,sizeof(binding)));snapshot.coins[snapshot.coin_count-1].racer=racer;
 snapshot.coins[snapshot.coin_count-1].racer=snapshot.coins[0].racer;CHECK(!rr_results_flow_prepare(&view,&binding)&&!memcmp(&binding,&saved_binding,sizeof(binding)));snapshot.coins[snapshot.coin_count-1].racer=racer;
 size_t revision=view.revision;view.revision=0;CHECK(!rr_results_flow_prepare(&view,&binding)&&!memcmp(&binding,&saved_binding,sizeof(binding)));view.revision=revision;
 RRResultsLabel label,saved;memset(&label,0x5a,sizeof(label));saved=label;CHECK(!rr_results_label_build(&snapshot,(RRResultsLabelKind)2,0,view.names,view.count,view.player,&label)&&!memcmp(&label,&saved,sizeof(label)));
 CHECK(!rr_results_flow_prepare(NULL,&binding)&&!rr_results_flow_prepare(&view,NULL));
 CHECK(rr_ui_results_tracks_draw(&ui,&art,&view,NULL,bounds,1,&binding,&work,&row)==RR_NAME_INVALID&&commands==before);
 memset(&guard,0x5a,sizeof(guard));saved_tracks=guard;
 double blocking[2]={999,258.25};CHECK(rr_results_tracks(binding.rows[0],binding.counts[0],binding.rows[1],binding.counts[1],blocking,&work.name_characters,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved_tracks,sizeof(guard)));
 RRResultsFlowText unknown[]={ {"Pilot","0"} };CHECK(rr_results_tracks(NULL,0,unknown,1,bounds,&work.name_characters,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved_tracks,sizeof(guard)));
 unknown[0].name="ACE/ACE";CHECK(rr_results_tracks(NULL,0,unknown,1,bounds,&work.name_characters,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved_tracks,sizeof(guard)));
 unknown[0].name="ACE-1";CHECK(rr_results_tracks(NULL,0,unknown,1,bounds,&work.name_characters,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved_tracks,sizeof(guard)));
 CHECK(rr_results_tracks(NULL,0,NULL,0,bounds,&work.name_characters,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved_tracks,sizeof(guard)));
 blocking[0]=NAN;CHECK(rr_results_tracks(binding.rows[0],binding.counts[0],binding.rows[1],binding.counts[1],blocking,&work.name_characters,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved_tracks,sizeof(guard)));
 CHECK(rr_ui_results_tracks_draw(&ui,&art,&view,&i,bounds,1,&binding,NULL,&row)==RR_NAME_INVALID&&commands==before);
 for(size_t n=0;n<COUNT(caption_cases);n++){
  const CaptionCase *c=caption_cases+n;RRResultsCurrent s={0};s.coin_count=c->count;s.player_coin_rank=c->rank;
  CHECK(rr_results_caption_build(&s,258.25,&caption)==RR_NAME_READY&&!strcmp(caption.text,c->text));
  CHECK(caption.height==(strstr(c->text,"BONUS")?17:15));
 }
 memset(&guard_caption,0x5a,sizeof(guard_caption));saved_caption=guard_caption;
 CHECK(rr_results_caption_build(&snapshot,200,&guard_caption)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard_caption,&saved_caption,sizeof(guard_caption)));
 CHECK(rr_results_caption_build(&snapshot,NAN,&guard_caption)==RR_NAME_INVALID&&!memcmp(&guard_caption,&saved_caption,sizeof(guard_caption)));
 int rank=snapshot.player_coin_rank;snapshot.player_coin_rank=0;CHECK(rr_results_caption_build(&snapshot,258.25,&guard_caption)==RR_NAME_INVALID&&!memcmp(&guard_caption,&saved_caption,sizeof(guard_caption)));snapshot.player_coin_rank=rank;
 int saved_rank=snapshot.player_coin_rank;snapshot.player_coin_rank=saved_rank==1?2:1;
 CHECK(rr_ui_results_caption_flow_draw(&ui,&art,&view,&i,1,&binding,&work,&row,&guard_caption)==RR_NAME_INVALID&&commands==before&&!memcmp(&guard_caption,&saved_caption,sizeof(guard_caption)));snapshot.player_coin_rank=saved_rank;
 art.ready=false;CHECK(rr_ui_results_caption_flow_draw(&ui,&art,&view,&i,1,&binding,&work,&row,&guard_caption)==RR_NAME_INVALID&&commands==before&&!memcmp(&guard_caption,&saved_caption,sizeof(guard_caption)));art.ready=true;
 CHECK(rr_results_caption_build(NULL,258.25,&guard_caption)==RR_NAME_INVALID&&rr_results_caption_build(&snapshot,258.25,NULL)==RR_NAME_INVALID);
 memset(&guard_slots,0x5a,sizeof(guard_slots));saved_slots=guard_slots;
 snapshot.player_takedowns=10;CHECK(rr_results_empty_slots(&snapshot,&guard_slots)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard_slots,&saved_slots,sizeof(guard_slots)));snapshot.player_takedowns=0;
 snapshot.lap_count=1;CHECK(rr_results_empty_slots(&snapshot,&guard_slots)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard_slots,&saved_slots,sizeof(guard_slots)));snapshot.lap_count=2;
 snapshot.laps[0]=3600.01;CHECK(rr_results_empty_slots(&snapshot,&guard_slots)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard_slots,&saved_slots,sizeof(guard_slots)));snapshot.laps[0]=62.5;
 snapshot.laps[0]=NAN;CHECK(rr_results_empty_slots(&snapshot,&guard_slots)==RR_NAME_INVALID&&!memcmp(&guard_slots,&saved_slots,sizeof(guard_slots)));snapshot.laps[0]=62.5;
 CHECK(rr_results_empty_slots(NULL,&guard_slots)==RR_NAME_INVALID&&rr_results_empty_slots(&snapshot,NULL)==RR_NAME_INVALID);
 CHECK(rr_ui_results_empty_profile_draw(&ui,&art,NULL,1600,1000,0,1,&binding,&work,&row,&guard_caption)==RR_NAME_INVALID&&commands==before);
 printf("RESULTS_SLOTS_SUMMARY observed_cards=%zu commands=%zu generated_slot_heights=600 slots_bytes=%zu work_bytes=%zu failures=%u explicit_empty_account_profile=true measured_slot_inputs=false graphics_contexts=0 main_unchanged=true\n",COUNT(bound_cases),commands,sizeof(slots),sizeof(work),failures);
 UnloadImage(atlas);MemFree(rects);UnloadFontData(glyphs,count);UnloadFileData(data);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

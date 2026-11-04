from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');backup=p/'reference/pre-results-flow-draw-20261104';backup.mkdir(exist_ok=False)
for f in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 dest=backup/f;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/f,dest)
header=r'''#ifndef RR_RESULTS_FLOW_DRAW_H
#define RR_RESULTS_FLOW_DRAW_H
#include "rr_results_art.h"
#include "rr_results_body_flow.h"
/* Frozen source-order strings and player flags. Input/font/work/row/flags/out
 * must remain disjoint and stable through synchronous preflight and emission;
 * sinks may record/paint but must not mutate those inputs or retain views.
 * Caller owns persistent list work and one row scratch. No heap/large local
 * text plans. Geometry follows qualified body flow; primary/fallback pen origins
 * use verified raw font advances, not raylib atlas advance approximations.
 * Baselines/ink/shadow raster remain nominal native approximations. Ordinals,
 * headings, caption and card decoration are not painted by this content bridge. */
RRResultsNameStatus rr_results_flow_emit(Font font,const RRResultsBodyInput *input,
 const bool *standings_player,const bool *coin_player,int device_scale,
 RRResultsListWork *work,RRResultsRow *row,RRResultsPaintSink sink,void *context);
/* Loaded-resource metadata preflight before all commands. Does not load images,
 * create a graphics/audio context, tick gameplay or activate main/accounts. */
RRResultsNameStatus rr_ui_results_flow_draw(RRUI *ui,RRResultsArt *art,
 const RRResultsBodyInput *input,const bool *standings_player,const bool *coin_player,
 int device_scale,RRResultsListWork *work,RRResultsRow *row);
#endif
'''
source=r'''#include "rr_results_flow_draw.h"
#include "rr_results_fallback_glyphs.h"
#include "rr_css_font_metrics.h"
#include <float.h>
#include <string.h>
typedef struct FlowSink {Font font;const RRResultsBodyInput *input;const bool *player[2];const RRResultsBodyLayout *body;RRResultsRow *row;RRResultsPaintSink sink;void *context;int scale,kind;bool ready;} FlowSink;
static void no_paint(void *context,const RRResultsPaint *paint){(void)context;(void)paint;}
static double pen(const RRResultsCharacters *c,size_t index){return index==c->count?c->advance:c->ranges[index].pen;}
static bool fragment(FlowSink *s,const char *text,const RRResultsCharacters *chars,size_t first,size_t count,double x,double top,bool value,bool player)
{
 RRResultsRow *row=s->row;
 if(!count||first>chars->count||count>chars->count-first||!isfinite(x)||!isfinite(top)||fabs(x)>FLT_MAX||fabs(top)>FLT_MAX||!rr_results_text_build(s->font,text,s->scale,&row->name)||row->name.count!=chars->count||!rr_results_text_build(s->font,"",s->scale,&row->value))return false;
 double origin=pen(chars,first);
 for(size_t j=0;j<count;j++){
  RRResultsToken token=row->name.tokens[first+j];if(token.codepoint!=chars->ranges[first+j].codepoint)return false;
  token.pen=(float)(pen(chars,first+j)-origin);token.advance=(float)(pen(chars,first+j+1)-pen(chars,first+j));row->name.tokens[j]=token;
 }
 row->name.count=count;row->name.width=(float)(pen(chars,first+count)-origin);
 row->left=(float)x;row->value_x=(float)x;row->baseline=(float)rr_css_line_baseline(top,21.296875,15,700);row->shadow=player;
 row->name_color=value?(s->kind?(Color){255,210,90,255}:player?(Color){255,43,214,255}:(Color){153,187,221,255}):player?(Color){255,255,255,255}:s->kind?(Color){243,224,176,255}:(Color){207,239,255,255};
 row->value_color=row->name_color;row->shadow_color=s->kind?(Color){255,160,18,255}:(Color){255,43,214,255};
 return rr_results_row_emit(row,s->sink,s->context);
}
static void row_flow(void *context,const RRResultsListRowView *v)
{
 FlowSink *s=context;if(!s->ready)return;const RRResultsFlowText *rows=s->kind?s->input->coins:s->input->standings;const RRUIRect box=s->body->list_boxes[s->kind];bool player=s->player[s->kind][v->index];
 for(size_t j=0;j<v->name->count;j++){
  const RRResultsNameLine *line=v->name->lines+j;
  if(!fragment(s,rows[v->index].name,v->name_characters,line->first,line->count,(double)box.x+26+line->x,(double)box.y+v->y+line->y-1,false,player)){s->ready=false;return;}
 }
 s->ready=fragment(s,rows[v->index].value,v->value_characters,0,v->value_characters->count,(double)box.x+26+v->value.x,(double)box.y+v->value.y,true,player);
}
RRResultsNameStatus rr_results_flow_emit(Font font,const RRResultsBodyInput *input,const bool *standings_player,const bool *coin_player,int scale,RRResultsListWork *work,RRResultsRow *row,RRResultsPaintSink sink,void *context)
{
 if(!input||!work||!row||!sink||scale<1||scale>3||(input->standings_count&&!standings_player)||(input->coin_count&&!coin_player))return RR_NAME_INVALID;
 RRResultsBodyLayout body;RRResultsNameStatus status=rr_results_body_flow(input,work,&body);if(status!=RR_NAME_READY)return status;
 for(int pass=0;pass<2;pass++){
  FlowSink s={font,input,{standings_player,coin_player},&body,row,pass?sink:no_paint,pass?context:NULL,scale,0,true};
  for(int k=0;k<2;k++){
   s.kind=k;RRResultsListSummary summary;
   status=rr_results_list_flow(k?RR_FLOW_COINS:RR_FLOW_STANDINGS,k?input->coins:input->standings,k?input->coin_count:input->standings_count,k?input->right_width-26:input->left_width-26,work,row_flow,&s,&summary);
   if(status!=RR_NAME_READY)return status;if(!s.ready)return RR_NAME_INVALID;
  }
 }
 return RR_NAME_READY;
}
typedef struct NativeFlow {Font font;RRResultsArt *art;} NativeFlow;
static void native_paint(void *context,const RRResultsPaint *p)
{NativeFlow *s=context;if(p->fallback<0)DrawTextCodepoint(s->font,(int)p->codepoint,p->origin,p->font_size,p->color);else DrawTexturePro(s->art->glyphs[p->fallback],p->source,p->destination,(Vector2){0},0,p->color);}
RRResultsNameStatus rr_ui_results_flow_draw(RRUI *ui,RRResultsArt *art,const RRResultsBodyInput *input,const bool *standings_player,const bool *coin_player,int scale,RRResultsListWork *work,RRResultsRow *row)
{
 if(!ui||!ui->loaded||!ui->bold.texture.id||!art||!art->ready)return RR_NAME_INVALID;
 for(int i=0;i<RR_RESULTS_ART_COUNT;i++)if(!art->glyphs[i].id||art->glyphs[i].width!=rr_results_fallback_glyphs[i].width_pixels||art->glyphs[i].height!=rr_results_fallback_glyphs[i].height_pixels)return RR_NAME_INVALID;
 NativeFlow sink={ui->bold,art};return rr_results_flow_emit(ui->bold,input,standings_player,coin_player,scale,work,row,native_paint,&sink);
}
'''
for f,s in [('src/presentation/rr_results_flow_draw.h',header),('src/presentation/rr_results_flow_draw.c',source)]:assert not(p/f).exists();(p/f).write_text(s,encoding='utf-8')
pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1');obs=json.loads((pack/'observations.json').read_text(encoding='utf-8'));body=json.loads((p/'reference/ui-parity-20261103/results-body-flow-capture/reference.json').read_text(encoding='utf-8'));lookup={(o['fixture'],o['scale'],o['offset']):o for o in obs};cases=[];glyphs=[]
for c in body['cases']:
 o=lookup[(c['fixture'],c['scale'],c['offset'])];flags=[];fragments=0;start=len(glyphs)
 for k,kind in enumerate(['res-standings','res-coins']):
  flags.append([])
  for row in o['lists'][kind]['rows']:
   player='me'in row['className'].split();flags[-1].append(player)
   # Browser character envelopes/line membership only. No emitter pens/baselines
   # or prototype line-break decisions generate these expectations.
   for value,nodekey in [(False,'directTextNodes'),(True,'spanTextNodes')]:
    nodes=row['valueSpan']['textNodes'] if value else row['directTextNodes']
    for node in nodes:
     chars=node['characterRectsViewport'];ys=sorted(set(rect['y']for ch in chars for rect in ch['rects']if rect['width']>0))
     for y in ys:
      visible=[(ch,rect)for ch in chars for rect in ch['rects']if rect['width']>0 and rect['y']==y]
      if not visible:continue
      fragments+=1
      for pass_index in range(5 if player else 1):
       shadow=player and pass_index<4;dx=-2 if shadow and pass_index==0 else 2 if shadow and pass_index==1 else 0;dy=-2 if shadow and pass_index==2 else 2 if shadow and pass_index==3 else 0
       for ch,rect in visible:
        cp=ord(ch['text']);glyphs.append(dict(cp=cp,left=rect['x'],right=rect['x']+rect['width'],line_y=y-1,kind=k,player=player,value=value,shadow=shadow,dx=dx,dy=dy))
 cases.append(dict(fixture=c['fixture'],scale=c['scale'],offset=c['offset'],start=start,count=len(glyphs)-start,flags=flags,fragments=fragments))
def nums(xs):return '{'+','.join(str(x)for x in xs)+'}'
fixture='''/* Source character selection envelopes; not native glyph ink bounds. */
typedef struct FlowGlyph {unsigned cp;double left,right,line_y;bool kind,player,value,shadow;int dx,dy;} FlowGlyph;
typedef struct FlowDrawCase {int scale;size_t first,count;bool players[2][11];} FlowDrawCase;
static const FlowGlyph flow_glyphs[]={
'''
for g in glyphs:fixture+=nums([g['cp'],g['left'],g['right'],g['line_y'],int(g['kind']),int(g['player']),int(g['value']),int(g['shadow']),g['dx'],g['dy']])+',\n'
fixture+='};\nstatic const FlowDrawCase draw_cases[]={\n'
for c in cases:fixture+='{%d,%d,%d,{%s}},\n'%(c['scale'],c['start'],c['count'],','.join(nums([int(x)for x in flags])if flags else '{0}'for flags in c['flags']))
fixture+='};\n';(p/'tests/results_flow_draw_reference.h').write_text(fixture,encoding='ascii')
test=r'''#include "rr_results_flow_draw.h"
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
 i=input(body_cases+6);RRResultsFlowText late[]={ {"ACE","0"},{"Z","1"} };CHECK(rr_results_characters(late[0].name,&work.name_characters)==RR_ADVANCE_READY&&rr_results_characters(late[1].name,&work.name_characters)==RR_ADVANCE_READY);bool flags[]={true,false};i.coins=late;i.coin_count=2;int z=GetGlyphIndex(ui.bold,'Z'),saved=glyphs[z].value;glyphs[z].value='?';CHECK(rr_ui_results_flow_draw(&ui,&art,&i,draw_cases[6].players[0],flags,1,&work,&row)==RR_NAME_INVALID&&commands==before);glyphs[z].value=saved;
 art.ready=false;CHECK(rr_ui_results_flow_draw(&ui,&art,&i,draw_cases[6].players[0],flags,1,&work,&row)==RR_NAME_INVALID&&commands==before);art.ready=true;
 ui.loaded=false;CHECK(rr_ui_results_flow_draw(&ui,&art,&i,draw_cases[6].players[0],flags,1,&work,&row)==RR_NAME_INVALID&&commands==before);ui.loaded=true;
 CHECK(rr_results_flow_emit(ui.bold,&i,draw_cases[6].players[0],NULL,1,&work,&row,NULL,NULL)==RR_NAME_INVALID&&commands==before);
 printf("RESULTS_FLOW_DRAW_SUMMARY observed_boards=%zu commands=%zu glyph_envelopes=%zu work_bytes=%zu row_bytes=%zu failures=%u native_endpoints_recorded=true graphics_contexts=0 texture_loads=0 main_unchanged=true\n",COUNT(draw_cases),commands,COUNT(flow_glyphs),sizeof(work),sizeof(row),failures);
 UnloadImage(atlas);MemFree(rects);UnloadFontData(glyphs,count);UnloadFileData(data);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
'''
(p/'tests/test_results_flow_draw.c').write_text(test,encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text();marker='        add_executable(rr_results_columns_draw_tests ';assert s.count(marker)==1
block='''        add_executable(rr_results_flow_draw_tests tests/test_results_flow_draw.c src/presentation/rr_results_flow_draw.c src/presentation/rr_results_text.c src/presentation/rr_subtext.c)
        target_include_directories(rr_results_flow_draw_tests PRIVATE src/presentation)
        target_link_libraries(rr_results_flow_draw_tests PRIVATE rr_core ${RR_RAYLIB_TARGET})
        target_compile_definitions(rr_results_flow_draw_tests PRIVATE DrawTextCodepoint=rr_record_DrawTextCodepoint DrawTexturePro=rr_record_DrawTexturePro)
        set_property(TARGET rr_results_flow_draw_tests APPEND PROPERTY LINK_DEPENDS ${RR_UI_FONT_FILES})
        if(MSVC)
            target_compile_options(rr_results_flow_draw_tests PRIVATE /W4 /WX)
        else()
            target_compile_options(rr_results_flow_draw_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
        endif()
        if(EMSCRIPTEN)
            target_link_options(rr_results_flow_draw_tests PRIVATE "-sUSE_GLFW=3" "-sALLOW_MEMORY_GROWTH=1" "--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme@/ui-theme")
            add_test(NAME browser_results_flow_native_drawing COMMAND rr_results_flow_draw_tests "/ui-theme/Orbitron-700.ttf")
        else()
            add_test(NAME browser_results_flow_native_drawing COMMAND rr_results_flow_draw_tests "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme/Orbitron-700.ttf")
        endif()
''';s=s.replace(marker,block+marker);s=s.replace('add_executable(RoadstersRace ','add_executable(RoadstersRace src/presentation/rr_results_flow_draw.c ');f.write_text(s)
dest=p/'reference/ui-parity-20261103/results-flow-draw-capture';dest.mkdir(exist_ok=False);(dest/'reference.json').write_text(json.dumps(dict(package=str(pack),cases=cases,glyphs=glyphs,scope='Expected character membership, source viewport selection envelopes and source row classes only. Baseline/ink/shadow approximations remain unqualified; no native pixels.'),indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(boards=len(cases),glyph_commands=len(glyphs),fragments=sum(c['fragments']for c in cases))))

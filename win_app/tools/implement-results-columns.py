from pathlib import Path
import shutil,hashlib,json
p=Path(r'C:\Users\User\Projects\neon_c\win_app')
b=p/'reference/pre-results-columns-20261104';b.mkdir(exist_ok=False)
for f in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 d=b/f;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/f,d)
files={
'src/presentation/rr_results_columns.h':r'''#ifndef RR_RESULTS_COLUMNS_H
#define RR_RESULTS_COLUMNS_H
#include "rr_results_bind.h"
#include "rr_results_card.h"
#include "rr_results_text.h"
typedef enum RRResultsColumnsStatus {RR_RESULTS_COLUMNS_INVALID,RR_RESULTS_COLUMNS_NEEDS_HEIGHT,RR_RESULTS_COLUMNS_NEEDS_WRAP,RR_RESULTS_COLUMNS_READY} RRResultsColumnsStatus;
typedef struct RRResultsColumnsResult {RRResultsColumnsStatus status;RRResultsRowKind kind;size_t row,rows;} RRResultsColumnsResult;
/* List tops are qualified measured border-box tops, not guessed header heights.
 * Source ordered-list padding26 and15px*1.42 line boxes; widths come from the
 * card's actual columns. Preflight BOTH complete lists before any paint command.
 * Reject unsupported wrapping or insufficient intrinsic column height explicitly;
 * do not truncate, shrink, substitute or paint a partial board. Ordinal markers,
 * headings and wrapped HTML inline/float layout remain separate work.
 * scratch is persistent caller storage (one row), no heap or22-row stack array.
 * View/font/card/tops must stay stable through the synchronous paint boundary;
 * sink must not mutate them or scratch. Result.row is meaningful only on failure.
 * This can be used from a bound-draw callback; it does not activate native main. */
RRResultsColumnsResult rr_results_columns_emit(Font font,const RRResultsBoundView *view,
 const RRResultsCardLayout *card,const double list_top[2],int device_scale,
 RRResultsRow *scratch,RRResultsPaintSink sink,void *context);
#endif
''',
'src/presentation/rr_results_columns.c':r'''#include "rr_results_columns.h"
#include "rr_css_font_metrics.h"
#include <float.h>
static bool rect_ready(RRUIRect r){return isfinite(r.x)&&isfinite(r.y)&&isfinite(r.w)&&isfinite(r.h)&&r.w>=26&&r.h>=0;}
RRResultsColumnsResult rr_results_columns_emit(Font font,const RRResultsBoundView *v,const RRResultsCardLayout *card,const double top[2],int scale,RRResultsRow *scratch,RRResultsPaintSink sink,void *ctx)
{
 RRResultsColumnsResult result={RR_RESULTS_COLUMNS_INVALID,RR_RESULTS_STANDINGS,0,0};
 if(!v||!v->results||!card||!top||!scratch||!sink||!v->revision||!v->count||v->count>RR_LOCAL_MAX_CARS||v->player>=v->count||scale<1||scale>3)return result;
 const RRResultsCurrent *s=v->results;
 if(s->coin_count!=v->count||s->time_count!=(v->count>1?v->count:0))return result;
 const size_t counts[2]={s->time_count,s->coin_count};const RRUIRect columns[2]={card->left,card->right};
 const double line=15*1.42;
 for(int k=0;k<2;k++){
  result.kind=(RRResultsRowKind)k;
  if(!rect_ready(columns[k])||!isfinite(top[k])||top[k]<columns[k].y)return result;
  double end=top[k]+line*(double)counts[k];
  if(!isfinite(end)||fabs(end)>FLT_MAX)return result;
  if(end>(double)columns[k].y+columns[k].h){result.status=RR_RESULTS_COLUMNS_NEEDS_HEIGHT;return result;}
 }
 /* Second pass rebuilds only after deterministic complete-board preflight.
  * No provider lookups, core ticks, publication or callback is done by pass0. */
 for(int pass=0;pass<2;pass++)for(int k=0;k<2;k++)for(size_t row=0;row<counts[k];row++){
  result.kind=(RRResultsRowKind)k;result.row=row;
  double left=(double)columns[k].x+26,right=(double)columns[k].x+columns[k].w;
  double baseline=rr_css_line_baseline(top[k]+line*(double)row,line,15,700);
  if(fabs(left)>FLT_MAX||fabs(right)>FLT_MAX||fabs(baseline)>FLT_MAX)return result;
  RRResultsRowStatus status=rr_results_row_build(font,s,(RRResultsRowKind)k,row,v->names,v->count,v->player,scale,(float)left,(float)right,(float)baseline,scratch);
  if(status!=RR_RESULTS_ROW_READY){result.status=status==RR_RESULTS_ROW_NEEDS_WRAP?RR_RESULTS_COLUMNS_NEEDS_WRAP:RR_RESULTS_COLUMNS_INVALID;return result;}
  /* Preflight emitter coordinate validation too, using no externally visible sink. */
  if(pass&& !rr_results_row_emit(scratch,sink,ctx))return result;
  if(pass)result.rows++;
 }
 result.status=RR_RESULTS_COLUMNS_READY;result.kind=RR_RESULTS_STANDINGS;result.row=0;return result;
}
'''}
files['tests/test_results_columns.c']=r'''#include "rr_results_columns.h"
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
'''
# emitter coordinates are finite/bounded by validated boxes; row_emit also checks
# fallback offsets. Use its own validation on pass0 with an internal no-op sink.
files['src/presentation/rr_results_columns.c']=files['src/presentation/rr_results_columns.c'].replace('static bool rect_ready','static void no_paint(void *context,const RRResultsPaint *paint){(void)context;(void)paint;}\nstatic bool rect_ready').replace('if(pass&& !rr_results_row_emit(scratch,sink,ctx))return result;','if(!rr_results_row_emit(scratch,pass?sink:no_paint,pass?ctx:NULL))return result;')
for f,s in files.items():
 assert not (p/f).exists(),f
 (p/f).write_text(s,encoding='utf-8')
cm=p/'CMakeLists.txt';s=cm.read_text();marker='        add_executable(rr_results_rows_tests '
assert s.count(marker)==1
block='''        add_executable(rr_results_columns_tests tests/test_results_columns.c src/presentation/rr_results_columns.c src/presentation/rr_results_text.c src/presentation/rr_subtext.c)
        target_include_directories(rr_results_columns_tests PRIVATE src/presentation)
        target_link_libraries(rr_results_columns_tests PRIVATE rr_core ${RR_RAYLIB_TARGET})
        set_property(TARGET rr_results_columns_tests APPEND PROPERTY LINK_DEPENDS ${RR_UI_FONT_FILES})
        if(MSVC)
            target_compile_options(rr_results_columns_tests PRIVATE /W4 /WX)
        else()
            target_compile_options(rr_results_columns_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
        endif()
        if(EMSCRIPTEN)
            target_link_options(rr_results_columns_tests PRIVATE "-sUSE_GLFW=3" "-sALLOW_MEMORY_GROWTH=1" "--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme@/ui-theme")
            add_test(NAME browser_results_complete_columns_preflight COMMAND rr_results_columns_tests "/ui-theme/Orbitron-700.ttf")
        else()
            add_test(NAME browser_results_complete_columns_preflight COMMAND rr_results_columns_tests "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme/Orbitron-700.ttf")
        endif()
'''
s=s.replace(marker,block+marker).replace('add_executable(RoadstersRace src/presentation/rr_results_text.c','add_executable(RoadstersRace src/presentation/rr_results_columns.c src/presentation/rr_results_text.c');cm.write_text(s)
print('Installed bounded columns preflight; test source required before building. No launcher change.')

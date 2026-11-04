from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');b=p/'reference/pre-results-caption-20261104';b.mkdir(exist_ok=False)
for name in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 d=b/name;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/name,d)
header=r'''#ifndef RR_RESULTS_CAPTION_H
#define RR_RESULTS_CAPTION_H
#include "rr_results_current.h"
#include "rr_results_name_flow.h"
enum {RR_RESULTS_CAPTION_BYTES=64};
typedef struct RRResultsCaption {char text[RR_RESULTS_CAPTION_BYTES];double height;} RRResultsCaption;
/* Canonical res-bank display text and unwrapped Windows-source font profile.
 * Count/rank1..11 only; ordinal/bonus wording matches source expressions. This
 * is DISPLAY METADATA ONLY: no wallet amount, save, reward or unlock operation.
 * Source12px/600 selects verified Orbitron700; missing middle dot uses source
 * Arial Black numeric ascent/descent. Round normal metrics separately, union
 * fallback extents when dot appears:15px plain,17px bonus. Qualified against60
 * existing captures, not arbitrary fonts/captions/HTML or native raster.
 * All66 canonical text bounds fit below258.25; smaller width NEEDS_FLOW rather
 * than guessing wrapping. Input/output disjoint; unchanged out on failure. */
RRResultsNameStatus rr_results_caption_build(const RRResultsCurrent *snapshot,
 double column_width,RRResultsCaption *out);
#endif
'''
source=r'''#include "rr_results_caption.h"
#include "rr_results_summary.h"
#include <math.h>
#include <float.h>
#include <stdio.h>
RRResultsNameStatus rr_results_caption_build(const RRResultsCurrent *s,double width,RRResultsCaption *out)
{
 if(!s||!out||!isfinite(width)||width<=0||width>FLT_MAX||!s->coin_count||s->coin_count>11||s->player_coin_rank<1||(size_t)s->player_coin_rank>s->coin_count)return RR_NAME_INVALID;
 if(width<258.25)return RR_NAME_NEEDS_FLOW;
 RRResultsCaption value={0};char ordinal[RR_RESULTS_ORDINAL_BYTES];if(!rr_results_ordinal(s->player_coin_rank,ordinal))return RR_NAME_INVALID;
 int bonus=s->coin_count>1?(s->player_coin_rank==1?15:s->player_coin_rank==2?8:s->player_coin_rank==3?4:0):0;
 int n=bonus?snprintf(value.text,sizeof(value.text),"COIN RACE %s \302\267 BONUS +%d",ordinal,bonus):snprintf(value.text,sizeof(value.text),"COIN RACE %s",ordinal);if(n<0||(size_t)n>=sizeof(value.text))return RR_NAME_INVALID;
 double ascent=floor(12.0*1011/1000+.5),descent=floor(12.0*243/1000+.5);
 if(bonus){ascent=fmax(ascent,floor(12.0*2254/2048+.5));descent=fmax(descent,floor(12.0*634/2048+.5));}
 value.height=ascent+descent;*out=value;return RR_NAME_READY;
}
'''
draw_header=r'''#ifndef RR_RESULTS_CAPTION_DRAW_H
#define RR_RESULTS_CAPTION_DRAW_H
#include "rr_results_caption.h"
#include "rr_results_tracks_draw.h"
/* Frozen view -> canonical display caption/height + guarded source tracks ->
 * existing complete-board native ROW draw adapter. Caller profile caption and
 * tracks/row pointers/counts are not consumed. Other slot metrics remain inputs.
 * Caption text is returned for future caption painting, not painted here. Out
 * updates only if full native adapter succeeds; no stale output paint on error.
 * Disjoint stable caller storage/lifetimes as existing adapters; no main or
 * reward/account activation, and no cold texture load/graphics creation. */
RRResultsNameStatus rr_ui_results_caption_flow_draw(RRUI *ui,RRResultsArt *art,
 const RRResultsBoundView *view,const RRResultsBodyInput *profile,int device_scale,
 RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row,
 RRResultsCaption *caption);
#endif
'''
draw_source=r'''#include "rr_results_caption_draw.h"
RRResultsNameStatus rr_ui_results_caption_flow_draw(RRUI *ui,RRResultsArt *art,const RRResultsBoundView *view,const RRResultsBodyInput *profile,int scale,RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row,RRResultsCaption *out)
{
 if(!view||!view->results||!profile||!out)return RR_NAME_INVALID;
 const RRResultsCurrent *s=view->results;RRResultsCaption caption;RRResultsNameStatus status=rr_results_caption_build(s,258.25,&caption);if(status!=RR_NAME_READY)return status;
 if(s->coins[s->player_coin_rank-1].racer<0||(size_t)s->coins[s->player_coin_rank-1].racer!=view->player)return RR_NAME_INVALID;
 RRResultsBodyInput input=*profile;input.caption_height=caption.height;const double bounds[2]={258.25,258.25};
 status=rr_ui_results_tracks_draw(ui,art,view,&input,bounds,scale,binding,work,row);if(status==RR_NAME_READY)*out=caption;return status;
}
'''
for f,s in [('src/presentation/rr_results_caption.h',header),('src/presentation/rr_results_caption.c',source),('src/presentation/rr_results_caption_draw.h',draw_header),('src/presentation/rr_results_caption_draw.c',draw_source)]:assert not(p/f).exists();(p/f).write_text(s,encoding='utf-8')
source_cases=json.loads((Path(__file__).parent/'results-caption-source.json').read_text(encoding='utf-8'));assert len(source_cases['cases'])==66
def cs(s):return '"'+''.join(chr(x)if 32<=x<127 and chr(x)not in '\\"'else '\\%03o'%x for x in s.encode('utf-8'))+'"'
fixture='''/* Pure expressions extracted from current browser race.js, no services. */
typedef struct CaptionCase {size_t count;int rank;const char *text;} CaptionCase;
static const CaptionCase caption_cases[]={
'''+''.join('{%d,%d,%s},\n'%(c['count'],c['rank'],cs(c['text']))for c in source_cases['cases'])+'};\n';(p/'tests/results_caption_reference.h').write_text(fixture,encoding='ascii')
test=(p/'tests/test_results_tracks.c').read_text().replace('#include "rr_results_tracks_draw.h"','#include "rr_results_caption_draw.h"\n#include "results_caption_reference.h"')
test=test.replace('static RRResultsTracks tracks,guard,saved_tracks;','static RRResultsCaption caption,guard_caption,saved_caption;static RRResultsTracks tracks,guard,saved_tracks;')
test=test.replace('snapshot.coin_count=b->count;','snapshot.coin_count=b->count;for(size_t j=0;j<b->count;j++)if(b->coins[j].racer==(int)b->player)snapshot.player_coin_rank=(int)j+1;')
test=test.replace('i.left_width=NAN;i.right_width=NAN;','i.left_width=NAN;i.right_width=NAN;i.caption_height=NAN;')
old='CHECK(rr_ui_results_tracks_draw(&ui,&art,&view,&i,bounds,current->scale,&binding,&work,&row)==RR_NAME_READY);';new='CHECK(rr_ui_results_caption_flow_draw(&ui,&art,&view,&i,current->scale,&binding,&work,&row,&caption)==RR_NAME_READY&&caption.height==body_cases[n].caption_height);';assert old in test;test=test.replace(old,new)
start=test.index(' printf("RESULTS_TRACKS_SUMMARY');end=test.index(' UnloadImage',start)
test=test[:start]+r''' for(size_t n=0;n<COUNT(caption_cases);n++){
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
 printf("RESULTS_CAPTION_SUMMARY observed_boards=%zu commands=%zu source_expression_cases=%zu observed_caption_heights=60 caption_bytes=%zu work_bytes=%zu failures=%u measured_caption_height_input=false caption_not_painted=true rewards_inactive=true graphics_contexts=0 main_unchanged=true\n",COUNT(bound_cases),commands,COUNT(caption_cases),sizeof(caption),sizeof(work),failures);
'''+test[end:];(p/'tests/test_results_caption.c').write_text(test,encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text().replace('add_library(rr_core STATIC ','add_library(rr_core STATIC src/presentation/rr_results_caption.c ');marker='        add_executable(rr_results_tracks_tests ';assert s.count(marker)==1
block='''        add_executable(rr_results_caption_tests tests/test_results_caption.c src/presentation/rr_results_caption_draw.c src/presentation/rr_results_tracks_draw.c src/presentation/rr_results_bound_flow.c src/presentation/rr_results_flow_draw.c src/presentation/rr_results_text.c src/presentation/rr_subtext.c)
        target_include_directories(rr_results_caption_tests PRIVATE src/presentation)
        target_link_libraries(rr_results_caption_tests PRIVATE rr_core ${RR_RAYLIB_TARGET})
        target_compile_definitions(rr_results_caption_tests PRIVATE DrawTextCodepoint=rr_record_DrawTextCodepoint DrawTexturePro=rr_record_DrawTexturePro)
        set_property(TARGET rr_results_caption_tests APPEND PROPERTY LINK_DEPENDS ${RR_UI_FONT_FILES})
        if(MSVC)
            target_compile_options(rr_results_caption_tests PRIVATE /W4 /WX)
        else()
            target_compile_options(rr_results_caption_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
        endif()
        if(EMSCRIPTEN)
            target_link_options(rr_results_caption_tests PRIVATE "-sUSE_GLFW=3" "-sALLOW_MEMORY_GROWTH=1" "--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme@/ui-theme")
            add_test(NAME browser_results_canonical_caption_height COMMAND rr_results_caption_tests "/ui-theme/Orbitron-700.ttf")
        else()
            add_test(NAME browser_results_canonical_caption_height COMMAND rr_results_caption_tests "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme/Orbitron-700.ttf")
        endif()
''';s=s.replace(marker,block+marker).replace('add_executable(RoadstersRace ','add_executable(RoadstersRace src/presentation/rr_results_caption_draw.c ');f.write_text(s)
dest=p/'reference/ui-parity-20261103/results-caption-capture';dest.mkdir(exist_ok=False);shutil.copy2(Path(__file__).parent/'results-caption-source.json',dest/'source-expressions.json')
(dest/'scope.json').write_text(json.dumps(dict(measured='60 existing browser caption border-box heights; native profile caption height poisoned to NaN.',derived='All66 canonical caption strings from actual pure source expressions, height extension from qualified primary/fallback normal line metrics and proven unwrapped bounds. Not66 new browser captures.',display_only='Caption text/height returned for future painting. No save/wallet/unlock/reward/account operations; main and caption painting remain inactive.'),indent=2)+'\n',encoding='utf-8')
print('Canonical source caption formatter/height adapter and CPU fixtures staged.')

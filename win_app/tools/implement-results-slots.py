from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');b=p/'reference/pre-results-slots-20261104';b.mkdir(exist_ok=False)
for name in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 d=b/name;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/name,d)
header=r'''#ifndef RR_RESULTS_SLOTS_H
#define RR_RESULTS_SLOTS_H
#include "rr_results_card.h"
#include "rr_results_current.h"
#include "rr_results_name_flow.h"
/* Explicit CAPTURE EMPTY-ACCOUNT profile, not inferred account status: online
 * visible/empty; unlock/next hidden. Exactly2 laps0..3600s each, count1..11,
 * valid frozen finish/coin ranks and takedowns0..9. Other states NEEDS_FLOW or
 * INVALID, never truncate/wrap/fabricate account text. Caller must select this
 * profile knowingly; not a substitute for live account/service integration.
 * Fixed source CSS/font metrics derive FINISH/place/total/laps/button heights.
 * COLUMNS=0 placeholder is overwritten by body solver. Output unchanged on
 * failure; no text painting, backend, reward, main or graphics activation. */
RRResultsNameStatus rr_results_empty_slots(const RRResultsCurrent *snapshot,
 RRResultsCardContent *out);
#endif
'''
source=r'''#include "rr_results_slots.h"
#include "rr_results_summary.h"
#include "rr_results_times.h"
#include <math.h>
static double normal(double size){return floor(size*1011/1000+.5)+floor(size*243/1000+.5);}
RRResultsNameStatus rr_results_empty_slots(const RRResultsCurrent *s,RRResultsCardContent *out)
{
 if(!s||!out)return RR_NAME_INVALID;
 RRResultsSummary summary;if(!rr_results_summary_build(s,&summary))return RR_NAME_INVALID;
 RRResultsTimes times;if(!rr_results_times_current(s,&times))return RR_NAME_INVALID;
 if(s->lap_count!=2||s->player_takedowns>9||s->laps[0]>3600||s->laps[1]>3600)return RR_NAME_NEEDS_FLOW;
 RRResultsCardContent value={{0},RR_CARD_REQUIRED};
 value.heights[RR_CARD_BRAND]=floor(34*.95*64)/64;
 value.heights[RR_CARD_PLACE]=s->coin_count>1?fmax(floor(24*1011.0/1000+.5),floor(24*2254.0/2048+.5))+fmax(floor(24*243.0/1000+.5),floor(24*634.0/2048+.5)):0;
 value.heights[RR_CARD_TOTAL]=normal(32);value.heights[RR_CARD_LAPS]=normal(16);
 value.heights[RR_CARD_AGAIN]=normal(20)+32+4;value.heights[RR_CARD_MENU]=value.heights[RR_CARD_AGAIN];
 *out=value;return RR_NAME_READY;
}
'''
draw_header=r'''#ifndef RR_RESULTS_SLOTS_DRAW_H
#define RR_RESULTS_SLOTS_DRAW_H
#include "rr_results_slots.h"
#include "rr_results_caption_draw.h"
/* Explicit empty-account source candidate only. Frozen view + viewport/scroll
 * derive slot/caption/track/body metrics and existing native ROW commands.
 * No measured card/slot/caption/track dimensions are supplied. Same disjoint,
 * stable storage/lifetime and failure contracts as caption adapter. Account
 * profile is not auto-detected. Does not paint header/caption/card decoration,
 * activate main, load cold resources or create a graphics/audio context. */
RRResultsNameStatus rr_ui_results_empty_profile_draw(RRUI *ui,RRResultsArt *art,
 const RRResultsBoundView *view,double width,double height,double scroll,int scale,
 RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row,
 RRResultsCaption *caption);
#endif
'''
draw_source=r'''#include "rr_results_slots_draw.h"
RRResultsNameStatus rr_ui_results_empty_profile_draw(RRUI *ui,RRResultsArt *art,const RRResultsBoundView *view,double width,double height,double scroll,int scale,RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row,RRResultsCaption *caption)
{
 if(!view||!view->results)return RR_NAME_INVALID;RRResultsCardContent slots;
 RRResultsNameStatus status=rr_results_empty_slots(view->results,&slots);if(status!=RR_NAME_READY)return status;
 RRResultsBodyInput profile={width,height,scroll,NAN,NAN,NAN,NULL,NULL,0,0,slots};
 return rr_ui_results_caption_flow_draw(ui,art,view,&profile,scale,binding,work,row,caption);
}
'''
for f,s in [('src/presentation/rr_results_slots.h',header),('src/presentation/rr_results_slots.c',source),('src/presentation/rr_results_slots_draw.h',draw_header),('src/presentation/rr_results_slots_draw.c',draw_source)]:assert not(p/f).exists();(p/f).write_text(s,encoding='utf-8')
test=(p/'tests/test_results_caption.c').read_text().replace('#include "rr_results_caption_draw.h"','#include "rr_results_slots_draw.h"');test=test.replace('static RRResultsCaption caption,guard_caption,saved_caption;','static RRResultsCardContent slots,guard_slots,saved_slots;static RRResultsBodyLayout body;static RRResultsCaption caption,guard_caption,saved_caption;')
test=test.replace('snapshot=(RRResultsCurrent){0};','snapshot=(RRResultsCurrent){0};snapshot.finish_position=1;snapshot.player_takedowns=0;snapshot.lap_count=2;snapshot.laps[0]=62.5;snapshot.laps[1]=61.25;')
old='CHECK(rr_ui_results_caption_flow_draw(&ui,&art,&view,&i,current->scale,&binding,&work,&row,&caption)==RR_NAME_READY&&caption.height==body_cases[n].caption_height);'
new='''CHECK(rr_results_empty_slots(&snapshot,&slots)==RR_NAME_READY);for(int slot=0;slot<RR_CARD_SLOTS;slot++)CHECK(slots.heights[slot]==body_cases[n].other.heights[slot]);CHECK(slots.visible==body_cases[n].other.visible);
  CHECK(rr_ui_results_empty_profile_draw(&ui,&art,&view,i.width,i.height,i.scroll,current->scale,&binding,&work,&row,&caption)==RR_NAME_READY&&caption.height==body_cases[n].caption_height);
  RRResultsBodyInput generated={i.width,i.height,i.scroll,tracks.width[0],tracks.width[1],caption.height,binding.rows[0],binding.rows[1],binding.counts[0],binding.counts[1],slots};CHECK(rr_results_body_flow(&generated,&work,&body)==RR_NAME_READY);
  CHECK(body.card.panel.x==body_cases[n].panel.x&&body.card.panel.y==body_cases[n].panel.y&&body.card.panel.w==body_cases[n].panel.w&&body.card.panel.h==body_cases[n].panel.h);''';assert old in test;test=test.replace(old,new)
start=test.index(' printf("RESULTS_CAPTION_SUMMARY');end=test.index(' UnloadImage',start)
test=test[:start]+r''' memset(&guard_slots,0x5a,sizeof(guard_slots));saved_slots=guard_slots;
 snapshot.player_takedowns=10;CHECK(rr_results_empty_slots(&snapshot,&guard_slots)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard_slots,&saved_slots,sizeof(guard_slots)));snapshot.player_takedowns=0;
 snapshot.lap_count=1;CHECK(rr_results_empty_slots(&snapshot,&guard_slots)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard_slots,&saved_slots,sizeof(guard_slots)));snapshot.lap_count=2;
 snapshot.laps[0]=3600.01;CHECK(rr_results_empty_slots(&snapshot,&guard_slots)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard_slots,&saved_slots,sizeof(guard_slots)));snapshot.laps[0]=62.5;
 snapshot.laps[0]=NAN;CHECK(rr_results_empty_slots(&snapshot,&guard_slots)==RR_NAME_INVALID&&!memcmp(&guard_slots,&saved_slots,sizeof(guard_slots)));snapshot.laps[0]=62.5;
 CHECK(rr_results_empty_slots(NULL,&guard_slots)==RR_NAME_INVALID&&rr_results_empty_slots(&snapshot,NULL)==RR_NAME_INVALID);
 CHECK(rr_ui_results_empty_profile_draw(&ui,&art,NULL,1600,1000,0,1,&binding,&work,&row,&guard_caption)==RR_NAME_INVALID&&commands==before);
 printf("RESULTS_SLOTS_SUMMARY observed_cards=%zu commands=%zu generated_slot_heights=600 slots_bytes=%zu work_bytes=%zu failures=%u explicit_empty_account_profile=true measured_slot_inputs=false graphics_contexts=0 main_unchanged=true\n",COUNT(bound_cases),commands,sizeof(slots),sizeof(work),failures);
'''+test[end:];(p/'tests/test_results_slots.c').write_text(test,encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text().replace('add_library(rr_core STATIC ','add_library(rr_core STATIC src/presentation/rr_results_slots.c ');marker='        add_executable(rr_results_caption_tests ';assert s.count(marker)==1
block='''        add_executable(rr_results_slots_tests tests/test_results_slots.c src/presentation/rr_results_slots_draw.c src/presentation/rr_results_caption_draw.c src/presentation/rr_results_tracks_draw.c src/presentation/rr_results_bound_flow.c src/presentation/rr_results_flow_draw.c src/presentation/rr_results_text.c src/presentation/rr_subtext.c)
        target_include_directories(rr_results_slots_tests PRIVATE src/presentation)
        target_link_libraries(rr_results_slots_tests PRIVATE rr_core ${RR_RAYLIB_TARGET})
        target_compile_definitions(rr_results_slots_tests PRIVATE DrawTextCodepoint=rr_record_DrawTextCodepoint DrawTexturePro=rr_record_DrawTexturePro)
        set_property(TARGET rr_results_slots_tests APPEND PROPERTY LINK_DEPENDS ${RR_UI_FONT_FILES})
        if(MSVC)
            target_compile_options(rr_results_slots_tests PRIVATE /W4 /WX)
        else()
            target_compile_options(rr_results_slots_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
        endif()
        if(EMSCRIPTEN)
            target_link_options(rr_results_slots_tests PRIVATE "-sUSE_GLFW=3" "-sALLOW_MEMORY_GROWTH=1" "--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme@/ui-theme")
            add_test(NAME browser_results_generated_empty_account_slots COMMAND rr_results_slots_tests "/ui-theme/Orbitron-700.ttf")
        else()
            add_test(NAME browser_results_generated_empty_account_slots COMMAND rr_results_slots_tests "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme/Orbitron-700.ttf")
        endif()
''';s=s.replace(marker,block+marker).replace('add_executable(RoadstersRace ','add_executable(RoadstersRace src/presentation/rr_results_slots_draw.c ');f.write_text(s)
dest=p/'reference/ui-parity-20261103/results-slots-capture';dest.mkdir(exist_ok=False);shutil.copy2(Path(__file__).parent/'results-slots-proof.json',dest/'fit-proof.json')
pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1');obs=json.loads((pack/'observations.json').read_text(encoding='utf-8'));assert all('lapTimes'not in o['input']and not o['input'].get('finishPos',0)for o in obs)
(dest/'source-profile.json').write_text(json.dumps(dict(package=str(pack),harness='tools/prepare-results-wrapping.mjs',default_laps=[62.5,61.25],default_finish_position=1,takedowns=0,account='Harness clears online block and hides unlock/next. Explicit fixture profile only, never inferred live account status.',slot_expected='Existing source card profiles/boxes and full panel geometry. Columns placeholder generated independently by body flow, not consumed.'),indent=2)+'\n',encoding='utf-8')
(dest/'caption-glyph-request.json').write_text(json.dumps(dict(codepoint=183,css_size=12,css_weight=600,primary='Orbitron700 selected by source stack',fallback='Arial Black / Arial-Black',device_scales=[1,2,3],variant='white core for source #c9b27a tint, no caption shadow',required=['Lossless isolated crop with source baseline x/y bearings, advance and draw dimensions','Platform-font resolution, source/installed-font hashes and source CSS/font provenance','CPU decode/checksum and crop roundtrip checks, native raster still unqualified'],existing='v1 dot masks are26CSSpx only; arbitrary downsampling is unqualified.',constraints='No game/WebGL/native graphics, paid generation or proprietary font binary redistribution; separate asset-team capture via supported workflow if authorized.'),indent=2)+'\n',encoding='utf-8')
print('Generated empty-account source slot profile and exact caption-glyph request staged.')

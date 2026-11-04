from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');b=p/'reference/pre-results-tracks-20261104';b.mkdir(exist_ok=False)
for name in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 d=b/name;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/name,d)
header=r'''#ifndef RR_RESULTS_TRACKS_H
#define RR_RESULTS_TRACKS_H
#include "rr_results_list_flow.h"
typedef struct RRResultsTracks {double minimum[2],width[2],overflow;} RRResultsTracks;
/* Fixed source616px grid /22px gap /1.3fr 1fr. Min-content from qualified body
 * name Latin/digit words, Latin-letter hyphens, source crown prefix and numeric
 * clock/coin/approx values, with26px list indent. Font/whitespace/GPOS rejects
 * follow character API. This is a BOUNDED source profile, not generic CSS/UAX.
 * Other content requires finite nonnegative qualified min-content UPPER bounds.
 * They must be nonbinding in the computed tracks; else NEEDS_FLOW without
 * guessing their actual min-content. Current fixed headings/canonical captions
 * have separately verified258.25px bounds, not arbitrary account/profile text.
 * Inputs/work/out disjoint/stable; scratch may change, output only on success.
 * No heap, layout pixels/draw or source measured track widths as inputs. */
RRResultsNameStatus rr_results_tracks(const RRResultsFlowText *standings,size_t standings_count,
 const RRResultsFlowText *coins,size_t coin_count,const double other_bounds[2],
 RRResultsCharacters *scratch,RRResultsTracks *out);
#endif
'''
source=r'''#include "rr_results_tracks.h"
#include <math.h>
static bool latin(uint32_t c){return(c>=65&&c<=90)||(c>=97&&c<=122);}
static double pen(const RRResultsCharacters *s,size_t n){return n==s->count?s->advance:s->ranges[n].pen;}
static RRResultsNameStatus minimum(const char *text,bool value,RRResultsCharacters *s,double *out)
{
 RRResultsAdvanceStatus status=rr_results_characters(text,s);if(status==RR_ADVANCE_INVALID)return RR_NAME_INVALID;if(status!=RR_ADVANCE_READY||!s->count)return RR_NAME_NEEDS_FLOW;
 size_t start=0;double widest=0;
 for(size_t j=0;j<s->count;j++){
  uint32_t cp=s->ranges[j].codepoint;bool digit=cp>=48&&cp<=57;
  if(value){if(!(digit||cp==32||cp==58||cp==46||cp==8776))return RR_NAME_NEEDS_FLOW;if(cp==8776&&(j||s->count<3||s->ranges[1].codepoint!=32))return RR_NAME_NEEDS_FLOW;}
  else{if(!(digit||latin(cp)||cp==32||cp==45||cp==9819))return RR_NAME_NEEDS_FLOW;if(cp==9819&&(j||s->count<3||s->ranges[1].codepoint!=32))return RR_NAME_NEEDS_FLOW;}
  if(cp==45&&(!j||j+1==s->count||!latin(s->ranges[j-1].codepoint)||!latin(s->ranges[j+1].codepoint)))return RR_NAME_NEEDS_FLOW;
  if(cp==32||cp==45){size_t end=cp==45?j+1:j;widest=fmax(widest,pen(s,end)-pen(s,start));start=j+1;}
 }
 widest=fmax(widest,s->advance-pen(s,start));*out=ceil(widest*64)/64;return RR_NAME_READY;
}
RRResultsNameStatus rr_results_tracks(const RRResultsFlowText *standings,size_t ns,const RRResultsFlowText *coins,size_t nc,const double other[2],RRResultsCharacters *scratch,RRResultsTracks *out)
{
 if(!scratch||!out||!other||ns>11||nc>11||(ns&&!standings)||(nc&&!coins)||!isfinite(other[0])||!isfinite(other[1])||other[0]<0||other[1]<0)return RR_NAME_INVALID;
 if(!nc)return RR_NAME_NEEDS_FLOW;
 RRResultsTracks result={0};const RRResultsFlowText *lists[2]={standings,coins};const size_t counts[2]={ns,nc};
 for(int k=0;k<2;k++)for(size_t j=0;j<counts[k];j++){
  double name,value;RRResultsNameStatus status=minimum(lists[k][j].name,false,scratch,&name);if(status!=RR_NAME_READY)return status;
  status=minimum(lists[k][j].value,true,scratch,&value);if(status!=RR_NAME_READY)return status;
  result.minimum[k]=fmax(result.minimum[k],26+fmax(name,value));
 }
 double left=594*1.3/2.3,right=594/2.3;
 if(result.minimum[0]+result.minimum[1]>594){left=result.minimum[0];right=result.minimum[1];}
 else if(result.minimum[0]>left){left=result.minimum[0];right=594-left;}
 else if(result.minimum[1]>right){right=result.minimum[1];left=594-right;}
 result.width[0]=floor(left*64)/64;result.width[1]=floor(right*64)/64;
 if(other[0]>result.width[0]||other[1]>result.width[1])return RR_NAME_NEEDS_FLOW;
 result.overflow=fmax(0,result.width[0]+22+result.width[1]-616);*out=result;return RR_NAME_READY;
}
'''
draw_header=r'''#ifndef RR_RESULTS_TRACKS_DRAW_H
#define RR_RESULTS_TRACKS_DRAW_H
#include "rr_results_bound_flow.h"
#include "rr_results_tracks.h"
/* Frozen bound view -> copied strings/styles -> computed bounded source tracks
 * -> existing complete native body/fragment preflight. Profile track values and
 * row pointers/counts are not consumed. Other min-content bounds, caption height
 * and other slot metrics remain explicit qualified inputs. No main activation.
 * Same disjoint/stable caller storage/lifetime contract as bound-flow adapter. */
RRResultsNameStatus rr_ui_results_tracks_draw(RRUI *ui,RRResultsArt *art,
 const RRResultsBoundView *view,const RRResultsBodyInput *profile,const double other_bounds[2],
 int device_scale,RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row);
#endif
'''
draw_source=r'''#include "rr_results_tracks_draw.h"
RRResultsNameStatus rr_ui_results_tracks_draw(RRUI *ui,RRResultsArt *art,const RRResultsBoundView *view,const RRResultsBodyInput *profile,const double other[2],int scale,RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row)
{
 if(!profile||!work||!rr_results_flow_prepare(view,binding))return RR_NAME_INVALID;
 RRResultsTracks tracks;RRResultsNameStatus status=rr_results_tracks(binding->rows[0],binding->counts[0],binding->rows[1],binding->counts[1],other,&work->name_characters,&tracks);if(status!=RR_NAME_READY)return status;
 RRResultsBodyInput input=*profile;input.left_width=tracks.width[0];input.right_width=tracks.width[1];input.standings=binding->rows[0];input.coins=binding->rows[1];input.standings_count=binding->counts[0];input.coin_count=binding->counts[1];
 return rr_ui_results_flow_draw(ui,art,&input,binding->player[0],binding->player[1],scale,work,row);
}
'''
for f,s in [('src/presentation/rr_results_tracks.h',header),('src/presentation/rr_results_tracks.c',source),('src/presentation/rr_results_tracks_draw.h',draw_header),('src/presentation/rr_results_tracks_draw.c',draw_source)]:assert not(p/f).exists();(p/f).write_text(s,encoding='utf-8')
test=(p/'tests/test_results_bound_flow.c').read_text().replace('#include "rr_results_bound_flow.h"','#include "rr_results_tracks_draw.h"');test=test.replace('static RRResultsFlowBinding binding,saved_binding;','static RRResultsTracks tracks,guard,saved_tracks;static const double bounds[2]={258.25,258.25};static RRResultsFlowBinding binding,saved_binding;')
old='RRResultsBodyInput i=input(body_cases+n);CHECK(rr_ui_results_bound_flow_draw(&ui,&art,&view,&i,current->scale,&binding,&work,&row)==RR_NAME_READY);'
new='''RRResultsBodyInput i=input(body_cases+n);i.left_width=NAN;i.right_width=NAN;
  CHECK(rr_results_flow_prepare(&view,&binding));CHECK(rr_results_tracks(binding.rows[0],binding.counts[0],binding.rows[1],binding.counts[1],bounds,&work.name_characters,&tracks)==RR_NAME_READY&&tracks.width[0]==body_cases[n].left&&tracks.width[1]==body_cases[n].right);
  CHECK(rr_ui_results_tracks_draw(&ui,&art,&view,&i,bounds,current->scale,&binding,&work,&row)==RR_NAME_READY);''';assert old in test;test=test.replace(old,new)
test=test.replace('rr_ui_results_bound_flow_draw(&ui,&art,&view,&i,1,','rr_ui_results_tracks_draw(&ui,&art,&view,&i,bounds,1,').replace('rr_ui_results_bound_flow_draw(&ui,&art,&view,NULL,1,','rr_ui_results_tracks_draw(&ui,&art,&view,NULL,bounds,1,')
marker=' printf("RESULTS_BOUND_FLOW_SUMMARY';start=test.index(marker);end=test.index(' UnloadImage',start)
test=test[:start]+r''' memset(&guard,0x5a,sizeof(guard));saved_tracks=guard;
 double blocking[2]={999,258.25};CHECK(rr_results_tracks(binding.rows[0],binding.counts[0],binding.rows[1],binding.counts[1],blocking,&work.name_characters,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved_tracks,sizeof(guard)));
 RRResultsFlowText unknown[]={ {"Pilot","0"} };CHECK(rr_results_tracks(NULL,0,unknown,1,bounds,&work.name_characters,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved_tracks,sizeof(guard)));
 unknown[0].name="ACE/ACE";CHECK(rr_results_tracks(NULL,0,unknown,1,bounds,&work.name_characters,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved_tracks,sizeof(guard)));
 unknown[0].name="ACE-1";CHECK(rr_results_tracks(NULL,0,unknown,1,bounds,&work.name_characters,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved_tracks,sizeof(guard)));
 CHECK(rr_results_tracks(NULL,0,NULL,0,bounds,&work.name_characters,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved_tracks,sizeof(guard)));
 blocking[0]=NAN;CHECK(rr_results_tracks(binding.rows[0],binding.counts[0],binding.rows[1],binding.counts[1],blocking,&work.name_characters,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved_tracks,sizeof(guard)));
 CHECK(rr_ui_results_tracks_draw(&ui,&art,&view,&i,bounds,1,&binding,NULL,&row)==RR_NAME_INVALID&&commands==before);
 printf("RESULTS_TRACKS_SUMMARY observed_boards=%zu commands=%zu computed_tracks=120 tracks_bytes=%zu work_bytes=%zu failures=%u measured_track_inputs=false other_bounds_nonbinding=true graphics_contexts=0 main_unchanged=true\n",COUNT(bound_cases),commands,sizeof(tracks),sizeof(work),failures);
'''+test[end:];(p/'tests/test_results_tracks.c').write_text(test,encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text().replace('add_library(rr_core STATIC ','add_library(rr_core STATIC src/presentation/rr_results_tracks.c ');marker='        add_executable(rr_results_bound_flow_tests ';assert s.count(marker)==1
block='''        add_executable(rr_results_tracks_tests tests/test_results_tracks.c src/presentation/rr_results_tracks_draw.c src/presentation/rr_results_bound_flow.c src/presentation/rr_results_flow_draw.c src/presentation/rr_results_text.c src/presentation/rr_subtext.c)
        target_include_directories(rr_results_tracks_tests PRIVATE src/presentation)
        target_link_libraries(rr_results_tracks_tests PRIVATE rr_core ${RR_RAYLIB_TARGET})
        target_compile_definitions(rr_results_tracks_tests PRIVATE DrawTextCodepoint=rr_record_DrawTextCodepoint DrawTexturePro=rr_record_DrawTexturePro)
        set_property(TARGET rr_results_tracks_tests APPEND PROPERTY LINK_DEPENDS ${RR_UI_FONT_FILES})
        if(MSVC)
            target_compile_options(rr_results_tracks_tests PRIVATE /W4 /WX)
        else()
            target_compile_options(rr_results_tracks_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
        endif()
        if(EMSCRIPTEN)
            target_link_options(rr_results_tracks_tests PRIVATE "-sUSE_GLFW=3" "-sALLOW_MEMORY_GROWTH=1" "--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme@/ui-theme")
            add_test(NAME browser_results_computed_source_tracks COMMAND rr_results_tracks_tests "/ui-theme/Orbitron-700.ttf")
        else()
            add_test(NAME browser_results_computed_source_tracks COMMAND rr_results_tracks_tests "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme/Orbitron-700.ttf")
        endif()
''';s=s.replace(marker,block+marker).replace('add_executable(RoadstersRace ','add_executable(RoadstersRace src/presentation/rr_results_tracks_draw.c ');f.write_text(s)
dest=p/'reference/ui-parity-20261103/results-tracks-capture';dest.mkdir(exist_ok=False)
for src,name in [('results-tracks-analysis.json','analysis.json'),('results-other-mins-proof.json','other-mins-proof.json')]:shutil.copy2(Path(__file__).parent/src,dest/name)
(dest/'scope.json').write_text(json.dumps(dict(expected_tracks='Independent source golden column widths, reused frozen source snapshot/command expectations.',input_profiles='No source-measured track widths. Qualified other min-content upper bounds258.25, caption height, other slots and viewport remain inputs. Canonical other bounds from numeric font proof; arbitrary captions/account content not inferred.',not_claimed='Generic CSS/UAX min-content, fresh browser/native pixels, caption heights or main activation.'),indent=2)+'\n',encoding='utf-8')
print('Computed-track solver/native frozen-view adapter staged with source fixtures.')

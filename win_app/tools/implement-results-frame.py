from pathlib import Path
import shutil,json
p=Path(r'C:\Users\User\Projects\neon_c\win_app');b=p/'reference/pre-results-frame-20261104';b.mkdir(exist_ok=False)
for name in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 d=b/name;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/name,d)
header=r'''#ifndef RR_RESULTS_EMPTY_FRAME_H
#define RR_RESULTS_EMPTY_FRAME_H
#include "rr_results_slots_draw.h"
/* Explicit EMPTY-ACCOUNT source candidate, not inferred live service state.
 * Zero once per application, persistent address, never shallow-copy or reset on
 * race/menu changes: queued callbacks and internal owner/string pointers live
 * across transitions. No heap/cold resource ownership in this state. */
typedef struct RRResultsEmptyFrame {
 RRResultsTimer timer;RRResultsBinding identity;RRResultsFlowBinding flow;
 RRResultsListWork work;RRResultsRow row;RRResultsCaption caption;
 RRResultsNameStatus draw_status;bool draw_attempted,caption_ready;
} RRResultsEmptyFrame;
/* Call exactly once AFTER a completed current tick/UI transition with monotonic
 * wall seconds. A tick is submitted once; NULL for UI-only frames. Delegate
 * source queue/hide/pump/frozen identity capture order to existing bind_frame.
 * draw_attempted disambiguates draw_status on hidden/bind-error frames. Caption
 * is exposed only when caption_ready; stale stored bytes are not a paint request.
 * Failure preserves existing published numeric/identity data/queue per their
 * contracts; caller must inspect/report, never replay simulation/account work.
 * Inputs, state, font/texture metadata remain disjoint/stable synchronously.
 * ROWS only: headers/caption/card not painted; main/full-pack/rewards inactive.
 * Does not load textures/open audio/create graphics or change actual run phase. */
bool rr_results_empty_frame(RRResultsEmptyFrame *state,const RRRunCurrent *run,
 const RRRunEvents *tick,double wall_seconds,bool hide,RRResultsNameLookup names,
 void *names_context,RRUI *ui,RRResultsArt *art,double width,double height,
 double scroll,int device_scale,RRResultsPump *events);
#endif
'''
source=r'''#include "rr_results_empty_frame.h"
typedef struct EmptyFrameDraw {RRResultsEmptyFrame *state;RRUI *ui;RRResultsArt *art;double width,height,scroll;int scale;} EmptyFrameDraw;
static bool draw(void *context,const RRResultsBoundView *view)
{
 EmptyFrameDraw *f=context;RRResultsEmptyFrame *s=f->state;s->draw_attempted=true;
 s->draw_status=rr_ui_results_empty_profile_draw(f->ui,f->art,view,f->width,f->height,f->scroll,f->scale,&s->flow,&s->work,&s->row,&s->caption);
 s->caption_ready=s->draw_status==RR_NAME_READY;return s->caption_ready;
}
bool rr_results_empty_frame(RRResultsEmptyFrame *s,const RRRunCurrent *run,const RRRunEvents *tick,double now,bool hide,RRResultsNameLookup names,void *names_context,RRUI *ui,RRResultsArt *art,double width,double height,double scroll,int scale,RRResultsPump *events)
{
 if(!s){if(events)*events=(RRResultsPump){0};return false;}
 s->draw_attempted=false;s->caption_ready=false;s->draw_status=RR_NAME_INVALID;
 EmptyFrameDraw context={s,ui,art,width,height,scroll,scale};
 return rr_results_bind_frame(&s->identity,&s->timer,run,tick,now,hide,names,names_context,draw,&context,events);
}
'''
for f,s in [('src/presentation/rr_results_empty_frame.h',header),('src/presentation/rr_results_empty_frame.c',source)]:assert not(p/f).exists();(p/f).write_text(s,encoding='utf-8')
test=r'''#include "rr_results_empty_frame.h"
#include "rr_results_fallback_glyphs.h"
#include "rr_ui_codepoints.h"
#include "results_bind_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,lookups,commands,frames,visible_frames,publications,positive_lookups;
static RRResultsEmptyFrame state;static RRRunCurrent run;static RRBattleCurrent graph;static RRLocalBattle battle;static RRLocalPack pack;static RRLocalRacer actors[11];static RRTrack track;static double laps[2];static RRUI ui;static RRResultsArt art;
static const BindAction *action;static char shared[512],expected[512];static int fail_index=-1,epoch;static double surface_width=1600,surface_height=1000;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<15)fprintf(stderr,"empty frame line %d: %s\n",__LINE__,#x);failures++;}}while(0)
/* Supported synthetic identity variant of existing source lifecycle fixtures.
 * Remove ONLY test-name angle brackets; original source fixture stays untouched.
 * Schedule/numeric/order/name-freezing expectations remain the source oracle.
 * This is NOT a new browser/native pixel measurement of these variant labels. */
static void supported_name(const char *in,char *out){while(*in){if(*in!='<'&&*in!='>')*out++=*in;in++;}*out=0;}
static const char *lookup(void *context,size_t index,const RRLocalRacer *r){CHECK(context==&run&&index<11&&r==battle.roster[index]);lookups++;if((int)index==fail_index)return NULL;if(action)supported_name(action->live_names[index],shared);else snprintf(shared,sizeof(shared),"N%d D%zu",epoch,index);return shared;}
void rr_record_DrawTextCodepoint(Font f,int cp,Vector2 origin,float size,Color color){CHECK(f.texture.id==ui.bold.texture.id&&f.glyphs==ui.bold.glyphs&&cp>=32&&isfinite(origin.x)&&isfinite(origin.y)&&size>0&&color.a>0);commands++;}
void rr_record_DrawTexturePro(Texture2D t,Rectangle source,Rectangle dest,Vector2 origin,float rotation,Color color){bool found=false;for(int i=0;i<RR_RESULTS_ART_COUNT;i++)if(t.id==art.glyphs[i].id)found=true;CHECK(found&&source.width>0&&source.height>0&&isfinite(dest.x)&&isfinite(dest.y)&&dest.width>0&&dest.height>0&&origin.x==0&&origin.y==0&&rotation==0&&color.a>0);commands++;}
static void setup(void){
 memset(&state,0,sizeof(state));memset(&run,0,sizeof(run));memset(&graph,0,sizeof(graph));memset(&battle,0,sizeof(battle));memset(&pack,0,sizeof(pack));memset(actors,0,sizeof(actors));memset(&track,0,sizeof(track));
 run.current=&graph;run.host.battle=&battle;graph.battle=&battle;graph.pack.pack=&pack;battle.pack=&pack;pack.player=battle.fire.player=actors;pack.track=&track;track.sample_count=4096;pack.finish_position=1;
 for(int i=0;i<11;i++){battle.roster[i]=actors+i;actors[i].race.mode=RR_RACE_TWO_LAPS;actors[i].race.lap=i?1:3;actors[i].race.next_checkpoint=i?1:0;actors[i].race.finished=i==0;actors[i].ai.sample=i*173;actors[i].coins=i%4;}
 actors[0].lap_times=laps;actors[0].lap_count=actors[0].lap_capacity=2;run.host.phase=RR_SESSION_FINISHED;run.host.race_time=80;actors[0].race.finish_time=80;laps[0]=32;laps[1]=48;
 action=NULL;fail_index=-1;surface_width=1600;surface_height=1000;
}
static bool frame(double now,bool finish,bool hide,RRResultsNameLookup provider,RRResultsPump *events){
 RRRunEvents tick={0};tick.player=RR_RACE_FINISH;double before=run.host.race_time;RRSessionPhase phase=run.host.phase;int coins=actors[0].coins;
 bool result=rr_results_empty_frame(&state,&run,finish?&tick:NULL,now,hide,provider,&run,&ui,&art,surface_width,surface_height,0,1,events);
 CHECK(run.host.race_time==before&&run.host.phase==phase&&actors[0].coins==coins);frames++;return result;
}
static void positive(void){
 for(size_t n=0;n<COUNT(bind_cases);n++){
  setup();
  for(size_t j=0;j<bind_cases[n].count;j++){
   action=bind_cases[n].actions+j;run.host.phase=(RRSessionPhase)action->phase;run.host.race_time=action->time;actors[0].race.finish_time=action->time;actors[0].coins=action->coins;laps[0]=action->time*.4;laps[1]=action->time*.6;
   unsigned before=commands,names_before=lookups;RRResultsPump e;CHECK(frame(action->now/1000,action->kind==0,action->kind==2,lookup,&e));const BindExpected *x=&action->expected;
   CHECK(e.callbacks==x->callbacks&&e.published==x->published&&state.timer.pending==x->pending&&state.timer.revision==x->revision&&state.timer.visible==x->visible);
   CHECK(lookups-names_before==(x->published?11u:0u)&&state.caption_ready==x->visible&&state.draw_attempted==x->visible);publications+=e.published;
   for(size_t k=0;k<x->pending;k++)CHECK(state.timer.due[k]==x->due[k]);
   if(x->visible){
    CHECK(state.draw_status==RR_NAME_READY&&commands>before&&state.identity.status==RR_RESULTS_BIND_READY);visible_frames++;
    CHECK(state.flow.counts[0]==x->time_count&&state.flow.counts[1]==x->coin_count);
    for(int k=0;k<2;k++)for(size_t row=0;row<state.flow.counts[k];row++){
     const BindRowText *text=k?x->coins+row:x->times+row;supported_name(text->name,expected);CHECK(!strcmp(state.flow.rows[k][row].name,expected)&&!strcmp(state.flow.rows[k][row].value,text->value)&&state.flow.player[k][row]==text->me);
    }
   }else CHECK(commands==before&&state.identity.status==RR_RESULTS_BIND_IDLE);
   if(x->cached){CHECK(state.identity.ready);for(size_t i=0;i<11;i++){supported_name(x->names[i],expected);CHECK(!strcmp(state.identity.buffers[state.identity.active].names[i],expected));}}
  }
 }
 positive_lookups=lookups;
}
static void negative(void){RRResultsPump e;
 setup();epoch=1;CHECK(frame(0,true,false,lookup,&e));art.ready=false;unsigned before=commands;CHECK(!frame(2,false,false,lookup,&e)&&e.published==1&&state.timer.visible&&state.identity.ready&&state.draw_attempted&&!state.caption_ready&&commands==before&&state.identity.status==RR_RESULTS_BIND_DRAW_ERROR);
 unsigned names=lookups;RRResultsCurrent frozen=state.timer.snapshot;art.ready=true;epoch=2;CHECK(frame(2.1,false,false,NULL,&e)&&lookups==names&&state.caption_ready&&state.draw_status==RR_NAME_READY&&!memcmp(&frozen,&state.timer.snapshot,sizeof(frozen)));CHECK(!strcmp(state.flow.labels[0][0].name,"N1 D0"));
 run.host.phase=RR_SESSION_MENU;before=commands;CHECK(frame(2.2,false,true,NULL,&e)&&!state.caption_ready&&!state.draw_attempted&&commands==before);
 setup();epoch=3;actors[0].takedowns=10;CHECK(frame(0,true,false,lookup,&e));before=commands;CHECK(!frame(2,false,false,lookup,&e)&&state.draw_status==RR_NAME_NEEDS_FLOW&&!state.caption_ready&&commands==before);names=lookups;actors[0].takedowns=0;CHECK(!frame(2.1,false,false,NULL,&e)&&state.draw_status==RR_NAME_NEEDS_FLOW&&lookups==names&&commands==before);CHECK(!frame(3,true,false,NULL,&e)&&state.timer.pending==1);CHECK(frame(5,false,false,lookup,&e)&&state.caption_ready&&state.timer.snapshot.player_takedowns==0);
 setup();epoch=4;CHECK(frame(0,true,false,lookup,&e));fail_index=5;before=commands;CHECK(!frame(2,false,false,lookup,&e)&&!state.draw_attempted&&!state.caption_ready&&commands==before&&state.identity.status==RR_RESULTS_BIND_IDENTITY_ERROR);names=lookups;fail_index=-1;CHECK(!frame(2.1,false,false,lookup,&e)&&lookups==names&&commands==before);CHECK(!frame(3,true,false,lookup,&e));CHECK(frame(5,false,false,lookup,&e)&&state.caption_ready);
 before=commands;CHECK(!frame(4,false,false,NULL,&e)&&state.timer.error&&!state.caption_ready&&!state.draw_attempted&&commands==before);
 CHECK(!rr_results_empty_frame(NULL,&run,NULL,6,false,NULL,NULL,&ui,&art,1600,1000,0,1,&e)&&!e.callbacks&&!e.published);
}
int main(int argc,char **argv){
 CHECK(argc==2);if(argc!=2)return 1;SetTraceLogLevel(LOG_WARNING);int bytes=0,count=0,cps[RR_UI_MAX_CODEPOINTS];unsigned char *data=LoadFileData(argv[1],&bytes);CHECK(data);if(!data)return 1;
 int requested=rr_ui_codepoints(cps,700);GlyphInfo *glyphs=LoadFontData(data,bytes,64,cps,requested,FONT_DEFAULT,&count);CHECK(glyphs&&count==96);if(!glyphs){UnloadFileData(data);return 1;}Rectangle *rects=NULL;Image atlas=GenImageFontAtlas(glyphs,&rects,count,64,4,0);CHECK(atlas.data&&rects);if(!rects){UnloadFontData(glyphs,count);UnloadFileData(data);return 1;}
 ui.loaded=true;ui.bold=(Font){64,count,0,{1,0,0,0,0},rects,glyphs};art.ready=true;for(int i=0;i<RR_RESULTS_ART_COUNT;i++){art.glyphs[i].id=(unsigned)i+100;art.glyphs[i].width=rr_results_fallback_glyphs[i].width_pixels;art.glyphs[i].height=rr_results_fallback_glyphs[i].height_pixels;}
 positive();negative();
 printf("RESULTS_EMPTY_FRAME_SUMMARY source_cases=%zu frames=%u visible_source_frames=%u source_publications=%u positive_identity_lookups=%u commands=%u state_bytes=%zu failures=%u source_schedule_with_supported_variant_names=true no_simulation_replay=true graphics_contexts=0 main_unchanged=true\n",COUNT(bind_cases),frames,visible_frames,publications,positive_lookups,commands,sizeof(state),failures);
 UnloadImage(atlas);MemFree(rects);UnloadFontData(glyphs,count);UnloadFileData(data);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
''';(p/'tests/test_results_empty_frame.c').write_text(test,encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text();marker='        add_executable(rr_results_slots_tests ';assert s.count(marker)==1
block='''        add_executable(rr_results_empty_frame_tests tests/test_results_empty_frame.c src/presentation/rr_results_empty_frame.c src/presentation/rr_results_slots_draw.c src/presentation/rr_results_caption_draw.c src/presentation/rr_results_tracks_draw.c src/presentation/rr_results_bound_flow.c src/presentation/rr_results_flow_draw.c src/presentation/rr_results_text.c src/presentation/rr_subtext.c)
        target_include_directories(rr_results_empty_frame_tests PRIVATE src/presentation)
        target_link_libraries(rr_results_empty_frame_tests PRIVATE rr_core ${RR_RAYLIB_TARGET})
        target_compile_definitions(rr_results_empty_frame_tests PRIVATE DrawTextCodepoint=rr_record_DrawTextCodepoint DrawTexturePro=rr_record_DrawTexturePro)
        set_property(TARGET rr_results_empty_frame_tests APPEND PROPERTY LINK_DEPENDS ${RR_UI_FONT_FILES})
        if(MSVC)
            target_compile_options(rr_results_empty_frame_tests PRIVATE /W4 /WX)
        else()
            target_compile_options(rr_results_empty_frame_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
        endif()
        if(EMSCRIPTEN)
            target_link_options(rr_results_empty_frame_tests PRIVATE "-sUSE_GLFW=3" "-sALLOW_MEMORY_GROWTH=1" "--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme@/ui-theme")
            add_test(NAME browser_results_empty_frame_native_lifecycle COMMAND rr_results_empty_frame_tests "/ui-theme/Orbitron-700.ttf")
        else()
            add_test(NAME browser_results_empty_frame_native_lifecycle COMMAND rr_results_empty_frame_tests "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme/Orbitron-700.ttf")
        endif()
''';s=s.replace(marker,block+marker).replace('add_executable(RoadstersRace ','add_executable(RoadstersRace src/presentation/rr_results_empty_frame.c ');f.write_text(s)
dest=p/'reference/ui-parity-20261103/results-frame-capture';dest.mkdir(exist_ok=False);(dest/'scope.json').write_text(json.dumps(dict(source_oracle='Existing results_bind_reference.h schedules/numeric ordering/freezing expectations; file untouched.',variant_names='Only angle brackets removed from test labels to fit already qualified break profile. This is a derived integration fixture, not exact source-name/pixel acceptance.',negative='Cold-resource metadata error/repaint recovery without identity recapture; unsupported frozen slot profile remains frozen despite live changes; failed identity revision not retried; new publication recovery; hide/clock errors.',production='Explicit empty-account row-frame candidate only; no main/full-pack/rewards/playback activation or game/graphics launch.'),indent=2)+'\n',encoding='utf-8')
print('Explicit empty-account results frame adapter/lifecycle tests staged.')

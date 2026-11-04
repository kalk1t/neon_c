from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');b=p/'reference/pre-current-host-frame-20261104';b.mkdir(exist_ok=False)
for name in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 d=b/name;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/name,d)
header=r'''#ifndef RR_CURRENT_EMPTY_HOST_H
#define RR_CURRENT_EMPTY_HOST_H
#include "rr_run_input.h"
#include "rr_results_empty_frame.h"
typedef struct RRCurrentEmptyRequest {
 RRRunInputFrame input;RRMouseSettings mouse;double dt,wall_seconds,width,height,scroll;
 int device_scale;bool simulate,hide;
} RRCurrentEmptyRequest;
typedef struct RRCurrentEmptyEvents {
 RRRunEvents tick;RRResultsPump results;
 bool tick_attempted,tick_ok,presentation_attempted,presentation_ok;
} RRCurrentEmptyEvents;
/* Candidate application boundary, explicit EMPTY-ACCOUNT ROW profile only.
 * simulate=true calls run_input_tick exactly once then submits its completed
 * event to results frame; simulate=false is UI-only (no input/aim/clock/RNG tick).
 * Simulation failure skips presentation and clears exposed per-frame draw flags.
 * A successful tick followed by paint
 * failure returns false BUT tick_ok remains true: do not replay that tick. Use
 * a later UI-only request to retry the frozen presentation instead. Existing
 * partial-failure/queue/clock latches remain authoritative, no rollback fiction.
 * Caller prepares/owns graph, normalized input, stable frame storage and loaded
 * resource metadata; source phase/transitions/reset remain caller responsibilities.
 * Borrowed const world/segment semantics preserved; NULL world means explicit
 * free space. No account/audio/graphics creation or production main activation. */
bool rr_current_empty_host(RRResultsEmptyFrame *results,RRRunInputState *input_state,
 RRRunCurrent *run,const RRCurrentEmptyRequest *request,const struct RRWorld *world,
 RRShotWorld segment,void *segment_context,RRResultsNameLookup names,void *names_context,
 RRUI *ui,RRResultsArt *art,RRCurrentEmptyEvents *events);
#endif
'''
source=r'''#include "rr_current_empty_host.h"
bool rr_current_empty_host(RRResultsEmptyFrame *results,RRRunInputState *inputs,RRRunCurrent *run,const RRCurrentEmptyRequest *request,const struct RRWorld *world,RRShotWorld segment,void *segment_context,RRResultsNameLookup names,void *names_context,RRUI *ui,RRResultsArt *art,RRCurrentEmptyEvents *events)
{
 if(!events)return false;*events=(RRCurrentEmptyEvents){0};if(!results||!inputs||!run||!request)return false;
 results->draw_attempted=false;results->caption_ready=false;results->draw_status=RR_NAME_INVALID;
 if(request->simulate){events->tick_attempted=true;events->tick_ok=rr_run_input_tick(inputs,run,request->input,request->mouse,request->dt,world,segment,segment_context,&events->tick);if(!events->tick_ok)return false;}
 events->presentation_attempted=true;
 events->presentation_ok=rr_results_empty_frame(results,run,request->simulate?&events->tick:NULL,request->wall_seconds,request->hide,names,names_context,ui,art,request->width,request->height,request->scroll,request->device_scale,&events->results);
 return events->presentation_ok;
}
'''
for f,s in [('src/presentation/rr_current_empty_host.h',header),('src/presentation/rr_current_empty_host.c',source)]:assert not(p/f).exists();(p/f).write_text(s,encoding='utf-8')
test=r'''#include "rr_current_empty_host.h"
static bool coupled_tick(RRRunInputState *,RRRunCurrent *,RRRunInputFrame,RRMouseSettings,double,const struct RRWorld *,RRShotWorld,void *,RRRunEvents *);
#define rr_run_input_tick coupled_tick
#define main original_source_input_main
#include "test_run_input.c"
#undef main
#undef rr_run_input_tick
static RRResultsEmptyFrame presentation;static RRUI ui;static RRResultsArt art;
static double wall_offset;static unsigned host_calls,ui_retries,paint_failures,lookups,finish_queues,ui_due_flushes,commands,invalid_tick_probes;static int last_trace=-1;static char name_buffer[32];
static const char *name_lookup(void *context,size_t index,const RRLocalRacer *r){RRRunCurrent *host=context;CHECK(index<11&&r==host->host.battle->roster[index]);lookups++;snprintf(name_buffer,sizeof(name_buffer),"D%zu",index);return name_buffer;}
void rr_record_DrawTextCodepoint(Font f,int cp,Vector2 origin,float size,Color color){(void)f;(void)cp;(void)origin;(void)size;(void)color;commands++;CHECK(false);}
void rr_record_DrawTexturePro(Texture2D t,Rectangle source,Rectangle dest,Vector2 origin,float rotation,Color color){(void)t;(void)source;(void)dest;(void)origin;(void)rotation;(void)color;commands++;CHECK(false);}
static bool coupled_tick(RRRunInputState *inputs_ptr,RRRunCurrent *host,RRRunInputFrame input,RRMouseSettings settings,double raw,const struct RRWorld *world_ptr,RRShotWorld segment_fn,void *segment_context,RRRunEvents *tick)
{
 bool source_frame=current<COUNT(frame_fixtures);const FrameFixture *f=source_frame?binding_frame:NULL;
 if(f&&(int)f->trace!=last_trace){memset(&presentation,0,sizeof(presentation));last_trace=(int)f->trace;}
 RRCurrentEmptyRequest request={input,settings,raw,(double)host_calls*.05+wall_offset,1600,1000,0,1,true,f&&f->restart};RRCurrentEmptyEvents e;host_calls++;
 bool ok=rr_current_empty_host(&presentation,inputs_ptr,host,&request,world_ptr,segment_fn,segment_context,name_lookup,host,&ui,&art,&e);
 CHECK(e.tick_attempted&&e.tick_ok&&e.presentation_attempted&&ok==e.presentation_ok);*tick=e.tick;
 bool presented=e.presentation_ok;RRCurrentEmptyRequest display_request=request;
 if(e.tick.player&RR_RACE_FINISH){
  finish_queues++;RRRunCurrent saved=*host;RRCar before=host->host.battle->fire.player->car;RRRunInputState saved_inputs=*inputs_ptr;Random *rng=host->host.battle->fire.random_ctx;unsigned draws_before=rng->draws;
  /* Explicit virtual UI-only boundary after source2s delay; no simulation time
   * is inserted into or removed from the unchanged source numerical oracle. */
  wall_offset+=2.001;display_request.wall_seconds+=2.001;display_request.simulate=false;display_request.hide=false;RRCurrentEmptyEvents due;
  CHECK(!rr_current_empty_host(&presentation,inputs_ptr,host,&display_request,world_ptr,segment_fn,segment_context,name_lookup,host,&ui,&art,&due));
  CHECK(!due.tick_attempted&&due.presentation_attempted&&!due.presentation_ok&&due.results.callbacks>0&&due.results.published>0);
  CHECK(!memcmp(&saved,host,sizeof(saved))&&!memcmp(&before,&host->host.battle->fire.player->car,sizeof(before))&&!memcmp(&saved_inputs,inputs_ptr,sizeof(saved_inputs))&&rng->draws==draws_before);presented=due.presentation_ok;ui_due_flushes++;
 }
 /* UI resources are intentionally unavailable: this test qualifies simulation
  * completion/queue/error ordering, NOT actual row drawing or cold resources. */
 if(!presented){
  paint_failures++;CHECK(presentation.timer.visible&&!presentation.caption_ready&&presentation.identity.ready);
  RRCar before=host->host.battle->fire.player->car;RRRunCurrent saved=*host;RRRunInputState saved_inputs=*inputs_ptr;
  Random *rng=host->host.battle->fire.random_ctx;unsigned draws_before=rng->draws,names_before=lookups;size_t pending=presentation.timer.pending,revision=presentation.timer.revision;
  RRCurrentEmptyRequest retry=display_request;retry.simulate=false;retry.hide=false;retry.dt=NAN;retry.input=(RRRunInputFrame){0};RRCurrentEmptyEvents again;
  CHECK(!rr_current_empty_host(&presentation,inputs_ptr,host,&retry,world_ptr,segment_fn,segment_context,NULL,NULL,&ui,&art,&again));
  CHECK(!again.tick_attempted&&!again.tick_ok&&again.presentation_attempted&&!again.presentation_ok&&!again.tick.player);
  CHECK(!memcmp(&before,&host->host.battle->fire.player->car,sizeof(before))&&!memcmp(&saved,host,sizeof(saved))&&!memcmp(&saved_inputs,inputs_ptr,sizeof(saved_inputs))&&rng->draws==draws_before&&lookups==names_before&&presentation.timer.pending==pending&&presentation.timer.revision==revision);ui_retries++;
 }
 if(!invalid_tick_probes){
  RRRunCurrent saved=*host;RRCar before=host->host.battle->fire.player->car;RRRunInputState saved_inputs=*inputs_ptr;Random *rng=host->host.battle->fire.random_ctx;unsigned draws_before=rng->draws;size_t pending=presentation.timer.pending;
  RRCurrentEmptyRequest bad=request;bad.mouse.sensitivity=NAN;bad.dt=.02;bad.hide=false;RRCurrentEmptyEvents failed;presentation.caption_ready=true;presentation.draw_attempted=true;
  CHECK(!rr_current_empty_host(&presentation,inputs_ptr,host,&bad,world_ptr,segment_fn,segment_context,NULL,NULL,&ui,&art,&failed)&&failed.tick_attempted&&!failed.tick_ok&&!failed.presentation_attempted);
  CHECK(!memcmp(&saved,host,sizeof(saved))&&!memcmp(&before,&host->host.battle->fire.player->car,sizeof(before))&&!memcmp(&saved_inputs,inputs_ptr,sizeof(saved_inputs))&&rng->draws==draws_before&&presentation.timer.pending==pending&&!presentation.caption_ready&&!presentation.draw_attempted);invalid_tick_probes++;
 }
 /* Preserve the source simulation oracle's tick return contract while separately
  * asserting the new application's combined presentation-failure result above. */
 return e.tick_ok;
}
int main(void){int original=original_source_input_main();CHECK(host_calls>0&&finish_queues>0&&ui_due_flushes==finish_queues&&paint_failures>0&&ui_retries==paint_failures&&lookups>0&&!commands&&invalid_tick_probes==1);RRCurrentEmptyEvents events;
 CHECK(!rr_current_empty_host(NULL,&inputs,&run,NULL,NULL,NULL,NULL,NULL,NULL,&ui,&art,&events)&&!events.tick_attempted&&!events.presentation_attempted);
 printf("CURRENT_EMPTY_HOST_SUMMARY source_input_frames=%zu host_calls=%u finish_events=%u explicit_due_ui_frames=%u presentation_failures=%u ui_only_retries=%u identity_lookups=%u invalid_tick_probes=%u event_bytes=%zu failures=%u source_simulation_oracle_reused=true resources_intentionally_unavailable=true graphics_contexts=0 commands=%u main_unchanged=true\n",COUNT(frame_fixtures),host_calls,finish_queues,ui_due_flushes,paint_failures,ui_retries,lookups,invalid_tick_probes,sizeof(events),failures,commands);return original||failures?EXIT_FAILURE:EXIT_SUCCESS;
}
''';(p/'tests/test_current_empty_host.c').write_text(test,encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text();marker='        add_executable(rr_results_empty_frame_tests ';assert s.count(marker)==1
block='''        add_executable(rr_current_empty_host_tests tests/test_current_empty_host.c src/presentation/rr_current_empty_host.c src/presentation/rr_results_empty_frame.c src/presentation/rr_results_slots_draw.c src/presentation/rr_results_caption_draw.c src/presentation/rr_results_tracks_draw.c src/presentation/rr_results_bound_flow.c src/presentation/rr_results_flow_draw.c src/presentation/rr_results_text.c src/presentation/rr_subtext.c)
        target_include_directories(rr_current_empty_host_tests PRIVATE src/presentation)
        target_link_libraries(rr_current_empty_host_tests PRIVATE rr_core ${RR_RAYLIB_TARGET})
        target_compile_definitions(rr_current_empty_host_tests PRIVATE DrawTextCodepoint=rr_record_DrawTextCodepoint DrawTexturePro=rr_record_DrawTexturePro)
        if(MSVC)
            target_compile_options(rr_current_empty_host_tests PRIVATE /W4 /WX)
        else()
            target_compile_options(rr_current_empty_host_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
        endif()
        if(EMSCRIPTEN)
            target_link_options(rr_current_empty_host_tests PRIVATE "-sUSE_GLFW=3" "-sALLOW_MEMORY_GROWTH=1")
        endif()
        add_test(NAME browser_current_input_results_host_boundary COMMAND rr_current_empty_host_tests)
''';s=s.replace(marker,block+marker).replace('add_executable(RoadstersRace ','add_executable(RoadstersRace src/presentation/rr_current_empty_host.c ');f.write_text(s)
dest=p/'reference/ui-parity-20261103/current-host-frame-capture';dest.mkdir(exist_ok=False);(dest/'scope.json').write_text(json.dumps(dict(source_oracle='Original test_run_input.c + run_input_reference.h included unchanged, normalized input/player/full-pack/world/RNG/events checks retained.',added='Actual input tick then completed finish event to wall-timer/results frame; deliberate unavailable UI resources; UI-only retry must not tick/change car/aim/clock/RNG/names/pending publication; invalid simulation skips presentation.',not_qualified='This failure-path integration test does not paint rows, qualify native resources/graphics or alter production main/full-pack switches. Existing positive row/lifecycle tests remain separate.',ownership='Borrowed prepared current graph/input state and persistent address-stable results frame. Phase/reset/application lifetime remains caller-owned.'),indent=2)+'\n',encoding='utf-8')
print('Input tick -> results host boundary and unchanged source simulation oracle test staged.')

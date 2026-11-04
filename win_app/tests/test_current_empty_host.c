#include "rr_current_empty_host.h"
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

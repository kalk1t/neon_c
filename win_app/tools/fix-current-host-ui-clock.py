from pathlib import Path
import ast,json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');cap=p/'reference/ui-parity-20261103/current-host-frame-capture'
for target in ['native','web']:
 for src,dst in [(p/f'reference/ui-parity-20261103/current-host-frame-{target}-build.log',cap/f'initial-{target}-build.log'),(p/f'build/current-host-frame-{target}/Testing/Temporary/LastTest.log',cap/f'initial-{target}-LastTest.log')]:assert not dst.exists();shutil.copy2(src,dst)
shutil.copy2(p/'tests/test_current_empty_host.c',cap/'initial-test_current_empty_host.c')
script=Path(__file__).parent/'implement-current-host-frame.py';s=script.read_text();replacements=[
 ('static unsigned host_calls,ui_retries,paint_failures,lookups,finish_queues,commands,invalid_tick_probes;','static double wall_offset;static unsigned host_calls,ui_retries,paint_failures,lookups,finish_queues,ui_due_flushes,commands,invalid_tick_probes;'),
 ('(double)host_calls*.05,1600','(double)host_calls*.05+wall_offset,1600'),
 (' if(e.tick.player&RR_RACE_FINISH)finish_queues++;',r''' bool presented=e.presentation_ok;RRCurrentEmptyRequest display_request=request;
 if(e.tick.player&RR_RACE_FINISH){
  finish_queues++;RRRunCurrent saved=*host;RRCar before=host->host.battle->fire.player->car;RRRunInputState saved_inputs=*inputs_ptr;Random *rng=host->host.battle->fire.random_ctx;unsigned draws_before=rng->draws;
  /* Explicit virtual UI-only boundary after source2s delay; no simulation time
   * is inserted into or removed from the unchanged source numerical oracle. */
  wall_offset+=2.001;display_request.wall_seconds+=2.001;display_request.simulate=false;display_request.hide=false;RRCurrentEmptyEvents due;
  CHECK(!rr_current_empty_host(&presentation,inputs_ptr,host,&display_request,world_ptr,segment_fn,segment_context,name_lookup,host,&ui,&art,&due));
  CHECK(!due.tick_attempted&&due.presentation_attempted&&!due.presentation_ok&&due.results.callbacks>0&&due.results.published>0);
  CHECK(!memcmp(&saved,host,sizeof(saved))&&!memcmp(&before,&host->host.battle->fire.player->car,sizeof(before))&&!memcmp(&saved_inputs,inputs_ptr,sizeof(saved_inputs))&&rng->draws==draws_before);presented=due.presentation_ok;ui_due_flushes++;
 }'''),
 (' if(!e.presentation_ok){',' if(!presented){'),
 ('RRCurrentEmptyRequest retry=request;','RRCurrentEmptyRequest retry=display_request;'),
 ('finish_queues>0&&paint_failures>0','finish_queues>0&&ui_due_flushes==finish_queues&&paint_failures>0'),
 ('finish_events=%u presentation_failures=','finish_events=%u explicit_due_ui_frames=%u presentation_failures='),
 ('host_calls,finish_queues,paint_failures','host_calls,finish_queues,ui_due_flushes,paint_failures')]
for before,after in replacements:assert s.count(before)==1,before;s=s.replace(before,after)
script.write_text(s,encoding='utf-8');tree=ast.parse(s);test=next(ast.literal_eval(n.value)for n in tree.body if isinstance(n,ast.Assign)and len(n.targets)==1 and isinstance(n.targets[0],ast.Name)and n.targets[0].id=='test');(p/'tests/test_current_empty_host.c').write_text(test,encoding='utf-8')
(cap/'ui-clock-correction.json').write_text(json.dumps(dict(initial_native='138/139 passed; original1440-frame simulation oracle passed; integration coverage assertion failed because no results callbacks became due.',initial_web='Same138/139 result; zero presentation failures/lookups confirmed the virtual UI window never reached a due callback.',correction='Add an explicit virtual UI-only frame2.001s after each finish; offset future UI clock to stay monotonic. Assert car/run/input/RNG unchanged during delayed publication, then retry without replay. Core tick sequence/source fixtures/runtime boundary unchanged.',preserved='Both initial logs/LastTest and original integration test source retained.'),indent=2)+'\n',encoding='utf-8')
print('Preserved initial evidence; added explicit due UI-only boundaries without changing simulation or runtime adapter.')

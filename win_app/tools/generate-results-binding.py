from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');d=json.loads((p/'reference/ui-parity-20261103/results-binding-capture/reference.json').read_text(encoding='utf-8'))
def q(s):return '"'+''.join(chr(b) if 32<=b<127 and b not in [34,92] else '\\"' if b==34 else '\\\\' if b==92 else '\\%03o'%b for b in s.encode('utf-8'))+'"'
def n(v):return ('true' if v else 'false') if isinstance(v,bool) else repr(v)
def row(v):return '{'+q(v['name'])+','+q(v['value'])+','+n(v['me'])+'}'
phases={'menu':0,'race':1,'paused':2,'countdown':3,'finished':4,'results':5}
lines=['''typedef struct BindRowText {const char *name,*value;bool me;} BindRowText;
typedef struct BindExpected {unsigned callbacks,published;size_t revision;bool visible,cached;size_t pending;double due[16],total;const char *total_text,*names[11];size_t time_count,coin_count;BindRowText times[11],coins[11];} BindExpected;
typedef struct BindAction {int kind;double now;int phase;double time;int coins;const char *live_names[11];BindExpected expected;} BindAction;
static const struct BindCase {size_t count;BindAction actions[32];} bind_cases[]={''']
for c in d['cases']:
 lines.append('{%d,{'%len(c['actions']))
 for a in c['actions']:
  e=a['expected'];b=e['board'];exp=','.join([n(e['callbacks']),n(e['published']),n(e['revision']),n(e['visible']),n(b is not None),n(len(e['pending'])),'{'+(','.join(n(v) for v in e['pending']) or '0')+'}',n(b['total'] if b else 0),q(b['total_text'] if b else ''),'{'+(','.join(q(v) for v in b['names']) if b else 'NULL')+'}',n(len(b['times']) if b else 0),n(len(b['coins']) if b else 0),'{'+(','.join(row(v) for v in b['times']) if b else '{NULL,NULL,false}')+'}','{'+(','.join(row(v) for v in b['coins']) if b else '{NULL,NULL,false}')+'}'])
  lines.append('{'+','.join([n(a['kind']),n(a['now']),n(phases[a['phase']]),n(a['time']),n(a['coins']),'{'+','.join(q(v) for v in a['live_names'])+'}','{'+exp+'}'])+'},')
 lines.append('}},')
lines.append('};');(p/'tests/results_bind_reference.h').write_text('\n'.join(lines)+'\n',encoding='utf-8')
(p/'tests/test_results_bind.c').write_text('''#include "rr_results_bind.h"
#include "rr_results_text.h"
#include "rr_results_times.h"
#include "rr_ui_codepoints.h"
#include "rr_css_font_metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "results_bind_reference.h"
static unsigned failures,lookups,draws,actions,publications,capture_boundaries,paint_rows,paint_commands;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"bind line %d: %s\\n",__LINE__,#x);failures++;}}while(0)
static RRResultsBinding bound;static RRResultsTimer timer;static RRRunCurrent run;static RRBattleCurrent graph;static RRLocalBattle battle;static RRLocalPack pack;static RRLocalRacer actors[11];static RRTrack track;static double laps[2];
static const BindAction *action;static size_t action_index;static Font font;static RRResultsRow scratch;
static int fail_index=-1,long_index=-1,epoch;static bool draw_ok=true;static char shared[512],too_long[513];
static char name_text[512],value_text[64];static size_t name_bytes,value_bytes,core_tokens;
static void setup(void){
 memset(&run,0,sizeof(run));memset(&graph,0,sizeof(graph));memset(&battle,0,sizeof(battle));memset(&pack,0,sizeof(pack));memset(actors,0,sizeof(actors));memset(&track,0,sizeof(track));
 run.current=&graph;run.host.battle=&battle;graph.battle=&battle;graph.pack.pack=&pack;battle.pack=&pack;pack.player=battle.fire.player=actors;pack.track=&track;track.sample_count=4096;pack.finish_position=1;
 for(int i=0;i<11;i++){battle.roster[i]=actors+i;actors[i].race.mode=RR_RACE_TWO_LAPS;actors[i].race.lap=i?1:3;actors[i].race.next_checkpoint=i?1:0;actors[i].race.finished=i==0;actors[i].ai.sample=i*173;actors[i].coins=i%4;}
 actors[0].lap_times=laps;actors[0].lap_count=actors[0].lap_capacity=2;
 run.host.phase=RR_SESSION_FINISHED;run.host.race_time=80;actors[0].race.finish_time=80;laps[0]=32;laps[1]=48;
}
static const char *lookup(void *context,size_t index,const RRLocalRacer *r){
 CHECK(context==&run&&index<11&&r==run.host.battle->roster[index]);lookups++;
 if((int)index==fail_index)return NULL;if((int)index==long_index)return too_long;
 if(action)snprintf(shared,sizeof(shared),"%s",action->live_names[index]);else snprintf(shared,sizeof(shared),"N%d D%zu",epoch,index);
 return shared;
}
static int encode(char out[5],uint32_t cp){int n=0;if(cp<128)out[n++]=(char)cp;else if(cp<2048){out[n++]=(char)(192|(cp>>6));out[n++]=(char)(128|(cp&63));}else{out[n++]=(char)(224|(cp>>12));out[n++]=(char)(128|((cp>>6)&63));out[n++]=(char)(128|(cp&63));}out[n]=0;return n;}
static void record(void *context,const RRResultsPaint *p){
 CHECK(context==&scratch);paint_commands++;if(p->shadow)return;char s[5];int n=encode(s,p->codepoint);
 if(core_tokens++<scratch.name.count){CHECK(name_bytes+(size_t)n<sizeof(name_text));if(name_bytes+(size_t)n<sizeof(name_text)){memcpy(name_text+name_bytes,s,(size_t)n);name_bytes+=(size_t)n;}}
 else{CHECK(value_bytes+(size_t)n<sizeof(value_text));if(value_bytes+(size_t)n<sizeof(value_text)){memcpy(value_text+value_bytes,s,(size_t)n);value_bytes+=(size_t)n;}}
}
static bool draw(void *context,const RRResultsBoundView *view){
 CHECK(context==&bound&&view&&view->count==11&&view->player==0&&view->revision>0);draws++;if(!view||!draw_ok)return false;
 if(!action)return true;const BindExpected *e=&action->expected;CHECK(e->cached&&view->revision==e->revision&&fabs(view->results->total-e->total)<1e-7);
 for(size_t i=0;i<11;i++)CHECK(!strcmp(view->names[i],e->names[i]));
 RRResultsTimes timing;CHECK(rr_results_times_current(view->results,&timing)&&!strcmp(timing.total,e->total_text));
 for(int kind=0;kind<2;kind++){
  size_t count=kind==RR_RESULTS_COINS?e->coin_count:e->time_count;CHECK((kind==RR_RESULTS_COINS?view->results->coin_count:view->results->time_count)==count);
  for(size_t i=0;i<count;i++){
   /* Retained source column tracks minus26px list padding. These row origins
    * are supplied content geometry, not variable-card/scroll qualification. */
   float left=32+(kind==RR_RESULTS_COINS?335.734375f+22:0)+26;
   float right=32+(kind==RR_RESULTS_COINS?335.734375f+22+258.25f:335.734375f);
   double top=185.296875+19+(kind==RR_RESULTS_COINS?6:0)+(double)i*21.296875;
   float baseline=(float)rr_css_line_baseline(top,21.296875,15,700);
   RRResultsRowStatus status=rr_results_row_build(font,view->results,(RRResultsRowKind)kind,i,view->names,view->count,view->player,(int)(action_index%3)+1,left,right,baseline,&scratch);
   CHECK(status==RR_RESULTS_ROW_READY);if(status!=RR_RESULTS_ROW_READY)return false;
   name_bytes=value_bytes=core_tokens=0;CHECK(rr_results_row_emit(&scratch,record,&scratch));name_text[name_bytes]=0;value_text[value_bytes]=0;
   const BindRowText *expected=kind==RR_RESULTS_COINS?e->coins+i:e->times+i;CHECK(!strcmp(name_text,expected->name)&&!strcmp(value_text,expected->value));paint_rows++;
  }
 }
 return true;
}
static void positive(void){
 for(size_t c=0;c<sizeof(bind_cases)/sizeof(bind_cases[0]);c++){
  setup();memset(&timer,0,sizeof(timer));memset(&bound,0,sizeof(bound));
  for(size_t j=0;j<bind_cases[c].count;j++){
   action=bind_cases[c].actions+j;action_index=j;run.host.phase=(RRSessionPhase)action->phase;run.host.race_time=action->time;actors[0].race.finish_time=action->time;actors[0].coins=action->coins;laps[0]=action->time*.4;laps[1]=action->time*.6;
   RRRunEvents finish={0};finish.player=RR_RACE_FINISH;RRResultsPump event;unsigned before_names=lookups,before_draws=draws;
   CHECK(rr_results_bind_frame(&bound,&timer,&run,action->kind==0?&finish:NULL,action->now/1000,action->kind==2,lookup,&run,draw,&bound,&event));
   const BindExpected *e=&action->expected;CHECK(event.callbacks==e->callbacks&&event.published==e->published&&timer.revision==e->revision&&timer.visible==e->visible&&timer.pending==e->pending);
   for(size_t k=0;k<timer.pending&&k<e->pending;k++)CHECK(fabs(timer.due[k]-e->due[k])<1e-6);
   CHECK(lookups-before_names==(e->published?11u:0u)&&draws-before_draws==(e->visible?1u:0u));
   CHECK(bound.ready==e->cached&&bound.attempted_revision==e->revision&&bound.status==(e->visible?RR_RESULTS_BIND_READY:RR_RESULTS_BIND_IDLE));
   if(e->cached){const RRResultsIdentities *ids=bound.buffers+bound.active;CHECK(ids->count==11&&ids->revision==e->revision);for(size_t k=0;k<11;k++)CHECK(!strcmp(ids->names[k],e->names[k]));}
   RRResultsBoundView view;CHECK(rr_results_bind_view(&bound,&view)==e->visible);actions++;publications+=event.published;capture_boundaries+=e->published>0;
  }
 }
 action=NULL;
}
static bool frame(double now,bool finish,bool hide,RRResultsNameLookup provider,RRResultsBoundDraw consumer,RRResultsPump *e){RRRunEvents tick={0};tick.player=RR_RACE_FINISH;return rr_results_bind_frame(&bound,&timer,&run,finish?&tick:NULL,now,hide,provider,&run,consumer,&bound,e);}
static void reset(void){setup();memset(&timer,0,sizeof(timer));memset(&bound,0,sizeof(bound));fail_index=long_index=-1;draw_ok=true;action=NULL;}
static void negative(void){RRResultsPump e;RRResultsBoundView view;
 reset();epoch=1;CHECK(frame(0,true,false,lookup,draw,&e));fail_index=5;CHECK(!frame(2,false,false,lookup,draw,&e));CHECK(e.published==1&&bound.status==RR_RESULTS_BIND_IDENTITY_ERROR&&!bound.ready&&!rr_results_bind_view(&bound,&view));
 unsigned names_before=lookups;fail_index=-1;epoch=2;CHECK(!frame(2.1,false,false,lookup,draw,&e)&&lookups==names_before);CHECK(!frame(3,true,false,lookup,draw,&e)&&timer.pending==1);CHECK(frame(5,false,false,lookup,draw,&e)&&e.published==1);CHECK(rr_results_bind_view(&bound,&view)&&!strcmp(view.names[0],"N2 D0"));
 reset();epoch=10;draw_ok=false;CHECK(frame(0,true,false,lookup,draw,&e));CHECK(!frame(2,false,false,lookup,draw,&e)&&bound.ready&&bound.status==RR_RESULTS_BIND_DRAW_ERROR);CHECK(rr_results_bind_view(&bound,&view)&&!strcmp(view.names[0],"N10 D0"));
 names_before=lookups;epoch=11;draw_ok=true;CHECK(frame(2.1,false,false,NULL,draw,&e)&&lookups==names_before);CHECK(rr_results_bind_view(&bound,&view)&&!strcmp(view.names[0],"N10 D0"));
 run.host.phase=RR_SESSION_MENU;CHECK(frame(2.2,false,true,NULL,NULL,&e)&&bound.ready);memset(&view,0x5a,sizeof(view));RRResultsBoundView unchanged=view;CHECK(!rr_results_bind_view(&bound,&view)&&!memcmp(&view,&unchanged,sizeof(view)));
 reset();epoch=20;CHECK(frame(0,true,false,lookup,NULL,&e));CHECK(!frame(2,false,false,lookup,NULL,&e)&&bound.ready&&bound.status==RR_RESULTS_BIND_DRAW_ERROR);names_before=lookups;epoch=21;CHECK(frame(2.1,false,false,NULL,draw,&e)&&lookups==names_before);CHECK(rr_results_bind_view(&bound,&view)&&!strcmp(view.names[0],"N20 D0"));
 reset();CHECK(frame(0,true,false,NULL,draw,&e));names_before=lookups;CHECK(!frame(2,false,false,NULL,draw,&e)&&!bound.ready);CHECK(!frame(2.1,false,false,lookup,draw,&e)&&lookups==names_before);
 reset();memset(too_long,'A',512);too_long[512]=0;long_index=0;CHECK(frame(0,true,false,lookup,draw,&e));CHECK(!frame(2,false,false,lookup,draw,&e)&&bound.status==RR_RESULTS_BIND_IDENTITY_ERROR);names_before=lookups;long_index=-1;CHECK(!frame(2.1,false,false,lookup,draw,&e)&&lookups==names_before);
 epoch=31;CHECK(!frame(3,true,false,lookup,draw,&e));CHECK(frame(5,false,false,lookup,draw,&e)&&e.published==1);CHECK(rr_results_bind_view(&bound,&view)&&!strcmp(view.names[0],"N31 D0"));
 RRResultsTimer other={0},saved_timer=timer;names_before=lookups;e=(RRResultsPump){9,9};CHECK(!rr_results_bind_frame(&bound,&other,&run,NULL,6,false,lookup,&run,draw,&bound,&e));CHECK(bound.status==RR_RESULTS_BIND_OWNER_ERROR&&!e.callbacks&&!e.published&&!memcmp(&timer,&saved_timer,sizeof(timer))&&lookups==names_before);
 timer.revision=0;CHECK(!frame(6,false,false,lookup,draw,&e)&&bound.status==RR_RESULTS_BIND_OWNER_ERROR);timer=saved_timer;
 CHECK(!frame(4,false,false,lookup,draw,&e)&&bound.status==RR_RESULTS_BIND_CORE_ERROR&&timer.error&&!rr_results_bind_view(&bound,&view));
}
int main(int argc,char **argv){CHECK(argc==2);if(argc!=2)return 1;SetTraceLogLevel(LOG_WARNING);int bytes=0,count=0,cps[RR_UI_MAX_CODEPOINTS];unsigned char *data=LoadFileData(argv[1],&bytes);CHECK(data);if(!data)return 1;
 int requested=rr_ui_codepoints(cps,700);GlyphInfo *glyphs=LoadFontData(data,bytes,64,cps,requested,FONT_DEFAULT,&count);CHECK(glyphs&&count==96);if(!glyphs){UnloadFileData(data);return 1;}Rectangle *rects=NULL;Image atlas=GenImageFontAtlas(glyphs,&rects,count,64,4,0);CHECK(atlas.data&&rects);if(!rects){UnloadFontData(glyphs,count);UnloadFileData(data);return 1;}
 /* CPU metadata marker, no GPU object. Glyph painting is recorded only. */
 font=(Font){0};font.texture.id=1;font.baseSize=64;font.glyphCount=count;font.glyphs=glyphs;font.recs=rects;
 positive();unsigned positive_names=lookups,positive_draws=draws;negative();
 printf("RESULTS_BIND_SUMMARY cases=%zu actions=%u source_publications=%u capture_boundaries=%u positive_lookups=%u positive_draws=%u rows=%u paints=%u binding_bytes=%zu view_bytes=%zu failures=%u no_graphics=true\\n",sizeof(bind_cases)/sizeof(bind_cases[0]),actions,publications,capture_boundaries,positive_names,positive_draws,paint_rows,paint_commands,sizeof(bound),sizeof(RRResultsBoundView),failures);
 UnloadImage(atlas);MemFree(rects);UnloadFontData(glyphs,count);UnloadFileData(data);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
''',encoding='utf-8')
(p/'tests/test_results_bind_replay.c').write_text('''#include "rr_results_bind.h"
#include <stdio.h>
#include <string.h>
static RRResultsBinding replay_bound;static unsigned bound_failures,bound_draws,bound_lookups;static size_t bound_frame,bound_pub_frame,bound_revision;static char bound_name[64];
static const char *replay_name(void *ctx,size_t index,const RRLocalRacer *racer){const RRRunCurrent *r=ctx;if(index>=11||racer!=r->host.battle->roster[index])bound_failures++;bound_lookups++;snprintf(bound_name,sizeof(bound_name),"F%zu D%zu",bound_frame,index);return bound_name;}
static bool replay_draw(void *ctx,const RRResultsBoundView *view){(void)ctx;if(!view||view->count!=11||!view->revision){bound_failures++;return false;}if(view->revision!=bound_revision){bound_revision=view->revision;bound_pub_frame=bound_frame;}for(size_t i=0;i<11;i++){char expected[64];snprintf(expected,sizeof(expected),"F%zu D%zu",bound_pub_frame,i);if(strcmp(expected,view->names[i]))bound_failures++;}bound_draws++;return true;}
static bool replay_tick(RRResultsTimer *timer,const RRRunCurrent *run,const RRRunEvents *tick,double now,RRResultsPump *events){
 /* Each source trace is an independent application/timer lifetime, not a race
  * restart. Ordinary source start() only hides timer and retains binding. */
 if(!timer->clock_ready){memset(&replay_bound,0,sizeof(replay_bound));bound_revision=0;}
 bool ok=rr_results_bind_frame(&replay_bound,timer,run,tick,now/1000,false,replay_name,(void *)run,replay_draw,NULL,events);bound_frame++;return ok;
}
#define rr_run_results_after_tick replay_tick
#define main results_binding_reference_main
#include "test_results_timer.c"
#undef main
#undef rr_run_results_after_tick
int main(void){int result=results_binding_reference_main();if(bound_draws!=timer_visible||bound_lookups!=timer_published*11)bound_failures++;printf("RESULTS_BIND_REPLAY frames=%zu draws=%u names=%u source_publications=%u failures=%u production_inactive=true\\n",bound_frame,bound_draws,bound_lookups,timer_published,bound_failures);return result||bound_failures;}
''',encoding='utf-8')
if Path(__file__).resolve().parent.name=='roadsters-ui-audit':
 shutil.copy2(Path(__file__).resolve().with_name('capture-results-binding.mjs'),p/'tools/capture-results-binding.mjs');shutil.copy2(__file__,p/'tools/generate-results-binding.py')
print('Generated source-name freeze/row binding cases, explicit error guards and unchanged full-pack oracle replay adapter.')

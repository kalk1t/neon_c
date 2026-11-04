from pathlib import Path
import json,shutil,ast
p=Path(r'C:\Users\User\Projects\neon_c\win_app');backup=p/'reference/pre-results-bound-flow-20261104';backup.mkdir(exist_ok=False)
for f in ['CMakeLists.txt','src/presentation/rr_results_text.c','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 dest=backup/f;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/f,dest)
label_header=r'''#ifndef RR_RESULTS_LABEL_H
#define RR_RESULTS_LABEL_H
#include "rr_results_current.h"
typedef enum RRResultsLabelKind {RR_LABEL_STANDINGS,RR_LABEL_COINS} RRResultsLabelKind;
typedef struct RRResultsLabel {char name[512],value[64];size_t racer;bool player;} RRResultsLabel;
/* Source text composition only, shared by retained row renderer and new flow
 * binding. Snapshot numeric order and frozen names are inputs. No re-sort,
 * provider/account access, font/fit claim, truncation or replacement glyphs.
 * Invalid input leaves output unchanged; UTF8/font shaping is downstream. */
bool rr_results_label_build(const RRResultsCurrent *snapshot,RRResultsLabelKind kind,
 size_t row,const char *const *names,size_t count,size_t player,RRResultsLabel *out);
#endif
'''
label_source=r'''#include "rr_results_label.h"
#include "rr_race_ui.h"
#include <string.h>
bool rr_results_label_build(const RRResultsCurrent *s,RRResultsLabelKind kind,size_t row,const char *const *names,size_t count,size_t player,RRResultsLabel *out)
{
 if(!s||!out||!names||!count||count>RR_LOCAL_MAX_CARS||player>=count)return false;
 RRResultsLabel value={0};char clock[32];int racer;bool estimate=false,crown=false;
 if(kind==RR_LABEL_STANDINGS){if(s->time_count>RR_LOCAL_MAX_CARS||row>=s->time_count||!isfinite(s->times[row].seconds)||s->times[row].seconds<0||s->times[row].seconds>1e12)return false;racer=s->times[row].racer;estimate=s->times[row].estimated;rr_ui_time(clock,s->times[row].seconds);int n=snprintf(value.value,sizeof(value.value),"%s%s",estimate?"\xe2\x89\x88 ":"",clock);if(n<0||(size_t)n>=sizeof(value.value))return false;}
 else if(kind==RR_LABEL_COINS){if(s->coin_count>RR_LOCAL_MAX_CARS||row>=s->coin_count||s->coins[row].coins<0)return false;racer=s->coins[row].racer;crown=row==0;int n=snprintf(value.value,sizeof(value.value),"%d",s->coins[row].coins);if(n<0||(size_t)n>=sizeof(value.value))return false;}
 else return false;
 if(racer<0||(size_t)racer>=count||!names[racer])return false;
 size_t length=0;while(length<512&&names[racer][length])length++;if(length>507)return false;
 int n=snprintf(value.name,sizeof(value.name),"%s%s",crown?"\xe2\x99\x9b ":"",names[racer]);if(n<0||(size_t)n>=sizeof(value.name))return false;
 value.racer=(size_t)racer;value.player=(size_t)racer==player;*out=value;return true;
}
'''
header=r'''#ifndef RR_RESULTS_BOUND_FLOW_H
#define RR_RESULTS_BOUND_FLOW_H
#include "rr_results_flow_draw.h"
#include "rr_results_label.h"
typedef struct RRResultsFlowBinding {
 RRResultsLabel labels[2][RR_LOCAL_MAX_CARS];RRResultsFlowText rows[2][RR_LOCAL_MAX_CARS];
 bool player[2][RR_LOCAL_MAX_CARS];size_t counts[2],revision;
} RRResultsFlowBinding;
/* Caller-owned persistent binding. The frozen bound view is formatted in its
 * existing numeric display order; complete-board validation precedes commit.
 * Requires each expected list to cover the roster once. Failure leaves binding
 * unchanged; never paints an old binding after new-view failure. Strings are
 * copied; row pointers refer INTO binding, which must not be shallow-copied.
 * This pure function does not advance timers, look up identities or tick core.
 * Font/unsupported-text/whole-card/graphics qualification remains downstream. */
bool rr_results_flow_prepare(const RRResultsBoundView *view,RRResultsFlowBinding *binding);
/* Bound-draw callback adapter: profile supplies qualified tracks/caption/other
 * slots and viewport/scroll only. Its row pointers/counts are overwritten from
 * frozen binding. All inputs/work/binding/row storage must be disjoint and stable
 * for the call. A repaint failure never replays a simulation/account operation.
 * Does not activate native main or enable rivals/coins/combat/rewards. */
RRResultsNameStatus rr_ui_results_bound_flow_draw(RRUI *ui,RRResultsArt *art,
 const RRResultsBoundView *view,const RRResultsBodyInput *profile,int device_scale,
 RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row);
#endif
'''
source=r'''#include "rr_results_bound_flow.h"
bool rr_results_flow_prepare(const RRResultsBoundView *v,RRResultsFlowBinding *binding)
{
 if(!v||!binding||!v->results||!v->revision||!v->count||v->count>RR_LOCAL_MAX_CARS||v->player>=v->count)return false;
 const RRResultsCurrent *s=v->results;if(s->coin_count!=v->count||s->time_count!=(v->count>1?v->count:0))return false;
 const size_t counts[2]={s->time_count,s->coin_count};RRResultsLabel check;
 for(int pass=0;pass<2;pass++)for(int k=0;k<2;k++){
  unsigned seen=0;
  for(size_t j=0;j<counts[k];j++){
   if(!rr_results_label_build(s,k?RR_LABEL_COINS:RR_LABEL_STANDINGS,j,v->names,v->count,v->player,&check))return false;
   unsigned bit=1u<<(unsigned)check.racer;if(seen&bit)return false;seen|=bit;
   if(pass){binding->labels[k][j]=check;binding->rows[k][j]=(RRResultsFlowText){binding->labels[k][j].name,binding->labels[k][j].value};binding->player[k][j]=check.player;}
  }
 }
 binding->counts[0]=counts[0];binding->counts[1]=counts[1];binding->revision=v->revision;return true;
}
RRResultsNameStatus rr_ui_results_bound_flow_draw(RRUI *ui,RRResultsArt *art,const RRResultsBoundView *view,const RRResultsBodyInput *profile,int scale,RRResultsFlowBinding *binding,RRResultsListWork *work,RRResultsRow *row)
{
 if(!profile||!rr_results_flow_prepare(view,binding))return RR_NAME_INVALID;
 RRResultsBodyInput input=*profile;input.standings=binding->rows[0];input.coins=binding->rows[1];input.standings_count=binding->counts[0];input.coin_count=binding->counts[1];
 return rr_ui_results_flow_draw(ui,art,&input,binding->player[0],binding->player[1],scale,work,row);
}
'''
for f,s in [('src/presentation/rr_results_label.h',label_header),('src/presentation/rr_results_label.c',label_source),('src/presentation/rr_results_bound_flow.h',header),('src/presentation/rr_results_bound_flow.c',source)]:assert not(p/f).exists();(p/f).write_text(s,encoding='utf-8')
# Extract exactly the source text composition used by the retained row painter.
f=p/'src/presentation/rr_results_text.c';s=f.read_text();start=s.index(' char label[RR_RESULTS_TEXT_BYTES]');end=s.index(' RRResultsRow v={0};',start)
s=s[:start]+''' RRResultsLabel label;if(!rr_results_label_build(s,kind==RR_RESULTS_STANDINGS?RR_LABEL_STANDINGS:kind==RR_RESULTS_COINS?RR_LABEL_COINS:(RRResultsLabelKind)2,row,names,count,player,&label))return RR_RESULTS_ROW_INVALID;
'''+s[end:];s=s.replace('#include "rr_results_text.h"','#include "rr_results_text.h"\n#include "rr_results_label.h"').replace('font,label,scale','font,label.name,scale').replace('font,value,scale','font,label.value,scale').replace('v.shadow=(size_t)racer==player;','v.shadow=label.player;');f.write_text(s,encoding='utf-8')
pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1');observations=json.loads((pack/'observations.json').read_text(encoding='utf-8'));gold=json.loads((pack/'native-layout-goldens.json').read_text(encoding='utf-8'));lookup={(o['fixture'],o['scale'],o['offset']):o for o in observations};cases=[]
for g in gold:
 o=lookup[(g['fixture'],g['scale'],g['offset'])];racers=o['input']['racers'];names=[v['name']for v in racers];player=next(i for i,v in enumerate(racers)if v['isPlayer']);times=[];coins=[]
 for k,kind in enumerate(['res-standings','res-coins']):
  for j,row in enumerate(g['lists'][kind]['rows']):
   name=row['name'][2:]if k==1 and j==0 else row['name'];index=names.index(name);assert ('me'in row['classes'].split())==(index==player)
   if k==0:
    value=row['value'];estimated=value.startswith('\u2248 ');value=value[2:]if estimated else value;minutes,seconds=value.split(':');times.append([index,int(minutes)*60+float(seconds),estimated])
   else:coins.append([index,int(row['value'])])
 cases.append(dict(fixture=g['fixture'],scale=g['scale'],offset=g['offset'],names=names,player=player,times=times,coins=coins))
def cs(s):return '"'+''.join(chr(x)if 32<=x<127 and chr(x)not in '\\"'else '\\%03o'%x for x in s.encode('utf-8'))+'"'
fixture='''/* Source frozen identity/ordered numeric display fixtures. Numeric times
 * are parsed from captured displayed values, not a new live/core race claim. */
typedef struct BoundFlowCase {size_t count,player,time_count;const char *names[11];RRResultsTime times[11];RRResultsCoin coins[11];} BoundFlowCase;
static const BoundFlowCase bound_cases[]={
'''
# Use actual shared current result entry typedefs; inspect header names below.
for c in cases:
 fixture+='{%d,%d,%d,{%s},{%s},{%s}},\n'%(len(c['names']),c['player'],len(c['times']),','.join(cs(x)for x in c['names']),','.join('{%d,%s,%s}'%(x,repr(t),str(e).lower())for x,t,e in c['times'])or'{0}',','.join('{%d,%d}'%(x,n)for x,n in c['coins']))
fixture+='};\n';(p/'tests/results_bound_flow_reference.h').write_text(fixture,encoding='ascii')
oldtest=(p/'tests/test_results_flow_draw.c').read_text();prefix=oldtest[:oldtest.index('static RRResultsBodyInput input(')];prefix=prefix.replace('#include "rr_results_flow_draw.h"','#include "rr_results_bound_flow.h"\n#include "results_bound_flow_reference.h"');loader=oldtest[oldtest.index(' CHECK(argc==2);'):oldtest.index(' CHECK(COUNT(body_cases)==COUNT(draw_cases));')]
test=prefix+r'''static RRResultsFlowBinding binding,saved_binding;static RRResultsCurrent snapshot;static RRResultsBoundView view;
static RRResultsBodyInput input(const BodyCase *c){RRResultsBodyInput i={c->width,c->height,c->scroll,c->left,c->right,c->caption_height,NULL,NULL,999,999,c->other};return i;}
int main(int argc,char **argv){
'''+loader+r'''
 CHECK(COUNT(bound_cases)==COUNT(body_cases));checking=true;
 for(size_t n=0;n<COUNT(bound_cases);n++){
  const BoundFlowCase *b=bound_cases+n;current=draw_cases+n;cursor=current->first;end=cursor+current->count;
  snapshot=(RRResultsCurrent){0};snapshot.time_count=b->time_count;snapshot.coin_count=b->count;memcpy(snapshot.times,b->times,sizeof(snapshot.times));memcpy(snapshot.coins,b->coins,sizeof(snapshot.coins));
  view=(RRResultsBoundView){0};view.results=&snapshot;view.count=b->count;view.player=b->player;view.revision=n+1;for(size_t j=0;j<b->count;j++)view.names[j]=b->names[j];
  RRResultsBodyInput i=input(body_cases+n);CHECK(rr_ui_results_bound_flow_draw(&ui,&art,&view,&i,current->scale,&binding,&work,&row)==RR_NAME_READY);CHECK(cursor==end&&binding.revision==view.revision);
  for(int k=0;k<2;k++)for(size_t j=0;j<binding.counts[k];j++){
   CHECK(!strcmp(binding.rows[k][j].name,body_cases[n].rows[k][j].name)&&!strcmp(binding.rows[k][j].value,body_cases[n].rows[k][j].value)&&binding.player[k][j]==current->players[k][j]);
   CHECK(binding.rows[k][j].name==binding.labels[k][j].name&&binding.rows[k][j].value==binding.labels[k][j].value);
  }
 }
 checking=false;size_t before=commands;RRResultsBodyInput i=input(body_cases);memcpy(&saved_binding,&binding,sizeof(binding));
 int racer=snapshot.coins[snapshot.coin_count-1].racer;snapshot.coins[snapshot.coin_count-1].racer=99;
 CHECK(rr_ui_results_bound_flow_draw(&ui,&art,&view,&i,1,&binding,&work,&row)==RR_NAME_INVALID&&commands==before&&!memcmp(&binding,&saved_binding,sizeof(binding)));snapshot.coins[snapshot.coin_count-1].racer=racer;
 snapshot.coins[snapshot.coin_count-1].racer=snapshot.coins[0].racer;CHECK(!rr_results_flow_prepare(&view,&binding)&&!memcmp(&binding,&saved_binding,sizeof(binding)));snapshot.coins[snapshot.coin_count-1].racer=racer;
 size_t revision=view.revision;view.revision=0;CHECK(!rr_results_flow_prepare(&view,&binding)&&!memcmp(&binding,&saved_binding,sizeof(binding)));view.revision=revision;
 RRResultsLabel label,saved;memset(&label,0x5a,sizeof(label));saved=label;CHECK(!rr_results_label_build(&snapshot,(RRResultsLabelKind)2,0,view.names,view.count,view.player,&label)&&!memcmp(&label,&saved,sizeof(label)));
 CHECK(!rr_results_flow_prepare(NULL,&binding)&&!rr_results_flow_prepare(&view,NULL));
 CHECK(rr_ui_results_bound_flow_draw(&ui,&art,&view,NULL,1,&binding,&work,&row)==RR_NAME_INVALID&&commands==before);
 printf("RESULTS_BOUND_FLOW_SUMMARY observed_boards=%zu commands=%zu formatted_rows=570 binding_bytes=%zu work_bytes=%zu failures=%u frozen_order_and_identity=true no_provider_or_timer_calls=true graphics_contexts=0 main_unchanged=true\n",COUNT(bound_cases),commands,sizeof(binding),sizeof(work),failures);
 UnloadImage(atlas);MemFree(rects);UnloadFontData(glyphs,count);UnloadFileData(data);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
''';test=test.replace('#include <stdlib.h>','#include <stdlib.h>\n#include <string.h>');(p/'tests/test_results_bound_flow.c').write_text(test,encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text().replace('add_library(rr_core STATIC ','add_library(rr_core STATIC src/presentation/rr_results_label.c ');marker='        add_executable(rr_results_flow_draw_tests ';assert s.count(marker)==1
block='''        add_executable(rr_results_bound_flow_tests tests/test_results_bound_flow.c src/presentation/rr_results_bound_flow.c src/presentation/rr_results_flow_draw.c src/presentation/rr_results_text.c src/presentation/rr_subtext.c)
        target_include_directories(rr_results_bound_flow_tests PRIVATE src/presentation)
        target_link_libraries(rr_results_bound_flow_tests PRIVATE rr_core ${RR_RAYLIB_TARGET})
        target_compile_definitions(rr_results_bound_flow_tests PRIVATE DrawTextCodepoint=rr_record_DrawTextCodepoint DrawTexturePro=rr_record_DrawTexturePro)
        set_property(TARGET rr_results_bound_flow_tests APPEND PROPERTY LINK_DEPENDS ${RR_UI_FONT_FILES})
        if(MSVC)
            target_compile_options(rr_results_bound_flow_tests PRIVATE /W4 /WX)
        else()
            target_compile_options(rr_results_bound_flow_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
        endif()
        if(EMSCRIPTEN)
            target_link_options(rr_results_bound_flow_tests PRIVATE "-sUSE_GLFW=3" "-sALLOW_MEMORY_GROWTH=1" "--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme@/ui-theme")
            add_test(NAME browser_results_frozen_bound_flow_drawing COMMAND rr_results_bound_flow_tests "/ui-theme/Orbitron-700.ttf")
        else()
            add_test(NAME browser_results_frozen_bound_flow_drawing COMMAND rr_results_bound_flow_tests "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui-theme/Orbitron-700.ttf")
        endif()
''';s=s.replace(marker,block+marker).replace('add_executable(RoadstersRace ','add_executable(RoadstersRace src/presentation/rr_results_bound_flow.c ');f.write_text(s)
dest=p/'reference/ui-parity-20261103/results-bound-flow-capture';dest.mkdir(exist_ok=False);(dest/'reference.json').write_text(json.dumps(dict(package=str(pack),cases=cases,scope='Frozen captured display numbers and roster identity into native command bridge. Display seconds parsed from source shown times, not fresh core/full-pack simulation or graphics acceptance.'),indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(boards=len(cases),rows=sum(len(c['times'])+len(c['coins'])for c in cases))))

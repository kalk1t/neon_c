from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');b=p/'reference/pre-results-list-flow-20261104';b.mkdir(exist_ok=False)
for name in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 q=b/name;q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/name,q)
header=r'''#ifndef RR_RESULTS_LIST_FLOW_H
#define RR_RESULTS_LIST_FLOW_H
#include "rr_results_name_flow.h"
typedef struct RRResultsNameTail {double y,advance;} RRResultsNameTail;
/* Current measured source one-line value float, relative to li. Prior right
 * floats must be inside width, line-aligned,21.296875 high and at/before li top.
 * Other contexts return NEEDS_FLOW. Tail is final NAME line-box y and raw pen
 * advance, not its outward Range width. Source cursor floors1/64; preferred
 * value width rounds outward. No global epsilon, shrink/truncate or repaint. */
RRResultsNameStatus rr_results_value_float(const char *value,double width,RRResultsNameTail tail,
 const RRResultsRightFloat *previous,size_t count,RRResultsCharacters *scratch,RRResultsRightFloat *out);
typedef enum RRResultsListKind {RR_FLOW_STANDINGS,RR_FLOW_COINS} RRResultsListKind;
typedef struct RRResultsFlowText {const char *name,*value;} RRResultsFlowText;
typedef struct RRResultsListSummary {size_t rows;double normal_height,float_bottom,height;} RRResultsListSummary;
typedef struct RRResultsListWork {RRResultsCharacters name_characters,value_characters;RRResultsNameFlow name;RRResultsRightFloat absolute[11],relative[11];} RRResultsListWork;
typedef struct RRResultsListRowView {size_t index;double y,height;const RRResultsNameFlow *name;const RRResultsCharacters *name_characters,*value_characters;RRResultsRightFloat value;} RRResultsListRowView;
typedef void (*RRResultsListSink)(void *context,const RRResultsListRowView *row);
/* Fully coupled layout from frozen strings and qualified content width only.
 * Ledger states are GENERATED, not measured inputs. Normal li flow advances by
 * name height even when current value floats below it. Source standings columns:1
 * contains floats; ordinary coins height remains normal flow. Both extents kept.
 * Preflight entire list before first synchronous sink. NULL sink measures only.
 * Borrowed views valid only during callback; sink must not mutate inputs/work or
 * retain pointers. Caller owns persistent work (sizes in tests), no heap/large
 * local plans. Work may change on failure; summary and external sinks do not.
 * No sorting/account/progression/main/painter/graphics activation. */
RRResultsNameStatus rr_results_list_flow(RRResultsListKind kind,const RRResultsFlowText *rows,size_t count,double width,
 RRResultsListWork *work,RRResultsListSink sink,void *context,RRResultsListSummary *out);
#endif
'''
source=r'''#include "rr_results_list_flow.h"
#include <math.h>
#include <float.h>
static const double line_height=21.296875;
RRResultsNameStatus rr_results_value_float(const char *value,double width,RRResultsNameTail tail,const RRResultsRightFloat *f,size_t n,RRResultsCharacters *scratch,RRResultsRightFloat *out)
{
 if(!out||!scratch||!isfinite(width)||width<=0||width>FLT_MAX||!isfinite(tail.y)||!isfinite(tail.advance)||tail.y<0||tail.advance<0||n>11||(n&&!f))return RR_NAME_INVALID;
 if(tail.y/line_height!=floor(tail.y/line_height)||tail.advance>width)return RR_NAME_NEEDS_FLOW;
 for(size_t i=0;i<n;i++){
  if(!isfinite(f[i].x)||!isfinite(f[i].y)||!isfinite(f[i].width)||!isfinite(f[i].height)||f[i].width<=0||f[i].height<=0||!isfinite(f[i].x+f[i].width)||!isfinite(f[i].y+f[i].height))return RR_NAME_INVALID;
  if(f[i].x<0||f[i].x+f[i].width>width||f[i].y>0||f[i].height!=line_height||f[i].y/line_height!=floor(f[i].y/line_height))return RR_NAME_NEEDS_FLOW;
 }
 RRResultsAdvanceStatus status=rr_results_characters(value,scratch);if(status==RR_ADVANCE_INVALID)return RR_NAME_INVALID;
 if(status!=RR_ADVANCE_READY||!scratch->count||scratch->range_width>width)return RR_NAME_NEEDS_FLOW;
 double y=tail.y,cursor=floor(tail.advance*64)/64;
 for(size_t attempt=0;attempt<13;attempt++){
  double right=width;for(size_t i=0;i<n;i++)if(f[i].y<y+line_height&&f[i].y+f[i].height>y)right=fmin(right,f[i].x);
  double used=y==tail.y?cursor:0;
  if(scratch->range_width+used<=right){*out=(RRResultsRightFloat){right-scratch->range_width,y,scratch->range_width,line_height};return RR_NAME_READY;}
  double next=y+line_height;if(!isfinite(next)||next<=y)return RR_NAME_NEEDS_FLOW;y=next;
 }
 return RR_NAME_NEEDS_FLOW;
}
static double pen(const RRResultsCharacters *s,size_t index){return index==s->count?s->advance:s->ranges[index].pen;}
static RRResultsNameStatus layout(RRResultsListKind kind,const RRResultsFlowText *rows,size_t count,double width,RRResultsListWork *w,RRResultsListSink sink,void *ctx,RRResultsListSummary *summary)
{
 double y=0,bottom=0;
 for(size_t row=0;row<count;row++){
  for(size_t i=0;i<row;i++){w->relative[i]=w->absolute[i];w->relative[i].y-=y;}
  RRResultsNameStatus status=rr_results_name_flow(rows[row].name,width,w->relative,row,&w->name_characters,&w->name);if(status!=RR_NAME_READY)return status;
  const RRResultsNameLine *last=w->name.lines+w->name.count-1;
  RRResultsNameTail tail={last->y-1,pen(&w->name_characters,last->first+last->count)-pen(&w->name_characters,last->first)};
  RRResultsRightFloat placed;status=rr_results_value_float(rows[row].value,width,tail,w->relative,row,&w->value_characters,&placed);if(status!=RR_NAME_READY)return status;
  placed.y+=y;w->absolute[row]=placed;bottom=fmax(bottom,placed.y+placed.height);
  if(sink){RRResultsListRowView view={row,y,w->name.height,&w->name,&w->name_characters,&w->value_characters,placed};sink(ctx,&view);}
  y+=w->name.height;
 }
 *summary=(RRResultsListSummary){count,y,bottom,kind==RR_FLOW_STANDINGS?fmax(y,bottom):y};return RR_NAME_READY;
}
RRResultsNameStatus rr_results_list_flow(RRResultsListKind kind,const RRResultsFlowText *rows,size_t count,double width,RRResultsListWork *work,RRResultsListSink sink,void *ctx,RRResultsListSummary *out)
{
 if(!work||!out||(unsigned)kind>RR_FLOW_COINS||count>11||(count&&!rows)||!isfinite(width)||width<=0||width>FLT_MAX)return RR_NAME_INVALID;
 RRResultsListSummary summary;RRResultsNameStatus status=layout(kind,rows,count,width,work,NULL,NULL,&summary);if(status!=RR_NAME_READY)return status;
 if(sink){status=layout(kind,rows,count,width,work,sink,ctx,&summary);if(status!=RR_NAME_READY)return status;}
 *out=summary;return RR_NAME_READY;
}
'''
for name,s in [('rr_results_list_flow.h',header),('rr_results_list_flow.c',source)]:
 f=p/'src/presentation'/name;assert not f.exists();f.write_text(s,encoding='utf-8')
pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1');gold=json.loads((pack/'native-layout-goldens.json').read_text(encoding='utf-8'));obs=json.loads((pack/'observations.json').read_text(encoding='utf-8'))
lookup={(o['fixture'],o['scale'],o['offset'],name,r['index']):r for o in obs for name,l in o['lists'].items() for r in l['rows']}
cases=[]
for o in gold:
 for kind,l in o['lists'].items():
  rows=[]
  for r in l['rows']:
   source=lookup[(o['fixture'],o['scale'],o['offset'],kind,r['ordinal'])];chars=source['directTextNodes'][0]['characterRectsViewport'];name_lines=[]
   for rect in r['nameLineRects']:
    absolute_y=rect['y']-r['box']['y']+source['viewport']['y'];indices=[i for i,c in enumerate(chars) if any(v['width']>0 and v['y']==absolute_y for v in c['rects'])];assert indices
    name_lines.append([min(indices),max(indices)-min(indices)+1,rect['x']-r['box']['x'],rect['y']-r['box']['y'],rect['width'],rect['height']])
   f=r['floatBox'];rows.append(dict(name=r['name'],value=r['value'],y=r['box']['y']-l['box']['y'],height=r['box']['height'],lines=name_lines,value_box=[f['x']-(l['box']['x']+26),f['y']-l['box']['y'],f['width'],f['height']]))
  cases.append(dict(fixture=o['fixture'],scale=o['scale'],offset=o['offset'],kind=0 if kind=='res-standings' else 1,width=l['box']['width']-26,height=l['box']['height'],normal_height=sum(r['height'] for r in rows),float_bottom=max([0]+[r['value_box'][1]+r['value_box'][3] for r in rows]),rows=rows))
assert len(cases)==120 and sum(len(c['rows']) for c in cases)==570
def c(s):return '"'+''.join(chr(x) if 32<=x<127 and chr(x) not in '\\"' else '\\%03o'%x for x in s.encode('utf-8'))+'"'
fixture='''/* Independent source observations; only strings/width/kind are inputs.
 * No measured preceding float ledger enters the coupled runtime. */
typedef struct FlowExpectedRow {const char *name,*value;double y,height;size_t count;RRResultsNameLine lines[2];RRResultsRightFloat value_box;} FlowExpectedRow;
typedef struct FlowExpectedList {RRResultsListKind kind;double width;size_t count;double normal_height,float_bottom,height;FlowExpectedRow rows[11];} FlowExpectedList;
static const FlowExpectedList flow_lists[]={
'''
for x in cases:
 rows=[]
 for r in x['rows']:
  lines=','.join('{%d,%d,%s}'%(l[0],l[1],','.join(repr(v) for v in l[2:])) for l in r['lines']);assert len(r['lines'])<=2
  rows.append('{%s,%s,%s,%s,%d,{%s},{%s}}'%(c(r['name']),c(r['value']),repr(r['y']),repr(r['height']),len(r['lines']),lines,','.join(repr(v) for v in r['value_box'])))
 fixture+='{(RRResultsListKind)%d,%s,%d,%s,%s,%s,{%s}},\n'%(x['kind'],repr(x['width']),len(rows),repr(x['normal_height']),repr(x['float_bottom']),repr(x['height']),','.join(rows) or '{0}')
fixture+='};\n';(p/'tests/results_list_flow_reference.h').write_text(fixture,encoding='ascii')
test=r'''#include "rr_results_list_flow.h"
#include "results_list_flow_reference.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,callbacks,rows_seen,lines_seen,relocated;static const FlowExpectedList *expected;
static RRResultsListWork work;static RRResultsListSummary summary,guard,saved;static RRResultsFlowText rows[11];
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"list flow line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void observe(void *ctx,const RRResultsListRowView *v){
 CHECK(ctx==&work&&v->index==callbacks&&v->index<expected->count);callbacks++;if(v->index>=expected->count)return;const FlowExpectedRow *e=expected->rows+v->index;
 CHECK(v->y==e->y&&v->height==e->height&&v->name->count==e->count&&v->name_characters->count>0&&v->value_characters->count>0);
 CHECK(v->value.x==e->value_box.x&&v->value.y==e->value_box.y&&v->value.width==e->value_box.width&&v->value.height==e->value_box.height);
 for(size_t i=0;i<v->name->count&&i<e->count;i++){const RRResultsNameLine *a=v->name->lines+i,*b=e->lines+i;CHECK(a->first==b->first&&a->count==b->count&&a->x==b->x&&a->y==b->y&&a->width==b->width&&a->height==b->height);lines_seen++;}
 rows_seen++;relocated+=v->value.y>v->y;
}
int main(void){
 for(size_t i=0;i<COUNT(flow_lists);i++){
  expected=flow_lists+i;for(size_t j=0;j<expected->count;j++)rows[j]=(RRResultsFlowText){expected->rows[j].name,expected->rows[j].value};callbacks=0;
  CHECK(rr_results_list_flow(expected->kind,rows,expected->count,expected->width,&work,observe,&work,&summary)==RR_NAME_READY);
  CHECK(callbacks==expected->count&&summary.rows==expected->count&&summary.normal_height==expected->normal_height&&summary.float_bottom==expected->float_bottom&&summary.height==expected->height);
 }
 /* Last-row failure must not expose even the first row's geometry. */
 rows[0]=(RRResultsFlowText){"ACE","0"};rows[1]=(RRResultsFlowText){"YOU","^"};callbacks=0;memset(&guard,0x5a,sizeof(guard));saved=guard;
 CHECK(rr_results_list_flow(RR_FLOW_COINS,rows,2,232.25,&work,observe,&work,&guard)==RR_NAME_INVALID&&!callbacks&&!memcmp(&guard,&saved,sizeof(guard)));
 rows[1].value="00:00.00";rows[1].name="AWX";CHECK(rr_results_list_flow(RR_FLOW_COINS,rows,2,232.25,&work,observe,&work,&guard)==RR_NAME_NEEDS_FLOW&&!callbacks&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_list_flow(RR_FLOW_COINS,NULL,0,232.25,&work,NULL,NULL,&summary)==RR_NAME_READY&&!summary.rows&&!summary.height&&!summary.float_bottom);
 CHECK(rr_results_list_flow((RRResultsListKind)2,rows,1,232.25,&work,NULL,NULL,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_list_flow(RR_FLOW_COINS,rows,12,232.25,&work,NULL,NULL,&guard)==RR_NAME_INVALID);
 CHECK(rr_results_list_flow(RR_FLOW_COINS,rows,1,NAN,&work,NULL,NULL,&guard)==RR_NAME_INVALID);
 RRResultsRightFloat placed,before;memset(&placed,0x5a,sizeof(placed));before=placed;RRResultsRightFloat prior={100,21.296875,100,21.296875};
 CHECK(rr_results_value_float("0",232.25,(RRResultsNameTail){0,35},&prior,1,&work.value_characters,&placed)==RR_NAME_NEEDS_FLOW&&!memcmp(&placed,&before,sizeof(placed)));
 CHECK(rr_results_value_float("0",232.25,(RRResultsNameTail){0,NAN},NULL,0,&work.value_characters,&placed)==RR_NAME_INVALID&&!memcmp(&placed,&before,sizeof(placed)));
 CHECK(rr_results_value_float("0",1,(RRResultsNameTail){0,0},NULL,0,&work.value_characters,&placed)==RR_NAME_NEEDS_FLOW&&!memcmp(&placed,&before,sizeof(placed)));
 CHECK(rr_results_value_float(NULL,232.25,(RRResultsNameTail){0,0},NULL,0,&work.value_characters,&placed)==RR_NAME_INVALID);
 printf("RESULTS_LIST_FLOW_SUMMARY observed_lists=%zu rows=%u name_fragments=%u relocated_values=%u work_bytes=%zu summary_bytes=%zu failures=%u prior_floats_generated=true no_painter_or_main_activation=true\n",COUNT(flow_lists),rows_seen,lines_seen,relocated,sizeof(work),sizeof(summary),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
'''
f=p/'tests/test_results_list_flow.c';assert not f.exists();f.write_text(test,encoding='utf-8')
dest=p/'reference/ui-parity-20261103/results-list-flow-capture';dest.mkdir(exist_ok=False);analysis=json.loads((Path(__file__).parent/'results-list-flow-analysis.json').read_text(encoding='utf-8'));assert analysis['hypotheses'][0]['mismatches']==0
(dest/'reference.json').write_text(json.dumps(dict(package=str(pack),cases=cases,hypotheses=analysis['hypotheses'],scope='All expected geometry from browser goldens/character ranges. Only ordered strings, list kind and measured content widths enter runtime. No measured prior float or row position is supplied to the coupled solver.'),indent=2)+'\n',encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text().replace('add_library(rr_core STATIC ','add_library(rr_core STATIC src/presentation/rr_results_list_flow.c ');marker='    add_executable(rr_results_name_flow_tests ';assert s.count(marker)==1
block='''    add_executable(rr_results_list_flow_tests tests/test_results_list_flow.c)
    target_include_directories(rr_results_list_flow_tests PRIVATE src/presentation)
    target_link_libraries(rr_results_list_flow_tests PRIVATE rr_core)
    if(MSVC)
        target_compile_options(rr_results_list_flow_tests PRIVATE /W4 /WX)
    else()
        target_compile_options(rr_results_list_flow_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME browser_results_coupled_value_float_flow COMMAND rr_results_list_flow_tests)
''';f.write_text(s.replace(marker,block+marker))
print(json.dumps(dict(lists=120,rows=570,source_float_states_used_as_inputs=False,source_height_profiles=['standings contains floats','coins normal flow'])))

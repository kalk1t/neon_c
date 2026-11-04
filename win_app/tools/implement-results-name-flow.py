from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');b=p/'reference/pre-results-name-flow-20261104';b.mkdir(exist_ok=False)
for name in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 q=b/name;q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/name,q)
header=r'''#ifndef RR_RESULTS_NAME_FLOW_H
#define RR_RESULTS_NAME_FLOW_H
#include "rr_results_advance.h"
enum {RR_RESULTS_NAME_LINES=128,RR_RESULTS_NAME_FLOATS=11};
typedef struct RRResultsRightFloat {double x,y,width,height;} RRResultsRightFloat;
typedef struct RRResultsNameLine {size_t first,count;double x,y,width,height;} RRResultsNameLine;
typedef struct RRResultsNameFlow {RRResultsNameLine lines[RR_RESULTS_NAME_LINES];size_t count;double height;} RRResultsNameFlow;
typedef enum RRResultsNameStatus {RR_NAME_INVALID,RR_NAME_NEEDS_FLOW,RR_NAME_READY} RRResultsNameStatus;
/* Source15px/700 NAME text only. Container width and preceding right-float
 * boxes are qualified inputs relative to this li top/left. Current measured
 * one-line floats must align to21.296875px line boxes and remain inside width;
 * other float geometry returns NEEDS_FLOW. No current-row float is placed.
 * Supports measured single spaces and Latin-letter hyphen breaks. Unqualified
 * kerning/collapsing whitespace/break contexts or unbroken full-width overflow
 * return NEEDS_FLOW; no truncation, tidy-row rewrite or universal fit epsilon.
 * Output line ranges use original codepoint offsets; collapsed break spaces
 * are omitted. y is selection-range top (line+1), h19; not a glyph baseline.
 * height includes leading float clearance even with only one visible text line.
 * Caller owns output and character scratch; no heap/large local. Scratch may
 * change on failure, output does not. Text/floats/scratch/out must not alias or
 * mutate across synchronous deterministic preflight/emission passes. */
RRResultsNameStatus rr_results_name_flow(const char *text,double width,
 const RRResultsRightFloat *previous,size_t float_count,
 RRResultsCharacters *scratch,RRResultsNameFlow *out);
#endif
'''
source=r'''#include "rr_results_name_flow.h"
#include <math.h>
#include <float.h>
#include <string.h>
static const double line_height=21.296875;
static bool latin(uint32_t c){return (c>=65&&c<=90)||(c>=97&&c<=122);}
static double pen(const RRResultsCharacters *s,size_t i){return i==s->count?s->advance:s->ranges[i].pen;}
static RRResultsNameStatus pass(const RRResultsCharacters *s,double width,const RRResultsRightFloat *f,size_t nf,RRResultsNameFlow *out)
{
 size_t start=0,count=0,steps=0;double y=0;
 while(start<s->count){
  if(steps++>=RR_RESULTS_NAME_LINES+RR_RESULTS_NAME_FLOATS+1)return RR_NAME_NEEDS_FLOW;
  double right=width,next_y=DBL_MAX;
  for(size_t i=0;i<nf;i++)if(f[i].y<y+line_height&&f[i].y+f[i].height>y){right=fmin(right,f[i].x);next_y=fmin(next_y,f[i].y+f[i].height);}
  size_t end=s->count,next=end;double origin=pen(s,start);
  if(pen(s,end)-origin>right){
   end=start;next=start;
   for(size_t i=start;i<s->count;i++){
    uint32_t cp=s->ranges[i].codepoint;size_t candidate,following;
    if(cp==32){candidate=i;following=i+1;}
    else if(cp==45){
     if(!i||i+1==s->count||!latin(s->ranges[i-1].codepoint)||!latin(s->ranges[i+1].codepoint))return RR_NAME_NEEDS_FLOW;
     candidate=i+1;following=i+1;
    }else continue;
    if(candidate>start&&pen(s,candidate)-origin<=right){end=candidate;next=following;}
   }
   if(end==start){if(next_y==DBL_MAX||next_y<=y)return RR_NAME_NEEDS_FLOW;y=next_y;continue;}
  }
  if(count==RR_RESULTS_NAME_LINES)return RR_NAME_NEEDS_FLOW;
  if(out)out->lines[count]=(RRResultsNameLine){start,end-start,0,y+1,ceil((pen(s,end)-origin)*64)/64,19};
  count++;start=next;y+=line_height;
 }
 if(out){out->count=count;out->height=y;}return RR_NAME_READY;
}
RRResultsNameStatus rr_results_name_flow(const char *text,double width,const RRResultsRightFloat *f,size_t nf,RRResultsCharacters *scratch,RRResultsNameFlow *out)
{
 if(!scratch||!out||!isfinite(width)||width<=0||width>FLT_MAX||nf>RR_RESULTS_NAME_FLOATS||(nf&&!f))return RR_NAME_INVALID;
 for(size_t i=0;i<nf;i++){
  if(!isfinite(f[i].x)||!isfinite(f[i].y)||!isfinite(f[i].width)||!isfinite(f[i].height)||f[i].width<=0||f[i].height<=0||!isfinite(f[i].x+f[i].width)||!isfinite(f[i].y+f[i].height))return RR_NAME_INVALID;
  if(f[i].x<0||f[i].x+f[i].width>width||f[i].height!=line_height||f[i].y/line_height!=floor(f[i].y/line_height))return RR_NAME_NEEDS_FLOW;
 }
 RRResultsAdvanceStatus status=rr_results_characters(text,scratch);
 if(status==RR_ADVANCE_INVALID)return RR_NAME_INVALID;if(status!=RR_ADVANCE_READY||!scratch->count)return RR_NAME_NEEDS_FLOW;
 RRResultsNameStatus ready=pass(scratch,width,f,nf,NULL);if(ready!=RR_NAME_READY)return ready;
 memset(out,0,sizeof(*out));return pass(scratch,width,f,nf,out);
}
'''
for name,s in [('rr_results_name_flow.h',header),('rr_results_name_flow.c',source)]:
 f=p/'src/presentation'/name;assert not f.exists();f.write_text(s,encoding='utf-8')
analysis=json.loads((Path(__file__).parent/'results-name-flow-analysis.json').read_text(encoding='utf-8'));assert analysis['summary']['mismatches']==0 and len(analysis['cases'])==570
cases=analysis['cases']
source_observations=json.loads(Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1\observations.json').read_text(encoding='utf-8'))
source_rows={(o['fixture'],o['scale'],o['offset'],name,row['index']):row for o in source_observations for name,l in o['lists'].items() for row in l['rows']}
def c(s):return '"'+''.join(chr(x) if 32<=x<127 and chr(x) not in '\\"' else '\\%03o'%x for x in s.encode('utf-8'))+'"'
fixture='''/* Prior measured value-float states are inputs; name lines/li height
 * are independent observed outputs. Not a current-value float placer. */
typedef struct NameFlowCase {const char *text;double width;size_t float_count;RRResultsRightFloat floats[11];size_t count;RRResultsNameLine lines[128];double height;} NameFlowCase;
static const NameFlowCase name_flow_cases[]={
'''
for x in cases:
 floats=','.join('{'+','.join(repr(v) for v in f)+'}' for f in x['floats']) or '{0}'
 row=source_rows[(x['fixture'],x['scale'],x['offset'],x['list'],x['row'])];observed_lines=[]
 chars=row['directTextNodes'][0]['characterRectsViewport']
 for rect in x['expected']:
  absolute_y=row['viewport']['y']+rect[1]
  indices=[i for i,char in enumerate(chars) if any(r['width']>0 and r['y']==absolute_y for r in char['rects'])]
  assert indices
  observed_lines.append([min(indices),max(indices)+1,*rect])
 x['observed_codepoint_lines']=observed_lines
 lines=','.join('{%d,%d,%s,%s,%s,%s}'%(v[0],v[1]-v[0],repr(v[2]),repr(v[3]),repr(v[4]),repr(v[5])) for v in observed_lines)
 assert [tuple(v[2:]) for v in x['predicted']]==[tuple(v) for v in x['expected']]
 fixture+='{%s,%s,%d,{%s},%d,{%s},%s},\n'%(c(x['text']),repr(x['width']),len(x['floats']),floats,len(x['predicted']),lines,repr(x['expected_height']))
fixture+='};\n';(p/'tests/results_name_flow_reference.h').write_text(fixture,encoding='ascii')
test=r'''#include "rr_results_name_flow.h"
#include "results_name_flow_reference.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,lines,leading,multiline;static RRResultsCharacters scratch;static RRResultsNameFlow actual,guard,saved;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"name flow line %d: %s\n",__LINE__,#x);failures++;}}while(0)
int main(void){
 for(size_t i=0;i<COUNT(name_flow_cases);i++){
  const NameFlowCase *c=name_flow_cases+i;CHECK(rr_results_name_flow(c->text,c->width,c->floats,c->float_count,&scratch,&actual)==RR_NAME_READY);CHECK(actual.count==c->count&&actual.height==c->height);
  for(size_t j=0;j<actual.count&&j<c->count;j++){const RRResultsNameLine *a=actual.lines+j,*b=c->lines+j;CHECK(a->first==b->first&&a->count==b->count&&a->x==b->x&&a->y==b->y&&a->width==b->width&&a->height==b->height);lines++;}
  leading+=actual.lines[0].y>1;multiline+=actual.count>1;
 }
 memset(&guard,0x5a,sizeof(guard));saved=guard;
 CHECK(rr_results_name_flow("AWX",300,NULL,0,&scratch,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("WWWW",1,NULL,0,&scratch,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("W-0",1,NULL,0,&scratch,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("",300,NULL,0,&scratch,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("^",300,NULL,0,&scratch,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("ACE",NAN,NULL,0,&scratch,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("ACE",300,NULL,1,&scratch,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 RRResultsRightFloat f={100,0,200,20};CHECK(rr_results_name_flow("ACE",300,&f,1,&scratch,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 f.height=21.296875;f.y=.5;CHECK(rr_results_name_flow("ACE",300,&f,1,&scratch,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 f.y=NAN;CHECK(rr_results_name_flow("ACE",300,&f,1,&scratch,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow(NULL,300,NULL,0,&scratch,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_name_flow("ACE",300,NULL,0,NULL,&guard)==RR_NAME_INVALID&&rr_results_name_flow("ACE",300,NULL,0,&scratch,NULL)==RR_NAME_INVALID);
 printf("RESULTS_NAME_FLOW_SUMMARY observed_rows=%zu lines=%u leading_clearance=%u multiline=%u output_bytes=%zu scratch_bytes=%zu failures=%u prior_float_states_are_inputs=true current_value_placement_not_implemented=true\n",COUNT(name_flow_cases),lines,leading,multiline,sizeof(actual),sizeof(scratch),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
'''
f=p/'tests/test_results_name_flow.c';assert not f.exists();f.write_text(test,encoding='utf-8')
dest=p/'reference/ui-parity-20261103/results-name-flow-capture';dest.mkdir(exist_ok=False);(dest/'reference.json').write_text(json.dumps(analysis,indent=2)+'\n',encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text().replace('add_library(rr_core STATIC ','add_library(rr_core STATIC src/presentation/rr_results_name_flow.c ');marker='    add_executable(rr_results_characters_tests ';assert s.count(marker)==1
block='''    add_executable(rr_results_name_flow_tests tests/test_results_name_flow.c)
    target_include_directories(rr_results_name_flow_tests PRIVATE src/presentation)
    target_link_libraries(rr_results_name_flow_tests PRIVATE rr_core)
    if(MSVC)
        target_compile_options(rr_results_name_flow_tests PRIVATE /W4 /WX)
    else()
        target_compile_options(rr_results_name_flow_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME browser_results_name_flow_exclusions COMMAND rr_results_name_flow_tests)
''';f.write_text(s.replace(marker,block+marker))
print(json.dumps(dict(observed_rows=570,lines=591,leading_clearance=180,multiline_rows=21,current_float_placement=False)))

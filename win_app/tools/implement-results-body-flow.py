from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');b=p/'reference/pre-results-body-flow-20261104';b.mkdir(exist_ok=False)
for name in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 q=b/name;q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/name,q)
header=r'''#ifndef RR_RESULTS_BODY_FLOW_H
#define RR_RESULTS_BODY_FLOW_H
#include "rr_results_list_flow.h"
#include "rr_results_card.h"
/* Bounded source desktop profile. Width/height are integer logical CSS pixels;
 * qualified track widths and positive single-block caption height remain inputs.
 * Other content supplies measured intrinsic slot heights/visibility; COLUMNS is
 * overwritten from generated list flow, never consumed from that input. No font
 * shaping of labels/caption, track auto-minimum sizing or account calculation. */
typedef struct RRResultsBodyInput {
 double width,height,scroll,left_width,right_width,caption_height;
 const RRResultsFlowText *standings,*coins;size_t standings_count,coin_count;
 RRResultsCardContent other;
} RRResultsBodyInput;
typedef struct RRResultsBodyLayout {
 RRResultsCardLayout card;RRResultsGridMetrics metrics;
 RRResultsListSummary lists[2];RRUIRect labels[2],list_boxes[2],caption;
} RRResultsBodyLayout;
/* Fixed source label19, coin top margin6, caption top margin8, list indent26.
 * Parent natural heights include protruding floats independently of ordinary
 * list height. Native viewport boxes are float conversions of logical geometry.
 * Source-compatible floor1/64 centering and integer scrollHeight rounding are
 * separate from retained legacy card API. Scroll input is already logical CSS
 * pixels, not OS wheel/DPI policy. Whole output unchanged on failure; caller
 * work may change. No heap/large local scratch, sink/paint/main activation. */
RRResultsNameStatus rr_results_body_flow(const RRResultsBodyInput *input,
 RRResultsListWork *work,RRResultsBodyLayout *out);
#endif
'''
source=r'''#include "rr_results_body_flow.h"
#include <math.h>
#include <float.h>
static void shift(RRUIRect *r,double y){r->y=(float)((double)r->y+y);}
RRResultsNameStatus rr_results_body_flow(const RRResultsBodyInput *i,RRResultsListWork *w,RRResultsBodyLayout *out)
{
 if(!i||!w||!out||!isfinite(i->width)||!isfinite(i->height)||i->width<=0||i->height<=0||i->width>FLT_MAX||i->height>FLT_MAX||!isfinite(i->scroll)||!isfinite(i->left_width)||!isfinite(i->right_width)||!isfinite(i->caption_height)||i->left_width<=26||i->right_width<=26||i->caption_height<=0||i->caption_height>FLT_MAX)return RR_NAME_INVALID;
 if(floor(i->width)!=i->width||floor(i->height)!=i->height||!i->coin_count)return RR_NAME_NEEDS_FLOW;
 RRResultsBodyLayout value={0};RRResultsNameStatus status=rr_results_list_flow(RR_FLOW_STANDINGS,i->standings,i->standings_count,i->left_width-26,w,NULL,NULL,&value.lists[0]);if(status!=RR_NAME_READY)return status;
 status=rr_results_list_flow(RR_FLOW_COINS,i->coins,i->coin_count,i->right_width-26,w,NULL,NULL,&value.lists[1]);if(status!=RR_NAME_READY)return status;
 double bank_y=25+value.lists[1].height+8;
 value.metrics=(RRResultsGridMetrics){i->left_width,i->right_width,19+value.lists[0].height,fmax(bank_y+i->caption_height,25+value.lists[1].float_bottom)};
 RRResultsCardContent content=i->other;content.heights[RR_CARD_COLUMNS]=fmax(value.metrics.left_natural,value.metrics.right_natural);
 if(!rr_results_card_flow_intrinsic(i->width,i->height,&content,&value.metrics,0,&value.card))return RR_NAME_INVALID;
 /* Chromium's measured integer scrollHeight and 1/64 flex positioning. Keep
  * the legacy continuous card solver intact for its existing callers. */
 value.card.max_scroll=fmax(0,floor(value.card.content_height+32+.5)-i->height);
 value.card.scroll=fmin(value.card.max_scroll,fmax(0,i->scroll));
 double top=floor(fmax(16,(i->height-value.card.content_height)*.5)*64)/64-value.card.scroll;
 double delta=top-value.card.panel.y;if(!isfinite(top)||fabs(top)>FLT_MAX)return RR_NAME_INVALID;
 shift(&value.card.panel,delta);shift(&value.card.left,delta);shift(&value.card.right,delta);
 for(int slot=0;slot<RR_CARD_SLOTS;slot++)if(value.card.visible&(1u<<slot))shift(value.card.boxes+slot,delta);
 value.labels[0]=(RRUIRect){value.card.left.x,value.card.left.y,value.card.left.w,19};
 value.labels[1]=(RRUIRect){value.card.right.x,value.card.right.y,value.card.right.w,19};
 value.list_boxes[0]=(RRUIRect){value.card.left.x,(float)(value.card.left.y+19),value.card.left.w,(float)value.lists[0].height};
 value.list_boxes[1]=(RRUIRect){value.card.right.x,(float)(value.card.right.y+25),value.card.right.w,(float)value.lists[1].height};
 value.caption=(RRUIRect){value.card.right.x,(float)(value.card.right.y+bank_y),value.card.right.w,(float)i->caption_height};
 *out=value;return RR_NAME_READY;
}
'''
for name,text in [('src/presentation/rr_results_body_flow.h',header),('src/presentation/rr_results_body_flow.c',source)]:
 f=p/name;assert not f.exists();f.write_text(text,encoding='utf-8')
pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1')
gold=json.loads((pack/'native-layout-goldens.json').read_text(encoding='utf-8'));obs=json.loads((pack/'observations.json').read_text(encoding='utf-8'));gl={(o['fixture'],o['scale'],o['offset']):o for o in gold};cases=[]
for o in obs:
 g=gl[(o['fixture'],o['scale'],o['offset'])];caption=o['columns'][1]['childBoxes'][2]['viewport']['height'];multi=len(o['input']['racers'])>1
 # The supplied source account profile has empty online/unlock/next blocks.
 # Fixed header/button heights are existing qualified source profiles, not
 # card-minus-grid-derived geometry; each entire output is tested below.
 heights=[32.296875,33 if multi else 0,40,20,0,0,0,0,61,61]
 assert g['cardHeight']==48+sum(heights)+63+g['grid']['height']
 def rect(b):return [b['x'],b['y'],b['width'],b['height']]
 cases.append(dict(fixture=o['fixture'],scale=o['scale'],offset=o['offset'],width=o['viewport']['width'],height=o['viewport']['height'],scroll=g['scroll']['top'],tracks=g['columnWidths'],caption_height=caption,heights=heights,rows=[[dict(name=r['name'],value=r['value'])for r in g['lists'][kind]['rows']]for kind in ['res-standings','res-coins']],expected=dict(panel=rect(g['cardViewport']),grid=rect(o['grid']['viewport']),columns=[rect(c['viewport'])for c in o['columns']],labels=[rect(c['childBoxes'][0]['viewport'])for c in o['columns']],lists=[rect(o['lists'][kind]['viewport'])for kind in ['res-standings','res-coins']],caption=rect(o['columns'][1]['childBoxes'][2]['viewport']),natural=g['naturalColumnHeights'],max_scroll=g['scroll']['max'])))
assert len(cases)==60
def cs(s):return '"'+''.join(chr(x)if 32<=x<127 and chr(x)not in '\\"'else '\\%03o'%x for x in s.encode('utf-8'))+'"'
def nums(v):return '{'+','.join(repr(x)for x in v)+'}'
fixture='''/* Source boxes are expected outputs, not supplied row/list/card heights. */
typedef struct BodyCase {double width,height,scroll,left,right,caption_height;RRResultsCardContent other;size_t counts[2];RRResultsFlowText rows[2][11];RRUIRect panel,grid,columns[2],labels[2],lists[2],caption;double natural[2],max_scroll;} BodyCase;
static const BodyCase body_cases[]={
'''
for c in cases:
 e=c['expected'];rs=['{'+','.join('{'+cs(r['name'])+','+cs(r['value'])+'}'for r in rows)+'}'if rows else '{{0}}'for rows in c['rows']]
 fixture+='{%s,%s,%s,%s,%s,%s,{%s,RR_CARD_REQUIRED},%s,{%s},%s,%s,{%s},{%s},{%s},%s,%s,%s},\n'%(c['width'],c['height'],c['scroll'],*c['tracks'],c['caption_height'],nums(c['heights']),nums([len(x)for x in c['rows']]),','.join(rs),nums(e['panel']),nums(e['grid']),','.join(nums(x)for x in e['columns']),','.join(nums(x)for x in e['labels']),','.join(nums(x)for x in e['lists']),nums(e['caption']),nums(e['natural']),e['max_scroll'])
fixture+='};\n';(p/'tests/results_body_flow_reference.h').write_text(fixture,encoding='ascii')
test=r'''#include "rr_results_body_flow.h"
#include "results_body_flow_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,rows,scrolled,overflow;static RRResultsListWork work;static RRResultsBodyLayout out,guard,saved;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"body flow line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static void same(RRUIRect a,RRUIRect b){CHECK(a.x==b.x&&a.y==b.y&&a.w==b.w&&a.h==b.h);}
static RRResultsBodyInput input(const BodyCase *c){RRResultsBodyInput i={c->width,c->height,c->scroll,c->left,c->right,c->caption_height,c->rows[0],c->rows[1],c->counts[0],c->counts[1],c->other};return i;}
int main(void){
 for(size_t n=0;n<COUNT(body_cases);n++){
  const BodyCase *c=body_cases+n;RRResultsBodyInput i=input(c);i.other.heights[RR_CARD_COLUMNS]=NAN;
  CHECK(rr_results_body_flow(&i,&work,&out)==RR_NAME_READY);same(out.card.panel,c->panel);same(out.card.boxes[RR_CARD_COLUMNS],c->grid);same(out.card.left,c->columns[0]);same(out.card.right,c->columns[1]);same(out.caption,c->caption);
  for(size_t j=0;j<2;j++){same(out.labels[j],c->labels[j]);same(out.list_boxes[j],c->lists[j]);rows+=(unsigned)c->counts[j];}
  CHECK(out.metrics.left_natural==c->natural[0]&&out.metrics.right_natural==c->natural[1]&&out.card.max_scroll==c->max_scroll&&out.card.scroll==c->scroll&&out.card.content_height==c->panel.h);scrolled+=c->scroll>0;overflow+=c->left+22+c->right>616;
  /* Same frozen content: resize clamps previous scroll, negative request clamps
   * to zero, oversized request reaches integer source max without shrinking. */
  i.scroll=-100;CHECK(rr_results_body_flow(&i,&work,&out)==RR_NAME_READY&&out.card.scroll==0);
  i.scroll=1e9;CHECK(rr_results_body_flow(&i,&work,&out)==RR_NAME_READY&&out.card.scroll==out.card.max_scroll);
  i.height=2000;i.scroll=c->max_scroll;CHECK(rr_results_body_flow(&i,&work,&out)==RR_NAME_READY&&out.card.scroll==0);
 }
 RRResultsBodyInput i=input(body_cases);memset(&guard,0x5a,sizeof(guard));saved=guard;
 const double invalid[]={0,-1,NAN,INFINITY};for(size_t n=0;n<COUNT(invalid);n++){i.caption_height=invalid[n];CHECK(rr_results_body_flow(&i,&work,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));}i=input(body_cases);
 i.width=.5;CHECK(rr_results_body_flow(&i,&work,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));i=input(body_cases);i.coin_count=0;CHECK(rr_results_body_flow(&i,&work,&guard)==RR_NAME_NEEDS_FLOW&&!memcmp(&guard,&saved,sizeof(guard)));
 i=input(body_cases);RRResultsFlowText bad[]={ {"YOU","^"} };i.coins=bad;i.coin_count=1;CHECK(rr_results_body_flow(&i,&work,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 i=input(body_cases);i.other.visible=0;CHECK(rr_results_body_flow(&i,&work,&guard)==RR_NAME_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_body_flow(NULL,&work,&guard)==RR_NAME_INVALID&&rr_results_body_flow(&i,NULL,&guard)==RR_NAME_INVALID&&rr_results_body_flow(&i,&work,NULL)==RR_NAME_INVALID);
 printf("RESULTS_BODY_FLOW_SUMMARY observed_cards=%zu rows=%u bottom_scroll_observations=%u overflow_tracks=%u work_bytes=%zu output_bytes=%zu failures=%u caption_and_tracks_qualified_inputs=true native_main_unchanged=true\n",COUNT(body_cases),rows,scrolled,overflow,sizeof(work),sizeof(out),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
'''
(p/'tests/test_results_body_flow.c').write_text(test,encoding='utf-8')
dest=p/'reference/ui-parity-20261103/results-body-flow-capture';dest.mkdir(exist_ok=False);(dest/'reference.json').write_text(json.dumps(dict(package=str(pack),cases=cases,inputs='Frozen ordered strings, measured track widths and caption height, qualified other-slot profile and viewport/scroll. No measured list height, natural column height, grid height or card height enters runtime.'),indent=2)+'\n',encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text().replace('add_library(rr_core STATIC ','add_library(rr_core STATIC src/presentation/rr_results_body_flow.c ');marker='    add_executable(rr_results_list_flow_tests ';assert s.count(marker)==1
block='''    add_executable(rr_results_body_flow_tests tests/test_results_body_flow.c)
    target_include_directories(rr_results_body_flow_tests PRIVATE src/presentation)
    target_link_libraries(rr_results_body_flow_tests PRIVATE rr_core)
    if(MSVC)
        target_compile_options(rr_results_body_flow_tests PRIVATE /W4 /WX)
    else()
        target_compile_options(rr_results_body_flow_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME browser_results_generated_body_card_flow COMMAND rr_results_body_flow_tests)
''';f.write_text(s.replace(marker,block+marker))
print(json.dumps(dict(cards=60,rows=sum(len(rs)for c in cases for rs in c['rows']),generated_list_column_grid_card_heights=True)))

from pathlib import Path
import shutil,json
p=Path(r'C:\Users\User\Projects\neon_c\win_app');b=p/'reference/pre-results-grid-20261104';b.mkdir(exist_ok=False)
for f in ['src/presentation/rr_results_card.h','CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 q=b/f;q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/f,q)
header=r'''#ifndef RR_RESULTS_GRID_H
#define RR_RESULTS_GRID_H
#include "rr_ui_layout.h"
#include <float.h>
/* QUALIFIED intrinsic tracks/natural column heights. Browser auto-minimum grid
 * sizing and text/float layout must supply these; do not infer them from fixed
 * fractions, line counts or the presence of a relocated float. No font shaping. */
typedef struct RRResultsGridMetrics {double left_width,right_width,left_natural,right_natural;} RRResultsGridMetrics;
typedef struct RRResultsGridLayout {RRUIRect box,left,right;double overflow_x;} RRResultsGridLayout;
/* Source22px gap, max natural height and stretch of both columns. Preserve
 * widths extending beyond the fixed grid box rather than shrinking/dropping.
 * Invalid inputs/derived overflow leave out unchanged. No heap/draw/culling. */
static inline bool rr_results_grid_layout(RRUIRect anchor,const RRResultsGridMetrics *m,RRResultsGridLayout *out)
{
 if(!m||!out||!isfinite(anchor.x)||!isfinite(anchor.y)||!isfinite(anchor.w)||anchor.w<=0||
    !isfinite(m->left_width)||!isfinite(m->right_width)||!isfinite(m->left_natural)||!isfinite(m->right_natural)||
    m->left_width<=0||m->right_width<=0||m->left_natural<0||m->right_natural<0)return false;
 double h=fmax(m->left_natural,m->right_natural),right=(double)anchor.x+m->left_width+22;
 double end=right+m->right_width,overflow=fmax(0,m->left_width+22+m->right_width-anchor.w);
 if(h>FLT_MAX||m->left_width>FLT_MAX||m->right_width>FLT_MAX||!isfinite(end)||fabs(right)>FLT_MAX||fabs(end)>FLT_MAX||!isfinite(overflow)||overflow>FLT_MAX)return false;
 RRResultsGridLayout g={0};g.box=(RRUIRect){anchor.x,anchor.y,anchor.w,(float)h};g.left=(RRUIRect){anchor.x,anchor.y,(float)m->left_width,(float)h};g.right=(RRUIRect){(float)right,anchor.y,(float)m->right_width,(float)h};g.overflow_x=overflow;*out=g;return true;
}
#endif
'''
f=p/'src/presentation/rr_results_grid.h';assert not f.exists();f.write_text(header,encoding='utf-8')
f=p/'src/presentation/rr_results_card.h';s=f.read_text().replace('#include "rr_ui_layout.h"','#include "rr_ui_layout.h"\n#include "rr_results_grid.h"')
old='static inline bool rr_results_card_flow(double width,double height,const RRResultsCardContent *content,double requested_scroll,RRResultsCardLayout *out)'
new='static inline bool rr_results_card_flow_intrinsic(double width,double height,const RRResultsCardContent *content,const RRResultsGridMetrics *metrics,double requested_scroll,RRResultsCardLayout *out)'
assert s.count(old)==1;s=s.replace(old,new).replace('if(!content||!out||!isfinite(width)','if(!content||!metrics||!out||!isfinite(width)')
oldblock=''' value.left=(RRUIRect){columns.x,columns.y,335.734375f,columns.h};
 value.right=(RRUIRect){columns.x+335.734375f+22,columns.y,258.25f,columns.h};'''
newblock=''' RRResultsGridLayout grid;
 if(!rr_results_grid_layout(columns,metrics,&grid)||fmax(metrics->left_natural,metrics->right_natural)!=content->heights[RR_CARD_COLUMNS])return false;
 value.left=grid.left;value.right=grid.right;'''
assert oldblock in s;s=s.replace(oldblock,newblock)
s=s.replace('#endif',r'''/* Compatibility path is ONLY the retained nominal/legacy track profile. Source
 * auto-minimum tracks are not universally335.734375/258.25; use the intrinsic
 * API with qualified metrics for other content. Existing solo profile unchanged.
 * Source scrollbar quantization and actual input/pixels remain unqualified. */
static inline bool rr_results_card_flow(double width,double height,const RRResultsCardContent *content,double requested_scroll,RRResultsCardLayout *out)
{
 if(!content)return false;
 RRResultsGridMetrics m={335.734375,258.25,content->heights[RR_CARD_COLUMNS],content->heights[RR_CARD_COLUMNS]};
 return rr_results_card_flow_intrinsic(width,height,content,&m,requested_scroll,out);
}
#endif''');f.write_text(s)
pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1');obs=json.loads((pack/'observations.json').read_text(encoding='utf-8'));gold=json.loads((pack/'native-layout-goldens.json').read_text(encoding='utf-8'));assert len(obs)==len(gold)==60
def rect(r):return '{'+','.join(repr(r[k])+'f' if isinstance(r[k],float) else str(r[k])+'.0f' for k in ['x','y','width','height'])+'}'
lines=['''/* Verified browser wrapping package: measured track inputs, independently
 * observed stretched columns/grid height. No native pixels/shaping claimed. */
typedef struct GridCase {RRUIRect anchor;RRResultsGridMetrics metrics;RRUIRect grid,left,right;} GridCase;
static const GridCase grid_cases[]={''']
for o,g in zip(obs,gold):
 assert (o['fixture'],o['scale'],o['offset'])==(g['fixture'],g['scale'],g['offset'])
 cols=o['columns'];m=g['columnWidths']+g['naturalColumnHeights'];lines.append('{%s,{%s},%s,%s,%s},'%(rect(g['grid']),','.join(repr(v) for v in m),rect(g['grid']),rect(cols[0]['cardRelative']),rect(cols[1]['cardRelative'])))
lines.append('};\n');(p/'tests/results_grid_reference.h').write_text('\n'.join(lines),encoding='ascii')
test=r'''#include "rr_results_card.h"
#include "results_grid_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,expanded,integration;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"grid line %d: %s\n",__LINE__,#x);failures++;}}while(0)
static bool near(double a,double b){return fabs(a-b)<.002;}
static void same(RRUIRect a,RRUIRect b){CHECK(near(a.x,b.x)&&near(a.y,b.y)&&near(a.w,b.w)&&near(a.h,b.h));}
int main(void){
 for(size_t i=0;i<COUNT(grid_cases);i++){
  const GridCase *c=grid_cases+i;RRResultsGridLayout g;CHECK(rr_results_grid_layout(c->anchor,&c->metrics,&g));same(g.box,c->grid);same(g.left,c->left);same(g.right,c->right);
  CHECK(near(g.box.h,fmax(c->metrics.left_natural,c->metrics.right_natural))&&near(g.overflow_x,fmax(0,c->metrics.left_width+22+c->metrics.right_width-c->anchor.w)));expanded+=g.overflow_x>0;
  /* Derived integration uses the existing native content profile with measured
   * grid heights/tracks substituted, not the package's different frozen header. */
  RRResultsCardContent content=rr_results_card_legacy();content.heights[RR_CARD_COLUMNS]=fmax(c->metrics.left_natural,c->metrics.right_natural);
  RRResultsCardLayout card;CHECK(rr_results_card_flow_intrinsic(960,480,&content,&c->metrics,100,&card));
  CHECK(near(card.left.w,c->metrics.left_width)&&near(card.right.w,c->metrics.right_width)&&near(card.left.h,g.box.h)&&near(card.right.h,g.box.h)&&near(card.right.x-card.left.x,c->metrics.left_width+22));integration++;
 }
 RRResultsCardContent content=rr_results_card_legacy();RRResultsCardLayout old,now;RRResultsGridMetrics metrics={335.734375,258.25,content.heights[RR_CARD_COLUMNS],content.heights[RR_CARD_COLUMNS]};
 CHECK(rr_results_card_flow(1600,1000,&content,0,&old)&&rr_results_card_flow_intrinsic(1600,1000,&content,&metrics,0,&now)&&!memcmp(&old,&now,sizeof(old)));
 RRResultsGridLayout guard,saved;memset(&guard,0x5a,sizeof(guard));saved=guard;RRUIRect anchor={32,100,616,0};RRResultsGridMetrics m={300,300,20,40};
 double bad[]={NAN,INFINITY,-1};for(size_t i=0;i<COUNT(bad);i++){m.left_width=bad[i];CHECK(!rr_results_grid_layout(anchor,&m,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));m.left_width=300;m.right_natural=bad[i];CHECK(!rr_results_grid_layout(anchor,&m,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));m.right_natural=40;}
 m.left_width=0;CHECK(!rr_results_grid_layout(anchor,&m,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));m.left_width=300;
 volatile float huge=FLT_MAX;anchor.x=huge;m.left_width=huge;CHECK(!rr_results_grid_layout(anchor,&m,&guard)&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(!rr_results_grid_layout((RRUIRect){0,0,616,0},NULL,&guard)&&!rr_results_grid_layout((RRUIRect){0,0,616,0},&m,NULL));
 memset(&now,0x5a,sizeof(now));RRResultsCardLayout saved_card=now;metrics.left_natural=999;CHECK(!rr_results_card_flow_intrinsic(1600,1000,&content,&metrics,0,&now)&&!memcmp(&now,&saved_card,sizeof(now)));
 printf("RESULTS_GRID_SUMMARY observed=%zu expanded_observations=%u derived_integrations=%u metrics_bytes=%zu grid_bytes=%zu failures=%u measured_tracks_not_auto_min_solver=true no_float_or_native_pixel_claim=true\n",COUNT(grid_cases),expanded,integration,sizeof(metrics),sizeof(guard),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
'''
f=p/'tests/test_results_grid.c';assert not f.exists();f.write_text(test,encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text();marker='    add_executable(rr_audio_cue_tests ';assert s.count(marker)==1
block='''    add_executable(rr_results_grid_tests tests/test_results_grid.c)
    target_include_directories(rr_results_grid_tests PRIVATE src/presentation)
    target_link_libraries(rr_results_grid_tests PRIVATE rr_core)
    if(MSVC)
        target_compile_options(rr_results_grid_tests PRIVATE /W4 /WX)
    else()
        target_compile_options(rr_results_grid_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME browser_results_measured_intrinsic_grid COMMAND rr_results_grid_tests)
''';f.write_text(s.replace(marker,block+marker))
print('Integrated measured intrinsic grid metrics and60 source observations. Existing nominal profile preserved; no shaping/float/main activation.')

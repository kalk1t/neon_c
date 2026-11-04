from pathlib import Path
import sys,json,hashlib,shutil
sys.path.insert(0,str(Path(__file__).parent/'fonttools'))
from fontTools.ttLib import TTFont
p=Path(r'C:\Users\User\Projects\neon_c\win_app');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1');b=p/'reference/pre-results-advance-20261104';b.mkdir(exist_ok=False)
for f in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 q=b/f;q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/f,q)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
fontpath=p/'assets/ui-theme/Orbitron-700.ttf';font=TTFont(fontpath);cmap=font.getBestCmap();assert font['head'].unitsPerEm==1000 and not font['GSUB'].table.FeatureList.FeatureRecord
units={cp:font['hmtx'][name][0] for cp,name in cmap.items()};chars=set(range(32,127))|{8212};supported={cp for cp in chars if cp in cmap};assert chars-supported=={94}
reverse={g:cp for cp,g in cmap.items()};kern=[]
for lookup in font['GPOS'].table.LookupList.Lookup:
 if lookup.LookupType!=2:continue
 for sub in lookup.SubTable:
  assert sub.Format==1 and sub.ValueFormat1==4 and sub.ValueFormat2==0
  for left,pairs in zip(sub.Coverage.glyphs,sub.PairSet):
   for pair in pairs.PairValueRecord:
    if left in reverse and pair.SecondGlyph in reverse and reverse[left] in supported and reverse[pair.SecondGlyph] in supported and pair.Value1.XAdvance:
     kern.append((reverse[left],reverse[pair.SecondGlyph],pair.Value1.XAdvance))
kern=sorted(set(kern))
data='/* Derived numeric metrics; existing font/OFL unchanged. Primary15px/em1000 =>960 ticks/unit at64000 ticks/CSSpx. */\n'
data+='static const struct {uint32_t cp;uint32_t ticks;} rr_results_advances[]={\n'+''.join('{%du,%du},\n'%(cp,units[cp]*960) for cp in sorted(supported))+'{8776u,634000u},{9819u,960000u}\n};\n'
data+='static const struct {uint32_t left,right;} rr_results_nonzero_pairs[]={\n'+''.join('{%du,%du},\n'%(a,b) for a,b,k in kern)+'};\n'
header=r'''#ifndef RR_RESULTS_ADVANCE_H
#define RR_RESULTS_ADVANCE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef enum RRResultsAdvanceStatus {RR_ADVANCE_INVALID,RR_ADVANCE_NEEDS_SHAPING,RR_ADVANCE_READY} RRResultsAdvanceStatus;
typedef struct RRResultsAdvance {uint64_t ticks;size_t codepoints;double advance,range_width;} RRResultsAdvance;
/* Bound to verified15CSSpx/700 Orbitron metrics and existing approx/queen
 * fallback advances. Plain supported unkerned single-line runs only: printable
 * primary cmap excluding missing caret, plus em dash/approx/queen. Nonzero GPOS
 * pairs or leading/trailing/repeated spaces return NEEDS_SHAPING, never a guess.
 * Max128 codepoints/<512UTF8bytes; strict decoding, no '?' substitution. Exact
 * integer ticks use1/64000CSSpx; range_width is outward1/64 width envelope.
 * This is NOT per-character shaping, a float-fit epsilon, min-content/grid sizing,
 * line breaking or a new glyph painter. Failure leaves out untouched. No heap. */
RRResultsAdvanceStatus rr_results_advance(const char *text,RRResultsAdvance *out);
#endif
'''
source=r'''#include "rr_results_advance.h"
#include <string.h>
#include "rr_results_advance_data.h"
static bool decode(const unsigned char *s,size_t n,size_t *used,uint32_t *out)
{
 uint32_t c=s[0];if(c<128){*out=c;*used=1;return true;}
 unsigned count=c>=194&&c<=223?2:c>=224&&c<=239?3:c>=240&&c<=244?4:0;if(!count||count>n)return false;
 c&=(1u<<(7-count))-1u;for(unsigned i=1;i<count;i++){if((s[i]&192)!=128)return false;c=(c<<6)|(s[i]&63);}
 if((count==2&&c<128)||(count==3&&c<2048)||(count==4&&c<65536)||c>1114111||(c>=55296&&c<=57343))return false;
 *out=c;*used=count;return true;
}
RRResultsAdvanceStatus rr_results_advance(const char *text,RRResultsAdvance *out)
{
 if(!text||!out)return RR_ADVANCE_INVALID;size_t bytes=0;while(bytes<512&&text[bytes])bytes++;if(bytes==512)return RR_ADVANCE_INVALID;
 RRResultsAdvance value={0};uint32_t previous=0;bool complex=false;
 for(size_t offset=0;offset<bytes;){
  uint32_t cp=0,ticks=0;size_t used=0;if(value.codepoints==128||!decode((const unsigned char*)text+offset,bytes-offset,&used,&cp))return RR_ADVANCE_INVALID;offset+=used;
  bool found=false;for(size_t i=0;i<sizeof(rr_results_advances)/sizeof(rr_results_advances[0]);i++)if(rr_results_advances[i].cp==cp){found=true;ticks=rr_results_advances[i].ticks;break;}
  if(!found)return RR_ADVANCE_INVALID;
  if(cp==32&&(!value.codepoints||previous==32||offset==bytes))complex=true;
  for(size_t i=0;i<sizeof(rr_results_nonzero_pairs)/sizeof(rr_results_nonzero_pairs[0]);i++)if(rr_results_nonzero_pairs[i].left==previous&&rr_results_nonzero_pairs[i].right==cp)complex=true;
  value.ticks+=ticks;value.codepoints++;previous=cp;
 }
 if(complex)return RR_ADVANCE_NEEDS_SHAPING;
 value.advance=(double)value.ticks/64000;value.range_width=(double)((value.ticks+999)/1000)/64;*out=value;return RR_ADVANCE_READY;
}
'''
for name,s in [('rr_results_advance.h',header),('rr_results_advance.c',source),('rr_results_advance_data.h',data)]:
 f=p/'src/presentation'/name;assert not f.exists();f.write_text(s,encoding='utf-8')
obs=json.loads((pack/'observations.json').read_text(encoding='utf-8'));cases=[];excluded=0
for o in obs:
 for listname,l in o['lists'].items():
  for row in l['rows']:
   for kind,nodes in [('name',row['directTextNodes']),('value',row['valueSpan']['textNodes'])]:
    for node in nodes:
     boxes=node['rangeClientRectsViewport']
     if len(boxes)!=1:excluded+=1;continue
     cases.append(dict(fixture=o['fixture'],scale=o['scale'],offset=o['offset'],list=listname,row=row['index'],kind=kind,text=node['text'],range_width=boxes[0]['width']))
assert len(cases)==1119
def c(s):return '"'+''.join(chr(x) if 32<=x<127 and chr(x) not in '\\"' else '\\%03o'%x for x in s.encode('utf-8'))+'"'
fixture='/* Actual source single-line Range widths, not native glyph positions. */\ntypedef struct AdvanceCase {const char *text;double width;} AdvanceCase;\nstatic const AdvanceCase advance_cases[]={\n'+''.join('{%s,%s},\n'%(c(x['text']),repr(x['range_width'])) for x in cases)+'};\n'
(p/'tests/results_advance_reference.h').write_text(fixture,encoding='ascii')
test=r'''#include "rr_results_advance.h"
#include "results_advance_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"advance line %d: %s\n",__LINE__,#x);failures++;}}while(0)
int main(void){
 RRResultsAdvance a;
 for(size_t i=0;i<COUNT(advance_cases);i++)CHECK(rr_results_advance(advance_cases[i].text,&a)==RR_ADVANCE_READY&&a.range_width==advance_cases[i].width);
 CHECK(rr_results_advance("WWWWWWWWWWWWWWWW",&a)==RR_ADVANCE_READY&&a.ticks==18109440&&a.range_width==282.96875&&a.range_width+26==308.96875);
 CHECK(rr_results_advance("",&a)==RR_ADVANCE_READY&&!a.codepoints&&!a.ticks&&!a.range_width);
 RRResultsAdvance guard,saved;memset(&guard,0x5a,sizeof(guard));saved=guard;
 const char *complex[]={"AW"," A","A ","A  A"};for(size_t i=0;i<COUNT(complex);i++)CHECK(rr_results_advance(complex[i],&guard)==RR_ADVANCE_NEEDS_SHAPING&&!memcmp(&guard,&saved,sizeof(guard)));
 const char *invalid[]={"^","\n","\xc0\xaf","\xed\xa0\x80","\xf0\x9f\x98\x80","\xe2\x89"};for(size_t i=0;i<COUNT(invalid);i++)CHECK(rr_results_advance(invalid[i],&guard)==RR_ADVANCE_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 char long_text[513];memset(long_text,'W',512);long_text[512]=0;CHECK(rr_results_advance(long_text,&guard)==RR_ADVANCE_INVALID);long_text[129]=0;CHECK(rr_results_advance(long_text,&guard)==RR_ADVANCE_INVALID);long_text[128]=0;CHECK(rr_results_advance(long_text,&a)==RR_ADVANCE_READY&&a.codepoints==128);
 CHECK(rr_results_advance(NULL,&guard)==RR_ADVANCE_INVALID&&rr_results_advance("A",NULL)==RR_ADVANCE_INVALID);
 printf("RESULTS_ADVANCE_SUMMARY observed_single_line_ranges=%zu exact_widths=%zu plan_bytes=%zu failures=%u no_character_shaping_or_float_fit_claim=true\n",COUNT(advance_cases),COUNT(advance_cases),sizeof(a),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
'''
f=p/'tests/test_results_advance.c';assert not f.exists();f.write_text(test,encoding='utf-8')
dest=p/'reference/ui-parity-20261103/results-advance-capture';dest.mkdir(exist_ok=False)
analysis=json.loads((Path(__file__).parent/'results-shaping-analysis.json').read_text(encoding='utf-8'))
proof=dict(font=dict(path='assets/ui-theme/Orbitron-700.ttf',sha256=sha(fontpath),units_per_em=1000,weight=700,css_size=15),tool='Existing FontTools4.61.1',package=str(pack),package_manifest_sha256=sha(pack/'manifest.json'),cases=cases,excluded_multiline_or_nonrect_nodes=excluded,primary_codepoints=len(supported),nonzero_pairs_rejected=kern,fallback_advances={'8776':9.90625,'9819':15},analysis=analysis,scope='Widths only. Exact unit advance/outward range envelope qualified against1119 single-line source observations. No nonzero kern pair is in this corpus; all such pairs explicitly reject.213 node character-position mismatches from the simple snapping hypothesis are retained, not accepted as shaping.')
(dest/'reference.json').write_text(json.dumps(proof,indent=2)+'\n',encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text().replace('add_library(rr_core STATIC ','add_library(rr_core STATIC src/presentation/rr_results_advance.c ');marker='    add_executable(rr_results_grid_tests ';assert s.count(marker)==1
block='''    add_executable(rr_results_advance_tests tests/test_results_advance.c)
    target_include_directories(rr_results_advance_tests PRIVATE src/presentation)
    target_link_libraries(rr_results_advance_tests PRIVATE rr_core)
    if(MSVC)
        target_compile_options(rr_results_advance_tests PRIVATE /W4 /WX)
    else()
        target_compile_options(rr_results_advance_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME browser_results_precise_advance_widths COMMAND rr_results_advance_tests)
''';f.write_text(s.replace(marker,block+marker))
print(json.dumps(dict(source_ranges=len(cases),excluded=excluded,primary_codepoints=len(supported),kerning_pairs_rejected=len(kern),width_only=True)))

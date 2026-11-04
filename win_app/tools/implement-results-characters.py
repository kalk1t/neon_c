from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1');b=p/'reference/pre-results-characters-20261104';b.mkdir(exist_ok=False)
for f in ['src/presentation/rr_results_advance.h','src/presentation/rr_results_advance.c','CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 q=b/f;q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/f,q)
f=p/'src/presentation/rr_results_advance.h';s=f.read_text();assert s.count('#endif')==1;s=s.replace('#endif',r'''typedef struct RRResultsCharacterRange {uint32_t codepoint;double pen,left,right;} RRResultsCharacterRange;
typedef struct RRResultsCharacters {RRResultsCharacterRange ranges[128];size_t count;double advance,range_width;} RRResultsCharacters;
/* Supported single-line15px/700 primary + queen character Range X envelopes.
 * Reuses all advance preflight/kerning/whitespace/UTF8 limits. Approx fallback
 * returns NEEDS_SHAPING: its captured range width is not an unrounded raw advance.
 * Float32 font scale/advance, double accumulation, outward1/64 Range bounds.
 * These are character selection envelopes, NOT glyph bitmap bounds/origins,
 * raster/shadow/vertical placement, float wrapping or painter activation.
 * Persistent caller output (~4KiB), no heap or large temporary. Immutable text
 * must not alias output or change during synchronous multi-pass validation.
 * Any supported-input failure leaves output untouched. */
RRResultsAdvanceStatus rr_results_characters(const char *text,RRResultsCharacters *out);
#endif''');f.write_text(s)
f=p/'src/presentation/rr_results_advance.c';s=f.read_text();s+=r'''
RRResultsAdvanceStatus rr_results_characters(const char *text,RRResultsCharacters *out)
{
 if(!out)return RR_ADVANCE_INVALID;
 RRResultsAdvance checked;RRResultsAdvanceStatus status=rr_results_advance(text,&checked);if(status!=RR_ADVANCE_READY)return status;
 size_t bytes=strlen(text);
 for(size_t offset=0;offset<bytes;){uint32_t cp=0;size_t used=0;(void)decode((const unsigned char*)text+offset,bytes-offset,&used,&cp);if(cp==8776)return RR_ADVANCE_NEEDS_SHAPING;offset+=used;}
 /* All failures are preflighted before writing. Source/raw text stays immutable. */
 memset(out,0,sizeof(*out));double pen=0;const float scale=15.0f/1000.0f;
 for(size_t offset=0;offset<bytes;){
  uint32_t cp=0,ticks=0;size_t used=0;(void)decode((const unsigned char*)text+offset,bytes-offset,&used,&cp);offset+=used;
  for(size_t i=0;i<sizeof(rr_results_advances)/sizeof(rr_results_advances[0]);i++)if(rr_results_advances[i].cp==cp){ticks=rr_results_advances[i].ticks;break;}
  float advance=cp==9819?15.0f:(float)(ticks/960)*scale;
  double end=pen+(double)advance;
  out->ranges[out->count++]=(RRResultsCharacterRange){cp,pen,floor(pen*64)/64,ceil(end*64)/64};pen=end;
 }
 out->advance=pen;out->range_width=ceil(pen*64)/64;return RR_ADVANCE_READY;
}
''';s=s.replace('#include <string.h>','#include <string.h>\n#include <math.h>');f.write_text(s)
obs=json.loads((pack/'observations.json').read_text(encoding='utf-8'));cases=[];approx=0;multi=0;characters=0
for o in obs:
 for name,l in o['lists'].items():
  for row in l['rows']:
   for kind,ns in [('name',row['directTextNodes']),('value',row['valueSpan']['textNodes'])]:
    for node in ns:
     if len(node['rangeClientRectsViewport'])!=1:multi+=1;continue
     if '\u2248' in node['text']:approx+=1;continue
     origin=node['rangeClientRectsViewport'][0]['x'];chars=[]
     for info in node['characterRectsViewport']:
      assert len(info['rects'])==1 and len(info['text'])==1
      r=info['rects'][0];chars.append(dict(cp=ord(info['text']),left=r['x']-origin,right=r['x']-origin+r['width']))
     assert len(chars)==len(node['text']);characters+=len(chars)
     cases.append(dict(fixture=o['fixture'],scale=o['scale'],offset=o['offset'],list=name,row=row['index'],kind=kind,text=node['text'],width=node['rangeClientRectsViewport'][0]['width'],chars=chars))
assert len(cases)==1068 and approx==51 and multi==21 and characters==8166
def c(s):return '"'+''.join(chr(x) if 32<=x<127 and chr(x) not in '\\"' else '\\%03o'%x for x in s.encode('utf-8'))+'"'
fixture='''/* Measured browser Range X geometry; no glyph bitmap/raster claim. */
typedef struct ExpectedCharacter {uint32_t cp;double left,right;} ExpectedCharacter;
typedef struct CharacterCase {const char *text;double width;size_t count;ExpectedCharacter chars[128];} CharacterCase;
static const CharacterCase character_cases[]={
'''
for x in cases:fixture+='{%s,%s,%d,{%s}},\n'%(c(x['text']),repr(x['width']),len(x['chars']),','.join('{%du,%s,%s}'%(y['cp'],repr(y['left']),repr(y['right'])) for y in x['chars']))
fixture+='};\n';(p/'tests/results_characters_reference.h').write_text(fixture,encoding='ascii')
test=r'''#include "rr_results_advance.h"
#include "results_characters_reference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned failures,total;
static RRResultsCharacters actual,guard,saved;
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"characters line %d: %s\n",__LINE__,#x);failures++;}}while(0)
int main(void){
 for(size_t i=0;i<COUNT(character_cases);i++){
  const CharacterCase *c=character_cases+i;CHECK(rr_results_characters(c->text,&actual)==RR_ADVANCE_READY&&actual.count==c->count&&actual.range_width==c->width);
  for(size_t j=0;j<actual.count&&j<c->count;j++){const RRResultsCharacterRange *r=actual.ranges+j;CHECK(r->codepoint==c->chars[j].cp&&r->left==c->chars[j].left&&r->right==c->chars[j].right);total++;}
 }
 memset(&guard,0x5a,sizeof(guard));saved=guard;
 const char *needs[]={"\342\211\210 02:30.00","AWX"," A","A ","A  A"};for(size_t i=0;i<COUNT(needs);i++)CHECK(rr_results_characters(needs[i],&guard)==RR_ADVANCE_NEEDS_SHAPING&&!memcmp(&guard,&saved,sizeof(guard)));
 const char *bad[]={"^","A\nB","\xc0\xaf","\xed\xa0\x80","\xe2\x89","\xf0\x9f\x98\x80"};for(size_t i=0;i<COUNT(bad);i++)CHECK(rr_results_characters(bad[i],&guard)==RR_ADVANCE_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));
 CHECK(rr_results_characters(NULL,&guard)==RR_ADVANCE_INVALID&&rr_results_characters("A",NULL)==RR_ADVANCE_INVALID);
 CHECK(rr_results_characters("",&actual)==RR_ADVANCE_READY&&!actual.count&&!actual.advance&&!actual.range_width);
 char long_text[130];memset(long_text,'W',129);long_text[129]=0;CHECK(rr_results_characters(long_text,&guard)==RR_ADVANCE_INVALID&&!memcmp(&guard,&saved,sizeof(guard)));long_text[128]=0;CHECK(rr_results_characters(long_text,&actual)==RR_ADVANCE_READY&&actual.count==128);
 printf("RESULTS_CHARACTERS_SUMMARY supported_nodes=%zu character_ranges=%u output_bytes=%zu failures=%u approx_fallback_unqualified=true no_raster_float_or_main_claim=true\n",COUNT(character_cases),total,sizeof(actual),failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
'''
f=p/'tests/test_results_characters.c';assert not f.exists();f.write_text(test,encoding='utf-8')
dest=p/'reference/ui-parity-20261103/results-characters-capture';dest.mkdir(exist_ok=False)
analysis=json.loads((Path(__file__).parent/'results-float32-analysis.json').read_text(encoding='utf-8'))
(dest/'reference.json').write_text(json.dumps(dict(package=str(pack),cases=cases,supported_nodes=1068,characters=8166,approx_excluded_nodes=51,multiline_excluded_nodes=21,hypotheses=analysis,selected_model='float32 scale first; each primary glyph advance rounded float32; accumulate in double; queen15; outward floor/ceil1/64; rejectapprox/kerning/whitespace contexts.',scope='Range X geometry only on observed supported single-line runs. No fresh browser/native render or font redistribution.'),indent=2)+'\n',encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text();marker='    add_executable(rr_results_advance_tests ';assert s.count(marker)==1
block='''    add_executable(rr_results_characters_tests tests/test_results_characters.c)
    target_include_directories(rr_results_characters_tests PRIVATE src/presentation)
    target_link_libraries(rr_results_characters_tests PRIVATE rr_core)
    if(MSVC)
        target_compile_options(rr_results_characters_tests PRIVATE /W4 /WX)
    else()
        target_compile_options(rr_results_characters_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME browser_results_character_range_positions COMMAND rr_results_characters_tests)
''';f.write_text(s.replace(marker,block+marker))
print(json.dumps(dict(supported_nodes=1068,characters=8166,approx_nodes_unqualified=51,multiline_nodes_unqualified=21,existing_width_api_preserved=True)))

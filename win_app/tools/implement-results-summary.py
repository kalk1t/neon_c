from pathlib import Path
import shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');b=p/'reference/pre-results-summary-20261104';b.mkdir(exist_ok=False)
for f in ['src/core/rr_results_current.h','src/core/rr_results_current.c','CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 d=b/f;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/f,d)
f=p/'src/core/rr_results_current.h';s=f.read_text();assert s.count('player_coin_rank,finish_position;')==1;s=s.replace('player_coin_rank,finish_position;','player_coin_rank,finish_position,player_takedowns;').replace(' * Estimated time is display pace,',' * player_takedowns is copied at publication, never read live during repaint.\n * Estimated time is display pace,');f.write_text(s)
f=p/'src/core/rr_results_current.c';s=f.read_text();assert s.count('if(!p||p->lap_count>2')==1;s=s.replace('if(!p||p->lap_count>2','if(!p||p->takedowns<0||p->lap_count>2').replace('value.finish_position=finish_position;','value.finish_position=finish_position;value.player_takedowns=p->takedowns;');f.write_text(s)
files={
'src/presentation/rr_results_summary.h':r'''#ifndef RR_RESULTS_SUMMARY_H
#define RR_RESULTS_SUMMARY_H
#include "rr_results_current.h"
enum {RR_RESULTS_ORDINAL_BYTES=16,RR_RESULTS_PLACE_BYTES=96};
typedef struct RRResultsSummary {char place[RR_RESULTS_PLACE_BYTES],coin_ordinal[RR_RESULTS_ORDINAL_BYTES];} RRResultsSummary;
/* Source positive-integer ordinal suffix, including11/12/13 and111..113.
 * Caller owns a16-byte buffer; invalid input leaves it unchanged. No heap. */
bool rr_results_ordinal(int value,char out[RR_RESULTS_ORDINAL_BYTES]);
/* Source frozen place/takedown header and coin-position ordinal, display only.
 * Solo place text is empty. UTF8 middle dot is actual source text, not an ASCII
 * substitute. Does not compute bonus/bank amount, send a save or grant anything.
 * Use coin_ordinal as a value, not a complete res-bank/account message. Failure
 * leaves output untouched. Font/fallback measurement and painting are external. */
bool rr_results_summary_build(const RRResultsCurrent *snapshot,RRResultsSummary *out);
#endif
''',
'src/presentation/rr_results_summary.c':r'''#include "rr_results_summary.h"
#include <stdio.h>
#include <string.h>
bool rr_results_ordinal(int value,char out[RR_RESULTS_ORDINAL_BYTES])
{
 if(!out||value<1)return false;
 int last=value%10,teens=(value/10)%10;
 const char *suffix=teens==1?"th":last==1?"st":last==2?"nd":last==3?"rd":"th";
 char text[RR_RESULTS_ORDINAL_BYTES];int n=snprintf(text,sizeof(text),"%d%s",value,suffix);
 if(n<0||(size_t)n>=sizeof(text))return false;
 memcpy(out,text,(size_t)n+1);return true;
}
bool rr_results_summary_build(const RRResultsCurrent *s,RRResultsSummary *out)
{
 if(!s||!out||!s->coin_count||s->coin_count>RR_LOCAL_MAX_CARS||s->finish_position<1||(size_t)s->finish_position>s->coin_count||s->player_coin_rank<1||(size_t)s->player_coin_rank>s->coin_count||s->player_takedowns<0)return false;
 RRResultsSummary value={0};char ordinal[RR_RESULTS_ORDINAL_BYTES];
 if(!rr_results_ordinal(s->player_coin_rank,value.coin_ordinal))return false;
 if(s->coin_count>1){
  if(!rr_results_ordinal(s->finish_position,ordinal))return false;
  int n=snprintf(value.place,sizeof(value.place),"%s OF %u \302\267 %d TAKEDOWN%s",ordinal,(unsigned)s->coin_count,s->player_takedowns,s->player_takedowns==1?"":"S");
  if(n<0||(size_t)n>=sizeof(value.place))return false;
 }
 *out=value;return true;
}
''',
'tools/capture-results-summary.mjs':r'''import fs from 'node:fs';import path from 'node:path';import vm from 'node:vm';import crypto from 'node:crypto';
const root=process.argv[2],game=path.resolve(root,'../game'),out=path.join(root,'reference/ui-parity-20261103/results-summary-capture');fs.mkdirSync(out,{recursive:true});
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),race=fs.readFileSync(path.join(game,'src/race.js'),'utf8');
const ordinal=race.split('\n').find(l=>l.startsWith('export const ordinal ='))?.replace('export const','const');
const place=race.split('\n').find(l=>l.trimStart().startsWith("$('res-place').textContent ="));
if(!ordinal||!place||!place.includes('player.takedowns === 1'))throw Error('Source summary boundary changed');
const program=ordinal+'\n'+place+'\ncapture(ordinal(g.coinPos));',script=new vm.Script(program),cases=[];
for(let count=1;count<=11;count++)for(let finish=1;finish<=count;finish++)for(let coin=1;coin<=count;coin++)for(const takedowns of [0,1,2,11,2147483647]){
 let captured=null;const node={textContent:''};
 script.runInNewContext({g:{racers:Array(count),finishPos:finish,coinPos:coin},multi:count>1,player:{takedowns},$:id=>{if(id!=='res-place')throw Error('Unexpected node');return node;},capture:v=>captured=v,Math},{timeout:1000});
 if(captured===null)throw Error('Missing ordinal');cases.push({count,finish,coin,takedowns,place:node.textContent,coinOrdinal:captured});
}
const ordinalCases=[];
for(const n of [...Array.from({length:131},(_,i)=>i+1),2147483647]){let expected=null;vm.runInNewContext(ordinal+'\ncapture(ordinal(value));',{value:n,capture:v=>expected=v,Math},{timeout:1000});ordinalCases.push({n,expected});}
if(cases.length!==2530||ordinalCases.length!==132)throw Error('Unexpected count');
const sources=['src/race.js','package.json'].map(file=>({path:file,sha256:sha(fs.readFileSync(path.join(game,file)))}));
const data={version:JSON.parse(fs.readFileSync(path.join(game,'package.json'))).version,sources,program,programSHA256:sha(program),cases,ordinalCases,scope:'Actual source res-place assignment and ordinal helper only, isolated VM with local text node/scripted counts. No source game imports, simulation, DOM renderer, account/bonus/bank/progression/network or graphics execution.'};
fs.writeFileSync(path.join(out,'reference.json'),JSON.stringify(data)+'\n');console.log({cases:cases.length,ordinals:ordinalCases.length,sourceOnly:true});
'''}
for f,s in files.items():
 assert not(p/f).exists(),f
 (p/f).write_text(s,encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text();assert s.count('add_library(rr_core STATIC ')==1;s=s.replace('add_library(rr_core STATIC ','add_library(rr_core STATIC src/presentation/rr_results_summary.c ')
marker='    add_executable(rr_results_card_tests '
assert s.count(marker)==1
block='''    add_executable(rr_results_summary_tests tests/test_results_summary.c)
    target_include_directories(rr_results_summary_tests PRIVATE src/presentation)
    target_link_libraries(rr_results_summary_tests PRIVATE rr_core)
    if(MSVC)
        target_compile_options(rr_results_summary_tests PRIVATE /W4 /WX)
    else()
        target_compile_options(rr_results_summary_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME browser_results_frozen_place_summary COMMAND rr_results_summary_tests)
'''
f.write_text(s.replace(marker,block+marker))
print('Added frozen takedown data and display-only summary formatter. Generate source references/tests before building; no launcher change.')

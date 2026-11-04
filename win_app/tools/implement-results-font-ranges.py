from pathlib import Path
import json,hashlib,shutil,sys
sys.path.insert(0,str(Path(__file__).parent/'fonttools'))
from fontTools.ttLib import TTFont
p=Path(r'C:\Users\User\Projects\neon_c\win_app');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1');b=p/'reference/pre-results-font-ranges-20261104';b.mkdir(exist_ok=False)
for name in ['src/presentation/rr_results_advance.h','src/presentation/rr_results_advance.c','src/presentation/rr_results_advance_data.h','tests/test_results_characters.c','tests/results_characters_reference.h','CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 q=b/name;q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/name,q)
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
fonts=[]
for filename,cp in [('ariblk.ttf',8776),('seguisym.ttf',9819)]:
 path=Path(r'C:\Windows\Fonts')/filename;f=TTFont(path);u=f['head'].unitsPerEm;a=f['hmtx'][f.getBestCmap()[cp]][0]
 fonts.append(dict(source=str(path),sha256=sha(path),family=f['name'].getDebugName(1),postscript=f['name'].getDebugName(6),codepoint=cp,units_per_em=u,advance_units=a,css_size=15,raw_advance=a*15/u))
assert [(f['units_per_em'],f['advance_units']) for f in fonts]==[(2048,1352),(2048,2048)]
f=p/'src/presentation/rr_results_advance_data.h';s=f.read_text();assert s.count('{8776u,634000u}')==1;f.write_text(s.replace('{8776u,634000u}','{8776u,633750u}'))
f=p/'src/presentation/rr_results_advance.h';s=f.read_text().replace('primary + queen character Range X envelopes.','primary + source approx/queen character Range X envelopes.').replace(' * Reuses all advance preflight/kerning/whitespace/UTF8 limits. Approx fallback\n * returns NEEDS_SHAPING: its captured range width is not an unrounded raw advance.',' * Reuses all advance preflight/kerning/whitespace/UTF8 limits. Fallback numeric\n * metrics come from read-only source-matched installed-font metadata; no font\n * file/outline is redistributed or loaded by this API.');f.write_text(s)
f=p/'src/presentation/rr_results_advance.c';s=f.read_text();line=' for(size_t offset=0;offset<bytes;){uint32_t cp=0;size_t used=0;(void)decode((const unsigned char*)text+offset,bytes-offset,&used,&cp);if(cp==8776)return RR_ADVANCE_NEEDS_SHAPING;offset+=used;}\n';assert line in s;s=s.replace(line,'')
s=s.replace('float advance=cp==9819?15.0f:(float)(ticks/960)*scale;','float advance=cp==9819?15.0f:cp==8776?1352.0f*(15.0f/2048.0f):(float)(ticks/960)*scale;');f.write_text(s)
obs=json.loads((pack/'observations.json').read_text(encoding='utf-8'));cases=[];multi=0;characters=0;families=set()
for o in obs:
 for value in o['platformFonts'].values():
  for family in value:families.add((family['familyName'],family['postScriptName']))
 for name,l in o['lists'].items():
  for row in l['rows']:
   for kind,ns in [('name',row['directTextNodes']),('value',row['valueSpan']['textNodes'])]:
    for node in ns:
     if len(node['rangeClientRectsViewport'])!=1:multi+=1;continue
     origin=node['rangeClientRectsViewport'][0]['x'];chars=[]
     for info in node['characterRectsViewport']:
      assert len(info['rects'])==1 and len(info['text'])==1
      r=info['rects'][0];chars.append(dict(cp=ord(info['text']),left=r['x']-origin,right=r['x']-origin+r['width']))
     characters+=len(chars);cases.append(dict(fixture=o['fixture'],scale=o['scale'],offset=o['offset'],list=name,row=row['index'],kind=kind,text=node['text'],width=node['rangeClientRectsViewport'][0]['width'],chars=chars))
assert len(cases)==1119 and characters==8676 and multi==21
glyphpack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersFallbackGlyphs-v2')
glyphs=json.loads((glyphpack/'glyph-metrics.json').read_text(encoding='utf-8'))
approx=[g for g in glyphs if g['codepoint']==8776 and g['fontSizeCSS']==15]
assert len(approx)==3 and all(g['canvasAdvanceWithoutTracking']==9.90234375 and any(f['familyName']=='Arial Black' and f['postScriptName']=='Arial-Black' for f in g['observedFont']) for g in approx)
assert ('Segoe UI Symbol','SegoeUISymbol') in families
def c(s):return '"'+''.join(chr(x) if 32<=x<127 and chr(x) not in '\\"' else '\\%03o'%x for x in s.encode('utf-8'))+'"'
fixture='''/* Source-measured single-line Range geometry; system font NUMBERS only. */
typedef struct ExpectedCharacter {uint32_t cp;double left,right;} ExpectedCharacter;
typedef struct CharacterCase {const char *text;double width;size_t count;ExpectedCharacter chars[128];} CharacterCase;
static const CharacterCase character_cases[]={
'''
for x in cases:fixture+='{%s,%s,%d,{%s}},\n'%(c(x['text']),repr(x['width']),len(x['chars']),','.join('{%du,%s,%s}'%(y['cp'],repr(y['left']),repr(y['right'])) for y in x['chars']))
fixture+='};\n';(p/'tests/results_characters_reference.h').write_text(fixture,encoding='ascii')
f=p/'tests/test_results_characters.c';s=f.read_text();old='const char *needs[]={"\\342\\211\\210 02:30.00","AWX"';assert old in s;s=s.replace(old,'const char *needs[]={"AWX"')
s=s.replace(' memset(&guard,0x5a,sizeof(guard));saved=guard;',' CHECK(rr_results_characters("\\342\\211\\210",&actual)==RR_ADVANCE_READY&&actual.advance==9.90234375&&actual.range_width==9.90625);\n RRResultsAdvance width;CHECK(rr_results_advance("\\342\\211\\210",&width)==RR_ADVANCE_READY&&width.ticks==633750&&width.advance==9.90234375&&width.range_width==9.90625);\n memset(&guard,0x5a,sizeof(guard));saved=guard;')
s=s.replace('approx_fallback_unqualified=true','approx_raw_metric_verified=true');f.write_text(s)
f=p/'CMakeLists.txt';s=f.read_text();assert s.count('browser_results_character_range_positions')==1;f.write_text(s.replace('browser_results_character_range_positions','browser_results_font_run_range_positions'))
dest=p/'reference/ui-parity-20261103/results-font-ranges-capture';dest.mkdir(exist_ok=False)
analysis=json.loads((Path(__file__).parent/'results-float32-font-analysis.json').read_text(encoding='utf-8'));width_analysis=json.loads((Path(__file__).parent/'results-shaping-raw-fallback-analysis.json').read_text(encoding='utf-8'))
(dest/'reference.json').write_text(json.dumps(dict(package=str(pack),package_manifest_sha256=sha(pack/'manifest.json'),fallback_numeric_provenance=fonts,observed_platform_fonts=sorted(families),targeted_glyph_evidence=dict(path=str(glyphpack/'glyph-metrics.json'),sha256=sha(glyphpack/'glyph-metrics.json'),approx=approx,note='Wrapping corpus parent-list font reports do not include Arial Black in floated spans; targeted v2 glyph-run evidence supplies that family/raw canvas metric. No broader font coverage is inferred.'),cases=cases,supported_nodes=1119,characters=8676,multiline_excluded_nodes=21,hypotheses=analysis,width_analysis=width_analysis,scope='Numeric source-font metadata only; no system-font file/outline/bitmap copied, redistributed or runtime-loaded. Every measured supported single-line Range X endpoint compared; no raster, vertical/float/native-main qualification.'),indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(nodes=1119,characters=8676,font_files_copied=0,raw_approx=9.90234375,old_range_envelope=9.90625,prior_subset_builds_preserved=True)))

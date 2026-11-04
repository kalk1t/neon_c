from pathlib import Path
import json,sys,hashlib
sys.path.insert(0,str(Path(__file__).parent/'fonttools'))
from fontTools.ttLib import TTFont
p=Path(r'C:\Users\User\Projects\neon_c\win_app');pack=Path(r'C:\Users\User\Documents\Codex\2026-11-03\task\RoadstersResultsWrapping-v1')
def sha(f):return hashlib.sha256(f.read_bytes()).hexdigest()
assert '{8776u,633750u}' in (p/'src/presentation/rr_results_advance_data.h').read_text()
assert '1352.0f*(15.0f/2048.0f)' in (p/'src/presentation/rr_results_advance.c').read_text()
assert not(p/'reference/ui-parity-20261103/results-font-ranges-capture').exists()
fonts=[]
for filename,cp in [('ariblk.ttf',8776),('seguisym.ttf',9819)]:
 path=Path(r'C:\Windows\Fonts')/filename;f=TTFont(path);u=f['head'].unitsPerEm;a=f['hmtx'][f.getBestCmap()[cp]][0]
 fonts.append(dict(source=str(path),sha256=sha(path),family=f['name'].getDebugName(1),postscript=f['name'].getDebugName(6),codepoint=cp,units_per_em=u,advance_units=a,css_size=15,raw_advance=a*15/u))
script=(Path(__file__).parent/'implement-results-font-ranges.py').read_text();start=script.index('obs=json.loads(')
exec(compile(script[start:],'resume-verified-font-range-fixtures','exec'),dict(Path=Path,p=p,pack=pack,json=json,fonts=fonts,sha=sha,__file__=__file__))
note=dict(reason='Initial staging asserted Arial Black in parent-list platform-font inventory, but floated span font is absent from that inventory. Corrected validation to the existing targeted v2 glyph-run observedFont and canvasAdvanceWithoutTracking evidence; wrapping inputs were not changed.',partial_initial_writes=['src/presentation/rr_results_advance_data.h','src/presentation/rr_results_advance.h','src/presentation/rr_results_advance.c'],resumed_at='Read-only observation/fixture/provenance stage after verifying initial writes; backups were not replaced.',no_font_files_copied=True)
(p/'reference/ui-parity-20261103/results-font-ranges-capture/staging-note.json').write_text(json.dumps(note,indent=2)+'\n')

from pathlib import Path
import sys,json,hashlib
sys.path.insert(0,str(Path(__file__).parent/'fonttools'))
from fontTools.ttLib import TTFont
path=Path(r'C:\Windows\Fonts\ariblk.ttf');font=TTFont(path);units=font['head'].unitsPerEm;advance=font['hmtx'][font.getBestCmap()[8776]][0];raw=advance*15/units
proof=dict(source=str(path),sha256=hashlib.sha256(path.read_bytes()).hexdigest(),family=font['name'].getDebugName(1),postscript=font['name'].getDebugName(6),codepoint=8776,units_per_em=units,advance_units=advance,css_size=15,raw_advance=raw,scope='Read-only numeric metadata. No system font file, outline or bitmap copied or redistributed; no device or browser execution.')
(Path(__file__).parent/'results-fallback-font-provenance.json').write_text(json.dumps(proof,indent=2),encoding='utf-8')
source=(Path(__file__).parent/'analyze-results-float32.py').read_text().replace('for approx in [9.90625,9.9]:','for approx in ['+repr(raw)+']:').replace('results-float32-analysis.json','results-float32-font-analysis.json')
exec(compile(source,str(Path(__file__).parent/'analyze-results-float32.py'),'exec'),{'__file__':__file__})

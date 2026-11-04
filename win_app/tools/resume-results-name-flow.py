from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app')
assert(p/'src/presentation/rr_results_name_flow.c').exists() and not(p/'tests/test_results_name_flow.c').exists()
s=(Path(__file__).parent/'implement-results-name-flow.py').read_text();start=s.index('analysis=json.loads(')
exec(compile(s[start:],'resume-name-flow-fixtures','exec'),dict(Path=Path,p=p,json=json,shutil=shutil,__file__=__file__))
(p/'reference/ui-parity-20261103/results-name-flow-capture/staging-note.json').write_text(json.dumps(dict(reason='Initial generator compared JSON-loaded list slices with tuple-normalized expected rectangles, so equal numeric geometry failed a container-type assertion before fixtures/tests/CMake were written. Normalized both sides to tuples. Runtime source unchanged. Line character indices were additionally derived from independent measured character rectangles rather than the prototype.',resumed_stage='Fixture/source-observation processing only',initial_writes=['src/presentation/rr_results_name_flow.h','src/presentation/rr_results_name_flow.c']),indent=2)+'\n')

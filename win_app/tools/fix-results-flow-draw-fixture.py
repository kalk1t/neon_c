from pathlib import Path
import json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');r=p/'reference/ui-parity-20261103';cap=r/'results-flow-draw-capture'
for target in ['native','web']:
 for source,dest in [(r/f'results-flow-draw-{target}-build.log',cap/f'initial-{target}-build.log'),(p/f'build/results-flow-draw-{target}/Testing/Temporary/LastTest.log',cap/f'initial-{target}-LastTest.log')]:
  assert not dest.exists();shutil.copy2(source,dest)
test=p/'tests/test_results_flow_draw.c';s=test.read_text();old='RRResultsFlowText late[]={ {"Pilot","0"},{"ZED","1"} };';new='RRResultsFlowText late[]={ {"ACE","0"},{"Z","1"} };CHECK(rr_results_characters(late[0].name,&work.name_characters)==RR_ADVANCE_READY&&rr_results_characters(late[1].name,&work.name_characters)==RR_ADVANCE_READY);';assert old in s
shutil.copy2(test,cap/'initial-test_results_flow_draw.c');test.write_text(s.replace(old,new),encoding='utf-8')
script=Path(__file__).parent/'implement-results-flow-draw.py';s=script.read_text();assert old in s;script.write_text(s.replace(old,new),encoding='utf-8')
(cap/'fixture-correction.json').write_text(json.dumps(dict(initial_results={'native':'132/133 passed, observed14526 commands exact, one adversarial assertion failed','web':'132/133 passed, observed14526 commands exact, one adversarial assertion failed'},cause='Adversarial name Pilot has nonzero primary GPOS pairs il/lo/ot; the qualified layout returns NEEDS_FLOW before the deliberately missing Z font glyph. Expected INVALID was therefore wrong for that setup.',correction='Use verified supported ACE and Z names, assert their shaping-ready status before simulating missing Z. Runtime source and observed-source expectations unchanged.',preserved='Both initial build and LastTest logs plus original test source are in this folder.'),indent=2)+'\n',encoding='utf-8')
print('Preserved initial failures; corrected adversarial test fixture only.')

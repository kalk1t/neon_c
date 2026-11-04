from pathlib import Path
import ast,json,shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');s=(Path(__file__).parent/'implement-results-flow-draw.py').read_text();constants={}
for node in ast.parse(s).body:
 if isinstance(node,ast.Assign) and len(node.targets)==1 and isinstance(node.targets[0],ast.Name)and node.targets[0].id in ['header','source']:constants[node.targets[0].id]=ast.literal_eval(node.value)
for key,name in [('header','rr_results_flow_draw.h'),('source','rr_results_flow_draw.c')]:assert (p/'src/presentation'/name).read_text()==constants[key]
assert not(p/'tests/results_flow_draw_reference.h').exists() and 'rr_results_flow_draw_tests'not in(p/'CMakeLists.txt').read_text()
exec(compile(s[s.index('pack=Path('):],__file__,'exec'),globals())
dest=p/'reference/ui-parity-20261103/results-flow-draw-capture/staging-note.json';dest.write_text(json.dumps(dict(initial_staging='Stopped on wrong observation field name classes; actual schema is className, valueSpan.textNodes, character text.',completed_before_stop=['Original backups','Two new runtime files'],resumed='Fixture/CMake/reference suffix only after verifying runtime source literals. No build or tests had started; existing evidence preserved.'),indent=2)+'\n',encoding='utf-8')

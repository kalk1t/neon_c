from pathlib import Path
import ast
p=Path(r'C:\Users\User\Projects\neon_c\win_app');script=(Path(__file__).parent/'implement-current-host-frame.py').read_text();values={}
for node in ast.parse(script).body:
 if isinstance(node,ast.Assign)and len(node.targets)==1 and isinstance(node.targets[0],ast.Name)and node.targets[0].id in ['header','source','test']:values[node.targets[0].id]=ast.literal_eval(node.value)
for key,name in [('header','src/presentation/rr_current_empty_host.h'),('source','src/presentation/rr_current_empty_host.c'),('test','tests/test_current_empty_host.c')]:assert (p/name).exists();(p/name).write_text(values[key],encoding='utf-8')
print('Reviewed new staged boundary: clear stale draw flags even when simulation fails; no predecessor files changed.')

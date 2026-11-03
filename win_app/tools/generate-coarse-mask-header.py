from pathlib import Path
import hashlib,json,struct
r=Path(r'C:\Users\User\Projects\neon_c\win_app');s=Path(__file__).resolve().parents[1];raw=(r/'reference/city-coarse-witness-masks.bin').read_bytes();assert raw[:4]==b'CWM1' and len(raw)==141282
at=4;masks={}
for _ in range(2):
 view=struct.unpack_from('<I',raw,at)[0];at+=4;masks[view]=raw[at:at+70635];at+=70635
old=json.loads((r/'reference/city-visibility-results.json').read_text());counts={view:sum(b.bit_count() for b in mask) for view,mask in masks.items()}
for view,count in counts.items():assert old['occlusion'][view]['view']==view and old['occlusion'][view]['occluded_vertices']==count*3
header='/* Targeted original coarse occlusion masks: view255,view0; source-fidelity comparison only. */\nstatic const unsigned char coarse_witness_masks[2][70635]={\n'
for view in [255,0]:
 data=masks[view];header+='{\n'+',\n'.join(','.join(str(b) for b in data[i:i+64]) for i in range(0,len(data),64))+'\n},\n'
header+='};\n';(s/'tests/generated_coarse_witness_masks.h').write_text(header)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();meta={'views':[255,0],'occluded_triangle_counts':counts,'mask_sha256':sha(r/'reference/city-coarse-witness-masks.bin'),'geometry_sha256':sha(r/'reference/city-repacking-geometry.bin'),'source_test_sha256':sha(r/'tests/test_city_occlusion.c'),'kernel_sha256':sha(r/'src/experimental/rr_city_occlusion.c'),'scope':'Only targeted0/255 native mask run; bit population matches preserved coarse model. Source far/guards unchanged; no new camera sampling or runtime integration.'};(s/'reference/city-coarse-witness-masks.json').write_text(json.dumps(meta,indent=2)+'\n');print(json.dumps(meta,indent=2))

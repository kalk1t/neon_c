"""Import verified unmodified v2 static faces into a new or identical output folder.
python import-ui-static-fonts.py PACKAGE_ROOT OUTPUT_DIRECTORY
Existing different files are refused; preserve them before choosing a new destination.
No download, font instancing, browser execution or graphics initialization.
"""
from pathlib import Path
import json,sys,hashlib,struct,shutil
pkg=Path(sys.argv[1]);out=Path(sys.argv[2]);out.mkdir(parents=True,exist_ok=True)
manifest=json.loads((pkg/'manifest.json').read_text(encoding='utf-8-sig'));entries={x['file']:x for x in manifest['assets']}
faces=json.loads((pkg/'provenance/font-faces.json').read_text(encoding='utf-8-sig'))
mapping={'Orbitron-500.ttf':'Orbitron-500.ttf','Orbitron-700.ttf':'Orbitron-700.ttf','Orbitron-900.ttf':'Orbitron-900.ttf','Rajdhani-500.ttf':'Rajdhani-Medium.ttf','Orbitron-OFL.txt':'Orbitron-OFL.txt','Rajdhani-OFL.txt':'Rajdhani-OFL.txt'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for src,dst in mapping.items():
    p=pkg/'fonts'/src;entry=entries['fonts/'+src];assert p.stat().st_size==entry['bytes'] and sha(p)==entry['sha256']
    if p.suffix=='.ttf':
        b=p.read_bytes();tables={}
        for i in range(struct.unpack('>H',b[4:6])[0]):
            tag,_,offset,size=struct.unpack('>4sIII',b[12+16*i:28+16*i]);assert offset+size<=len(b);tables[tag]=(offset,size)
        assert b'fvar' not in tables;face=next(x for x in faces['faces'] if x['file']==src)
        offset,_=tables[b'OS/2'];assert struct.unpack('>H',b[offset+4:offset+6])[0]==int(face['weight']) and sha(p)==face['sha256']
    target=out/dst
    if target.exists():assert sha(target)==sha(p),f'Refuse overwrite of different file {target}'
    else:shutil.copy2(p,target)
print(json.dumps({'imported_or_verified':len(mapping),'source_manifest_sha256':sha(pkg/'manifest.json'),'font_request':faces['cssSource'],'output':str(out)},indent=2))

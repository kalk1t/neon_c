"""Reproduce static UI weights from the reviewed asset package; FontTools 4.61.1.
Usage: python prepare-ui-fonts.py PACKAGE_ROOT OUTPUT_DIRECTORY
Original files and browser source are never modified.
"""
from pathlib import Path
import sys,json,hashlib,shutil
from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont
package=Path(sys.argv[1]);output=Path(sys.argv[2]);output.mkdir(parents=True,exist_ok=True)
source=package/'runtime/fonts/Orbitron.ttf';checks=[]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for weight in (500,700,900):
    f=instantiateVariableFont(TTFont(source),{'wght':weight},inplace=True)
    # Preserve attribution and OFL, respect upstream Reserved Font Name.
    for n in f['name'].names:
        if n.nameID in (1,3,4,6,16,21,25):
            text=n.toUnicode().replace('Orbitron','RoadstersUI')
            n.string=text.encode(n.getEncoding())
    f.recalcTimestamp=False
    target=output/f'Orbitron-{weight}.ttf';f.save(target);check=TTFont(target)
    assert 'fvar' not in check and check['OS/2'].usWeightClass==weight
    assert all('Orbitron' not in n.toUnicode() for n in check['name'].names if n.nameID in (1,4,6,16))
    missing=[chr(i) for i in range(32,127) if i not in check.getBestCmap()];assert missing==['^']
    checks.append({'path':target.name,'sha256':digest(target),'weight':weight,'static':True,'primary_family':'RoadstersUI','missing_ascii':missing,'note':'Caret unused by bounded menu/road UI','glyph_count':check['maxp'].numGlyphs})
for name in ('Orbitron-OFL.txt','Rajdhani-Medium.ttf','Rajdhani-OFL.txt'):shutil.copy2(package/'runtime/fonts'/name,output/name)
(output/'provenance.json').write_text(json.dumps({'source':str(source),'source_sha256':digest(source),'method':'FontTools 4.61.1 instantiateVariableFont wght=500/700/900, unaltered interpolated outlines; derivative primary family renamed RoadstersUI to respect Orbitron RFN; filenames retain source lineage','fonts':checks,'license':'SIL OFL notices retained; original copyright attribution unchanged','native_rendering':'Unrun; CPU rasterization is separate evidence'},indent=2)+'\n')
print(json.dumps(checks,indent=2))

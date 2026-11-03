"""Read-only metadata/format audit for versioned Roadsters UI handoffs.
python audit-ui-package.py PACKAGE_ROOT REPORT_JSON
No component execution, browser/network/graphics, asset extraction or runtime edits.
"""
from pathlib import Path
import sys,json,hashlib,struct,collections,zipfile
root=Path(sys.argv[1]).resolve();report=Path(sys.argv[2])
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def child(name):
    p=(root/name).resolve()
    if not p.is_relative_to(root):raise ValueError('Manifest path leaves package root')
    return p
manifest=json.loads((root/'manifest.json').read_text(encoding='utf-8-sig'));assets=json.loads((root/'assets.json').read_text(encoding='utf-8-sig'))
pngs=[];formats=collections.Counter();checked=0
for entry in manifest['assets']:
    name=entry.get('file',entry.get('id'));p=child(name);b=p.read_bytes()
    if len(b)!=entry['bytes'] or hashlib.sha256(b).hexdigest()!=entry['sha256']:raise ValueError('Manifest mismatch: '+name)
    checked+=1
    if p.suffix.lower()=='.png':
        if b[:8]!=b'\x89PNG\r\n\x1a\n' or b[12:16]!=b'IHDR':raise ValueError('PNG header: '+name)
        w,h,depth,color,compression,filtering,interlace=struct.unpack('>IIBBBBB',b[16:29]);formats[(depth,color,interlace)]+=1
        if entry.get('width',w)!=w or entry.get('height',h)!=h:raise ValueError('PNG metadata dimensions: '+name)
        pngs.append({'path':name,'width':w,'height':h,'bit_depth':depth,'color_type':color,'interlace':interlace,'rgba8_bytes':w*h*4,'sha256':entry['sha256']})
png_index={p['path']:p for p in pngs};kinds=collections.Counter();dims=collections.defaultdict(set);candidates=[]
for item in assets:
    kinds[item['kind']]+=1;r=item.get('observed',{}).get('rect',{});dims[item['kind']].add((r.get('width'),r.get('height')))
    if item['id'].startswith('shop/cards/') and item.get('scale')==1 and item['id'].endswith('/default'):
        candidates.append(item)
state_paths={v for item in assets for k in ('chrome','reference') if (v:=item.get(k))}
if not state_paths<=set(png_index):raise ValueError('Component PNG reference missing from verified manifest')
summary={'package_root':str(root),'package_version':manifest.get('packageVersion'),'source_release':manifest.get('sourceRelease'),'manifest_sha256':digest(root/'manifest.json'),'verified_files':checked,'checksum_failures':0,'png_files':len(pngs),'png_header_format_counts':[{'bit_depth':k[0],'color_type':k[1],'interlace':k[2],'count':n} for k,n in sorted(formats.items())],'all_pngs_rgba8_noninterlaced':all(p['bit_depth']==8 and p['color_type']==6 and p['interlace']==0 for p in pngs),'all_pngs_8bit_rgb_or_rgba_noninterlaced':all(p['bit_depth']==8 and p['color_type'] in (2,6) and p['interlace']==0 for p in pngs),'all_png_rgba8_bytes_if_loaded':sum(p['rgba8_bytes'] for p in pngs),'state_png_paths':sorted(state_paths),'state_png_count':len(state_paths),'state_png_rgba8_bytes_if_loaded':sum(png_index[n]['rgba8_bytes'] for n in state_paths),'state_scale_records':len(assets),'unique_state_ids':len({a['id'] for a in assets}),'kinds':dict(kinds),'logical_dimensions_by_kind':{k:[list(x) for x in sorted(v,key=str)] for k,v in dims.items()},'garage_default_1x_records':candidates[:6],'png_inventory':pngs,'limits':'Fresh hash/length/IHDR/metadata audit only, not decoded pixel/color equality or native image/font/render/input/GPU acceptance. Reference files contain synthetic labels/data and are not live UI skins.'}
if len(sys.argv)>3:
    archive=Path(sys.argv[3]);archive_checked=0
    with zipfile.ZipFile(archive) as z:
        index={name.replace('\\','/'):name for name in z.namelist()}
        prefix='' if 'manifest.json' in index else root.name+'/'
        manifest_entry=index[prefix+'manifest.json'];assert z.getinfo(manifest_entry).file_size==(root/'manifest.json').stat().st_size
        if hashlib.sha256(z.read(manifest_entry)).hexdigest()!=summary['manifest_sha256']:raise ValueError('Archive/directory manifest mismatch')
        for entry in manifest['assets']:
            name=entry.get('file',entry.get('id'));key=prefix+name;item=index[key]
            if z.getinfo(item).file_size!=entry['bytes']:raise ValueError('Archive size mismatch: '+name)
            if hashlib.sha256(z.read(item)).hexdigest()!=entry['sha256']:raise ValueError('Archive checksum mismatch: '+name)
            archive_checked+=1
    summary['archive']={'path':str(archive),'sha256':digest(archive),'verified_files':archive_checked,'checksum_failures':0,'manifest_matches_directory':True}
report.parent.mkdir(parents=True,exist_ok=True);report.write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8')
print(json.dumps({k:summary[k] for k in ['verified_files','checksum_failures','png_files','state_scale_records','unique_state_ids','all_pngs_rgba8_noninterlaced','all_png_rgba8_bytes_if_loaded','logical_dimensions_by_kind']},indent=2))

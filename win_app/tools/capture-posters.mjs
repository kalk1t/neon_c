// Read-only browser CPU geometry + Canvas command recorder. No raster, browser,
// renderer, GPU, asset generation or source modification.
import {readFile,writeFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';import vm from 'node:vm';
import path from 'node:path';import {pathToFileURL} from 'node:url';
const native=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),browser=path.resolve(native,'../game');
const sha=b=>createHash('sha256').update(b).digest('hex'),load=p=>import(pathToFileURL(path.join(browser,p)).href);
const names=['package.json','src/city.js','src/materials.js','src/geo.js','src/roads.js','src/track.js','src/physics.js'];
const sources=await Promise.all(names.map(async p=>({path:p,sha256:sha(await readFile(path.join(browser,p)))})));
const [THREE,{Track,Corridor},{ROADS},{OBB},{rng,billboardTexture},{facadeBox}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/track.js'),load('src/roads.js'),load('src/physics.js'),load('src/materials.js'),load('src/city.js')]);
const source=await readFile(path.join(browser,'src/city.js'),'utf8');
let code=source.slice(source.indexOf('export function buildCity('),source.indexOf('  const winTex = windowTexture();'));
let a=code.indexOf('  const push ='),b=code.indexOf('\n\n  const neonStrip',a);
code=code.slice(0,a)+'  const push=(list,geo,lx,ly,lz,ry=0)=>capturePush(list,geo,lx,ly,lz,ry,_m);'+code.slice(b);
for(const [anchor,kind] of [['      tower(x, z, w, d, top, rot, dist < 420);',0],['    _m.makeTranslation(x, 0, z);',1],['    const ang = Math.atan2(-(b.z - a.z), b.x - a.x);',2]]){if(!code.includes(anchor))throw Error('source hook changed');code=code.replace(anchor,`    beginRecord(${kind});\n`+anchor);}
code=code.replace('export function','function')+'\nreturn {towers,heroes};\n}';
let id=-1;const records=[],posters=[];
const context={THREE,BASE:-260,HERO_ASPECT:2.95,rng,facadeBox,OBB,darkUV:g=>{for(let i=0;i<g.attributes.uv.count;i++)g.attributes.uv.setXY(i,.004,.004);return g;},beginRecord(kind){id++;records.push({id,kind,collider:null});},capturePush(list,g,lx,ly,lz,ry,m){if(list==='board'){const copy=(g.index?g.toNonIndexed():g.clone()).applyMatrix4(m.clone().multiply(new THREE.Matrix4().makeTranslation(lx,ly,lz)).multiply(new THREE.Matrix4().makeRotationY(ry)));const p=copy.attributes.position,n=copy.attributes.normal,u=copy.attributes.uv;posters.push({catalog_id:id,rows:Array.from({length:p.count},(_,i)=>[p.getX(i),p.getY(i),p.getZ(i),n.getX(i),n.getY(i),n.getZ(i),u.getX(i),u.getY(i)])});copy.dispose();}g.dispose();}};
vm.createContext(context);vm.runInContext(code+';this.run=buildCity;',context);
context.run({},new Corridor(ROADS.map(r=>new Track(r))),{add(c){if(records[id].collider)throw Error('two colliders');records[id].collider={center:c.c.toArray(),half:c.h.toArray(),cosine:c.cos,sine:c.sin};return c;}},{},{});
const commands=[],gradients=[];let canvas;
const ctx=new Proxy({createLinearGradient(...args){const gradient={id:gradients.length,args,stops:[],addColorStop(at,color){this.stops.push([at,color]);}};gradients.push(gradient);commands.push(['createLinearGradient',...args]);return gradient;}},{get(t,k){return k in t?t[k]:(...args)=>commands.push([k,...args]);},set(t,k,v){t[k]=v;commands.push(['set',k,v&&typeof v==='object'?{gradient:v.id}:v]);return true;}});
globalThis.document={createElement(type){if(type!=='canvas')throw Error('unexpected document access');canvas={width:0,height:0,getContext(type){if(type!=='2d')throw Error('not CPU canvas recorder');return ctx;}};return canvas;}};
const texture=billboardTexture();let material;
const materialCode=source.slice(source.indexOf('  const boardMat ='),source.indexOf('\n  const mats ='));
const mc={THREE,billboardTexture:()=>texture};vm.createContext(mc);vm.runInContext(materialCode+';this.material=boardMat;',mc);material=mc.material;
const glyphs=[];for(let i=0;i<commands.length;i++)if(commands[i][0]==='moveTo'){if(commands[i+1][0]!=='lineTo'||commands[i+2][0]!=='lineTo')throw Error('glyph path changed');glyphs.push(commands.slice(i,i+3).map(x=>x.slice(1)));}
const spec={type:material.type,width:canvas.width,height:canvas.height,color_space:texture.colorSpace,flip_y:texture.flipY,anisotropy:texture.anisotropy,wrap_s:texture.wrapS,wrap_t:texture.wrapT,min_filter:texture.minFilter,mag_filter:texture.magFilter,mipmaps:texture.generateMipmaps,premultiply_alpha:texture.premultiplyAlpha,side:material.side,color:material.color.toArray(),transparent:material.transparent,opacity:material.opacity,depth_test:material.depthTest,depth_write:material.depthWrite,fog:material.fog,tone_mapped:material.toneMapped};
if(records.length!==2741||glyphs.length!==28)throw Error(`poster source coverage changed: records=${records.length},posters=${posters.length},glyphs=${glyphs.length}`);
for(const s of sources)if(sha(await readFile(path.join(browser,s.path)))!==s.sha256)throw Error('source changed');
await writeFile(out,JSON.stringify({sources,scope:'Actual browser CPU geometry/material construction and Canvas command recorder; no raster/pixel/render/GPU result.',records:records.length,posters:posters.map(p=>({...p,collider:records[p.catalog_id].collider})),material:spec,gradients:gradients.map(({id,args,stops})=>({id,args,stops})),glyphs,commands},null,2)+'\n');
console.log({posters:posters.length,commands:commands.length,glyphs:glyphs.length,material:spec});

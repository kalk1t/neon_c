import {readFile,writeFile,mkdir} from 'node:fs/promises';import {createHash} from 'node:crypto';import {fileURLToPath,pathToFileURL} from 'node:url';import path from 'node:path';import vm from 'node:vm';
const root=fileURLToPath(new URL('../',import.meta.url)),browser=path.resolve(process.argv[2]??path.join(root,'../game')),sha=b=>createHash('sha256').update(b).digest('hex');
const files=['package.json','src/city.js','src/skyprops.js','src/materials.js','src/geo.js','src/physics.js','src/track.js','src/roads.js','src/main.js','public/models/props/city_tower.glb'];
const hashes=()=>Promise.all(files.map(async p=>({path:p,sha256:sha(await readFile(path.join(browser,p)))})));const before=await hashes(),load=p=>import(pathToFileURL(path.join(browser,p)).href);
const [THREE,{Track,Corridor},{ROADS},{OBB,Colliders},{rng},{facadeBox},{norm}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/track.js'),load('src/roads.js'),load('src/physics.js'),load('src/materials.js'),load('src/city.js'),load('src/geo.js')]);
const tracks=ROADS.map(r=>new Track(r)),corridor=new Corridor(tracks),source=await readFile(path.join(browser,'src/city.js'),'utf8');
const tile={gx:[20,47],gz:[21,44],step:82,cx:200,cz:0,extent:2700};const bounds={minX:200-2700+20*82-41,maxX:200-2700+48*82-41,minZ:-2700+21*82-41,maxZ:-2700+45*82-41};
const records=[];let current=null,skippedBoards=0;
const dummy=()=>({attributes:{uv:{count:0,setXY(){}}},rotateX(){return this;},rotateY(){return this;}});
const constructors=new Set(['BoxGeometry','CylinderGeometry','ConeGeometry','PlaneGeometry','TorusGeometry']);
const proxy=new Proxy(THREE,{get(target,key){if(!constructors.has(key))return target[key];return function(...args){return current?new target[key](...args):dummy();};}});
const palette={body:0x0d0b24,neonC:0x14e4ff,neonM:0xff26d4,tip:0xff26d4};
function begin(gx,gz,x,z,w,d,top,rot,near){const ix=Math.round((gx+2500)/82),iz=Math.round((gz+2700)/82);current=null;if(ix>=20&&ix<=47&&iz>=21&&iz<=44){current={gx:ix,gz:iz,x,z,w,d,top,rot,near,parts:[],vertices:0,collider:null};records.push(current);}}
function push(list,geo,lx,ly,lz,ry,matrix){
 if(!current)return;if(list==='board'){skippedBoards++;return;}
 const transform=matrix.clone().multiply(new THREE.Matrix4().makeTranslation(lx,ly,lz)).multiply(new THREE.Matrix4().makeRotationY(ry));
 const g=norm(geo.clone());g.applyMatrix4(transform);current.parts.push({g,color:palette[list]});current.vertices+=g.attributes.position.count;
}
let body=source.slice(source.indexOf('export function buildCity('),source.indexOf('  // Distant silhouette ring'));
const pa=body.indexOf('  const push ='),pb=body.indexOf('\n\n  const neonStrip',pa);if(pa<0||pb<0)throw Error('Push source changed');body=body.slice(0,pa)+'  const push=(list,geo,lx,ly,lz,ry=0)=>capturePush(list,geo,lx,ly,lz,ry,_m);'+body.slice(pb);
const call='      tower(x, z, w, d, top, rot, dist < 420);';if(!body.includes(call))throw Error('Grid call changed');body=body.replace(call,'      beginBuilding(gx,gz,x,z,w,d,top,rot,dist<420);\n'+call);
body=body.replace('export function','function')+'\n return {towers,heroes};\n}';
const context={THREE:proxy,BASE:-260,HERO_ASPECT:2.95,rng,facadeBox:(...args)=>current?facadeBox(...args):dummy(),norm,darkUV:g=>{const uv=g.attributes.uv;for(let i=0;i<uv.count;i++)uv.setXY(i,.004,.004);return g;},OBB,capturePush:push,beginBuilding:begin};vm.createContext(context);
const colliders={add:c=>{if(current){if(current.collider)throw Error('Expected one city body collider');current.collider=c;}return c;}};
vm.runInContext(body+'\nthis.run=buildCity;',context);const result=context.run({},corridor,colliders,{},{});
if(records.length>400)throw Error('Bounded tile building capacity exceeded');
const heroes=result.heroes.filter(h=>records.some(r=>r.x===h.x&&r.z===h.z)),tileWorld=new Colliders();for(const r of records){if(!r.collider||!r.vertices||r.vertices>100000)throw Error('Invalid captured building');tileWorld.add(r.collider);}
await mkdir(path.join(root,'assets/city-core'),{recursive:true});const assets=[];let rows='';
for(const [index,r]of records.entries()){
 const bytes=Buffer.alloc(8+r.vertices*36);bytes.write('RRC1');bytes.writeUInt32LE(r.vertices,4);let at=8;const box=new THREE.Box3();
 for(const {g,color}of r.parts){if(color===undefined)throw Error('Unknown city palette');const{position,normal,uv}=g.attributes;g.computeBoundingBox();box.union(g.boundingBox);
  for(let i=0;i<position.count;i++){for(const v of[position.getX(i),position.getY(i),position.getZ(i),normal.getX(i),normal.getY(i),normal.getZ(i),uv.getX(i),uv.getY(i)]){if(!Number.isFinite(v))throw Error('Nonfinite city mesh');bytes.writeFloatLE(v,at);at+=4;}bytes[at++]=(color>>16)&255;bytes[at++]=(color>>8)&255;bytes[at++]=color&255;bytes[at++]=255;}
 }
 const file=`assets/city-core/b${String(index).padStart(4,'0')}.rrmesh`;await writeFile(path.join(root,file),bytes);assets.push({path:file,vertices:r.vertices,bytes:bytes.length,sha256:sha(bytes)});
 const c=r.collider,hero=heroes.findIndex(h=>h.x===r.x&&h.z===r.z);rows+=`{{${c.c.toArray()}},{${c.h.toArray()}},${Math.atan2(c.sin,c.cos)},${c.cos},${c.sin},{{${box.min.toArray()}},{${box.max.toArray()}}},${r.gx},${r.gz},${hero},${r.vertices}},\n`;
 for(const{g}of r.parts)g.dispose();r.parts=[];
}
let data='/* Canonical browser city core, generated with original full-grid RNG flow. */\n#ifndef RR_CANONICAL_CITY_DATA_H\n#define RR_CANONICAL_CITY_DATA_H\n';data+=`#define RR_CANONICAL_COUNT ${records.length}\n#define RR_CANONICAL_HERO_COUNT ${heroes.length}\n#define RR_CANONICAL_MIN_X ${bounds.minX}.0\n#define RR_CANONICAL_MAX_X ${bounds.maxX}.0\n#define RR_CANONICAL_MIN_Z ${bounds.minZ}.0\n#define RR_CANONICAL_MAX_Z ${bounds.maxZ}.0\nstatic const RRCanonicalBuilding rr_canonical_buildings[]={\n${rows}};\nstatic const RRCanonicalHero rr_canonical_heroes[]={\n${heroes.map(h=>`{{${h.x},${h.y},${h.z}},${h.s},${h.rot}}`).join(',\n')}\n};\n#endif\n`;await writeFile(path.join(root,'src/core/rr_canonical_city_data.h'),data);
let fixtures='/* Actual browser tile-only Colliders queries. */\nstatic const CanonicalCollisionFixture canonical_collision_fixtures[]={\n',queries=0;
for(const r of records)for(const p of[r.collider.c.clone(),new THREE.Vector3(r.x+25,118,r.z),new THREE.Vector3(r.x, r.top+2,r.z)]){const q=p.clone(),hit=tileWorld.collide(q,2.1);fixtures+=`{{${p.toArray()}},{${q.toArray()}},${!!hit},${hit?`{${hit.n.toArray()}}`:'{0,0,0}'},${hit?hit.depth:0}},\n`;queries++;}
fixtures+='};\n';let intersections=0;for(const track of tracks)for(const p of track.pts){const q=p.clone();if(tileWorld.collide(q,2.1))intersections++;}
fixtures+=`#define RR_CANONICAL_ROUTE_INTERSECTIONS ${intersections}\n`;await writeFile(path.join(root,'tests/generated_canonical_city_reference.h'),fixtures);
const after=await hashes();if(JSON.stringify(before)!==JSON.stringify(after))throw Error('Reference changed');await writeFile(path.join(root,'reference/canonical-city-core.json'),JSON.stringify({captured_utc:new Date().toISOString(),sources:before,content_hashes_verified_twice:true,tile,bounds,all_grid_towers:result.towers.length,all_grid_heroes:result.heroes.length,selected_buildings:records.length,selected_heroes:heroes.length,hero_anchors:heroes,queries,route_samples:tracks.reduce((n,t)=>n+t.n,0),route_intersections:intersections,omitted_poster_planes:skippedBoards,assets,total_vertices:assets.reduce((n,a)=>n+a.vertices,0),max_building_vertices:Math.max(...assets.map(a=>a.vertices)),material_scope:'Actual source body/neon/tip geometry and base RGB; poster image planes omitted, facade window/PBR maps and full lighting unported.',scope:'Fixed 28x24 canonical source-cell core, including four appended east columns; full original grid RNG/branch flow retained. Outside-cell geometry sinks skip allocation only; no RNG/layout alteration. Distant ring/skyways/traffic excluded from this tile slice.'},null,2)+'\n');console.log(JSON.stringify({buildings:records.length,heroes:heroes.length,all:result.towers.length,total_vertices:assets.reduce((n,a)=>n+a.vertices,0),max_vertices:Math.max(...assets.map(a=>a.vertices)),intersections,bounds},null,2));

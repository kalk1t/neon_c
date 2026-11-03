import {readFile,writeFile,mkdir} from 'node:fs/promises';
import {createHash} from 'node:crypto';import {fileURLToPath,pathToFileURL} from 'node:url';import path from 'node:path';import vm from 'node:vm';
const root=fileURLToPath(new URL('../',import.meta.url)),browser=path.resolve(process.argv[2]??path.join(root,'../game'));
const files=['package.json','src/course.js','src/props.js','src/geo.js','src/materials.js','src/roads.js','src/track.js'];
const hashes=()=>Promise.all(files.map(async p=>({path:p,sha256:createHash('sha256').update(await readFile(path.join(browser,p))).digest('hex')})));
const before=await hashes(),load=p=>import(pathToFileURL(path.join(browser,p)).href);
const [THREE,{Track},{ROADS},props,{rng}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/track.js'),load('src/roads.js'),load('src/props.js'),load('src/materials.js')]);
const {OBB,Cyl,Torus}=await load('src/physics.js'),source=await readFile(path.join(browser,'src/course.js'),'utf8');
const start=source.indexOf('  function setRoad(track) {'),end=source.indexOf('  // ---------- service platforms',start);if(start<0||end<0)throw Error('Course structure changed');
let header='/* Actual browser setRoad furniture output in isolated CPU adapter. */\nstatic const CourseCountFixture course_counts[]={\n',dots=[],items=[],colliders=[],counts=[];
function shape(s){const kind=s instanceof OBB?0:s instanceof Cyl?1:2,c=s.c??new THREE.Vector3(s.x,0,s.z),h=s.h??new THREE.Vector3();return `{${kind},{${c.toArray()}},{${h.toArray()}},${s.cos!==undefined?Math.atan2(s.sin,s.cos):0},${s.R??s.r??0},${s instanceof Torus?s.r:0},${s.y0??0},${s.y1??0}}`;}
for(const roadDef of ROADS){
 const track=new Track(roadDef),cols=[],stub=kind=>({instanced:matrices=>({kind,matrices})});
 const context={THREE,OBB,Cyl,Torus,track,road:null,roam:false,RING:props.RING,group:{add(){},remove(){}},colliders:{add(c){cols.push(c);return c;},remove(){}},shared:new Set(),M:{},ringGeo:{neon:new THREE.BufferGeometry(),halo:new THREE.BufferGeometry()},rng,GREEN:new THREE.Color(0x2bff7a),UP:new THREE.Vector3(0,1,0),hdr:(c,k)=>c.clone().multiplyScalar(k),
 ringStationPrefab:()=>stub(5),trailChevronPrefab:()=>stub(4),beaconPrefab:()=>stub(0),barrierPrefab:()=>stub(1),signPrefab:(m,dir)=>stub(dir>0?2:3),
 yawMat:(pos,yaw,s=1)=>new THREE.Matrix4().compose(pos,new THREE.Quaternion().setFromAxisAngle(new THREE.Vector3(0,1,0),yaw),new THREE.Vector3(s,s,s)),rotY:(x,z,yaw)=>[x*Math.cos(yaw)+z*Math.sin(yaw),-x*Math.sin(yaw)+z*Math.cos(yaw)]};
 vm.createContext(context);vm.runInContext(source.slice(start,end)+'\nsetRoad(track);this.output=road;',context);const out=context.output,kindCounts=[0,0,0,0,0];
 for(let i=0;i<out.dotCount;i++){const m=new THREE.Matrix4();out.dots.getMatrixAt(i,m);dots.push(`{${roadDef.id},${i},{${m.elements.slice(12,15)}}}`);}
 for(const object of out.objs)if(object.kind>=0&&object.kind<5)for(const m of object.matrices){const e=m.elements,k=object.kind;items.push(`{${roadDef.id},${k},${kindCounts[k]++},{${e.slice(12,15)}},{${e.slice(0,3)}},{${e.slice(4,7)}},{${e.slice(8,11)}}}`);}
 cols.forEach((s,i)=>colliders.push(`{${roadDef.id},${i},${shape(s)}}`));counts.push({road:roadDef.id,dots:out.dotCount,items:kindCounts.reduce((a,b)=>a+b),kinds:kindCounts,colliders:cols.length});
 header+=`{${roadDef.id},${out.dotCount},{${kindCounts}},${cols.length}},\n`;
}
header+='};\nstatic const CourseDotFixture course_dots[]={\n'+dots.join(',\n')+'};\nstatic const CourseItemFixture course_items[]={\n'+items.join(',\n')+'};\nstatic const CourseColliderFixture course_colliders[]={\n'+colliders.join(',\n')+'};\n';
await writeFile(path.join(root,'tests/generated_course_furniture_reference.h'),header);
// Existing procedural prefab geometry is exported without invoking makeMaterials,
// canvas textures, browser renderer or any paid generation.
const M=new Proxy({}, {get:(target,key)=>target[key]??(target[key]={key})});
const palettes={black:0x0b0d15,blackSmooth:0x07080e,chrome:0xd2d6e2,gunmetal:0x3c414e,vent:0x030306,neonC:0x14e4ff,neonCSoft:0x14e4ff,neonM:0xff26d4,neonMSoft:0xff26d4,neonG:0x2bff7a,white:0xffffff};
const meshes=[['beacon',props.beaconPrefab(M)],['barrier',props.barrierPrefab(M,1)],['sign-pos',props.signPrefab(M,1)],['sign-neg',props.signPrefab(M,-1)],['chevron',props.trailChevronPrefab(M)],['ring-station',props.ringStationPrefab(M)]];
await mkdir(path.join(root,'assets/course'),{recursive:true});const assets=[];
for(const [name,prefab] of meshes){
 const chunks=prefab.build();let vertices=0;for(const {geometry}of chunks)vertices+=geometry.index?geometry.index.count:geometry.attributes.position.count;
 if(vertices>100000)throw Error('Prefab upload cap exceeded');const bytes=Buffer.alloc(8+vertices*36);bytes.write('RRC1');bytes.writeUInt32LE(vertices,4);let at=8;
 for(const {geometry,material}of chunks){const {position,normal,uv}=geometry.attributes,col=palettes[material.key];if(col===undefined)throw Error('Unknown material '+material.key);
  const count=geometry.index?geometry.index.count:position.count;
  for(let i=0;i<count;i++){const k=geometry.index?geometry.index.getX(i):i;const values=[position.getX(k),position.getY(k),position.getZ(k),normal.getX(k),normal.getY(k),normal.getZ(k),uv?uv.getX(k):0,uv?uv.getY(k):0];
   for(const value of values){if(!Number.isFinite(value))throw Error('Invalid geometry');bytes.writeFloatLE(value,at);at+=4;}
   bytes[at++]=(col>>16)&255;bytes[at++]=(col>>8)&255;bytes[at++]=col&255;bytes[at++]=255;
  }
 }
 const file=`assets/course/${name}.rrmesh`;await writeFile(path.join(root,file),bytes);assets.push({path:file,vertices,bytes:bytes.length,sha256:createHash('sha256').update(bytes).digest('hex')});
}
const after=await hashes();if(JSON.stringify(before)!==JSON.stringify(after))throw Error('Browser changed during capture');
await writeFile(path.join(root,'reference/course-furniture.json'),JSON.stringify({captured_utc:new Date().toISOString(),sources:before,content_hashes_verified_twice:true,counts,fixtures:{dots:dots.length,items:items.length,colliders:colliders.length},assets,material_scope:'Existing prefab triangle geometry/normals/UV and source base RGB palette; native vertex-color shader, no browser HDR/PBR/maps/bloom equivalence.',scope:'Selected-road guidance, chevrons, beacons, turn barriers/signs, full ring supports; no platforms/trees/city layout/progress/renderer execution.'},null,2)+'\n');
console.log(JSON.stringify({counts,fixtures:{dots:dots.length,items:items.length,colliders:colliders.length},assets:assets.map(a=>({path:a.path,vertices:a.vertices}))},null,2));

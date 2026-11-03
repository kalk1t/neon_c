// CPU-only actual Corridor methods and extracted default city.js fitting block.
import {readFile,writeFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {fileURLToPath,pathToFileURL} from 'node:url';
import path from 'node:path';
import vm from 'node:vm';
const root=fileURLToPath(new URL('../',import.meta.url));
const browser=process.argv[2]?path.resolve(process.argv[2]):path.resolve(root,'../game');
const inputs=['package.json','src/roads.js','src/track.js','src/city.js','node_modules/three/src/extras/curves/CatmullRomCurve3.js','node_modules/three/src/extras/core/Curve.js'];
const hashes=()=>Promise.all(inputs.map(async p=>({path:p,sha256:createHash('sha256').update(await readFile(path.join(browser,p))).digest('hex')})));
const before=await hashes(),load=p=>import(pathToFileURL(path.join(browser,p)).href);
const [{ROADS},{Track,Corridor},THREE]=await Promise.all([load('src/roads.js'),load('src/track.js'),load('node_modules/three/build/three.module.js')]);
const tracks=ROADS.map(r=>new Track(r)),all=new Corridor(tracks);
const V=(x,y,z)=>new THREE.Vector3(x,y,z);
const synthetic=(points,gates=[])=>new Corridor([{coarse:points,pts:points,checkpoints:gates,start:{pos:points[0]},bounds:new THREE.Box3().setFromPoints(points)}]);
const contexts=[all,synthetic([V(0,260,0),V(10,160,0),V(-161,118,-161),V(161,90,161)]),synthetic([V(0,200,0)]),synthetic([V(500,200,500)],[{pos:V(0,118,0)}])];
const city=await readFile(path.join(browser,'src/city.js'),'utf8');
const a=city.indexOf('      if (Math.abs(x) < 120'),b=city.indexOf('      tower(x, z, w, d, top, rot, dist < 420);',a);
if(a<0||b<0)throw Error('City fitting source structure changed');
let block=city.slice(a,b);if(!block.includes('top -= 15;'))throw Error('City lowering loop changed');
block=block.replace('top -= 15;','{top -= 15; steps++;}');
const adapter={};vm.createContext(adapter);
vm.runInContext(`this.fit=function(track,x,z,w,d,top,randomValue){
const {y:ty}=track.nearestXZ(x,z),rad=Math.hypot(w,d)/2,cps=track.checkpoints,opts={},LOW=45;
let calls=0,steps=0,accepted=false;const r=()=>{calls++;return randomValue;};
for(let once=0;once<1;once++){${block}\naccepted=true;}
return {accepted,top,calls,steps};};`,adapter);
const f=x=>{if(!Number.isFinite(x))throw Error('Nonfinite reference');return x.toPrecision(17);};
const vec=p=>`{${[p.x,p.y,p.z].map(f).join(',')}}`;
const queries=[],fits=[];
function query(context,x,z,radius,top,margin){const c=contexts[context],n=c.nearestXZ(x,z);queries.push(`{${context},${[x,z,radius,top,margin,n.d,n.y].map(f).join(',')},${c.corridorClear(x,z,radius,top,margin)?1:0}}`);}
for(let x=-2200;x<=2200;x+=220)for(let z=-2200;z<=2200;z+=220)
  for(const [radius,top,margin] of [[12,45,68],[32,160,50],[55,420,40]])query(0,x,z,radius,top,margin);
for(let i=0;i<all.coarse.length;i+=37){const p=all.coarse[i];for(const offset of [-.001,.001])query(0,p.x+88+offset,p.z,20,p.y-68+offset,68);}
for(const [x,z,r,top,m] of [[0,0,0,200,68],[-160,-160,32,250,50],[160,160,4,120,45],[0,0,220,400,68],[3000,3000,4,500,40],[5,0,0,100,50]])query(1,x,z,r,top,m);
for(const [x,z,r,top,m] of [[70,0,20,151,50],[69.999,0,20,151,50],[-70,0,20,151,50],[-69.999,0,20,151,50],[0,0,20,150,50],[0,0,20,150.001,50],[0,0,0,132,68],[0,0,0,132.001,68],[500,500,200,800,160]])query(2,x,z,r,top,m);
function fit(context,x,z,w,d,top,random){const out=adapter.fit(contexts[context],x,z,w,d,top,random);fits.push(`{${context},${[x,z,w,d,top,random].map(f).join(',')},${out.accepted?1:0},${f(out.top)},${out.calls},${out.steps}}`);}
let index=0;
for(let x=-12;x<=12;x++)for(let z=-12;z<=12;z++){
  const i=index++;fit(0,x*160+Math.sin(i*.7)*30,z*160+Math.cos(i*.31)*30,32+(i%6)*6,28+(i%7)*6,65+(i%9)*35,(i%7)/7);
}
for(const x of [0,40,120,160,300])for(const top of [44,55,160,300])fit(1,x,0,32,38,top,.5);
for(const [x,z,w,d,top,r] of [[0,0,32,38,300,.5],[0,0,32,38,54,.1],[120,180,32,38,240,.9],[700,700,32,38,240,.2]])fit(2,x,z,w,d,top,r);
fit(3,0,0,32,38,300,.5);fit(3,700,700,32,38,200,.5);
const coarse=[],points=[];
for(let i=0;i<all.coarse.length;i+=17)coarse.push(`{${i},${vec(all.coarse[i])}}`);
for(let i=0;i<all.pts.length;i+=79)points.push(`{${i},${vec(all.pts[i])}}`);
const after=await hashes();if(JSON.stringify(before)!==JSON.stringify(after))throw Error('Source drift; existing fixtures preserved');
await writeFile(path.join(root,'tests/generated_city_clearance_reference.h'),`/* Actual browser Corridor and extracted default city.js fitting block; no graphics. */\nstatic const CorridorQueryFixture corridor_queries[]={\n${queries.join(',\n')}\n};\nstatic const CityFitFixture city_fit_fixtures[]={\n${fits.join(',\n')}\n};\nstatic const CorridorPointFixture corridor_coarse_fixtures[]={\n${coarse.join(',\n')}\n};\nstatic const CorridorPointFixture corridor_point_fixtures[]={\n${points.join(',\n')}\n};\nstatic const int reference_corridor_counts[]={${all.n},${all.coarse.length},${all.checkpoints.length},${all.grid.size}};\nstatic const RRVec3 reference_corridor_min=${vec(all.bounds.min)},reference_corridor_max=${vec(all.bounds.max)};\n`);
await writeFile(path.join(root,'reference/city-clearance.json'),JSON.stringify({captured_utc:new Date().toISOString(),sources:after,content_hashes_verified_twice:true,queries:queries.length,fit_cases:fits.length,contexts:['all nine real tracks','synthetic high/lower crossing and negative cells','single-point strict boundaries','checkpoint exclusion'],corridor:{points:all.n,coarse:all.coarse.length,gates:all.checkpoints.length,occupied_cells:all.grid.size},scope:'Actual union Corridor queries and the default city candidate start/checkpoint/height-fitting block, with fixed injected lowering randomness. No full browser city layout/style/random stream, meshes, colliders, traffic, rendering, profile or progress writes.'},null,2)+'\n');
console.log(`${queries.length} browser corridor queries; ${fits.length} actual fitting-block cases; ${all.n} route points; ${all.coarse.length} coarse; ${all.grid.size} occupied cells. No graphics.`);
